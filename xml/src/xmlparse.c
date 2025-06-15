#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "XMLTree.h"
#include "TagStack.h"

int is_valid_tag(const char* tag_name) {
    // Tên thẻ không được rỗng
    if (tag_name == NULL || *tag_name == '\0') {
        printf("[is_valid_tag] Tên thẻ không được rỗng.\n");
        return 0;
    }

    // Tên thẻ không được bắt đầu bằng "xml" (phân biệt chữ hoa/thường)
    if (strncasecmp(tag_name, "xml", 3) == 0) {
        printf("[is_valid_tag] Tên thẻ không được bắt đầu bằng 'xml'.\n");
        return 0;
    }

    // Ký tự đầu tiên phải là chữ cái hoặc dấu gạch dưới
    if (!isalpha(tag_name[0]) && tag_name[0] != '_') {
        printf("[is_valid_tag] Ký tự đầu tiên của tên thẻ phải là chữ cái hoặc dấu gạch dưới.\n");
        return 0;
    }

    // Các ký tự còn lại chỉ được chứa chữ cái, số, dấu gạch dưới, dấu gạch ngang, dấu chấm.
    for (int i = 1; tag_name[i] != '\0'; i++) {
        if (!isalnum(tag_name[i]) && tag_name[i] != '_' && tag_name[i] != '-' && tag_name[i] != '.') {
            printf("[is_valid_tag] Tên thẻ chỉ được chứa chữ cái, số, dấu gạch dưới, dấu gạch ngang và dấu chấm.\n");
            return 0;
        }
    }

    // Không chứa dấu hai chấm (:) trong tên thẻ
    if (strchr(tag_name, ':') != NULL) {
        printf("[is_valid_tag] Tên thẻ không được chứa dấu hai chấm (:).\n");
        return 0;
    }

    // Không chứa khoảng trắng trong tên thẻ
    if (strchr(tag_name, ' ') != NULL) {
        printf("[is_valid_tag] Tên thẻ không được chứa khoảng trắng.\n");
        return 0;
    }

    return 1; // Tên thẻ hợp lệ
}

int is_valid_xml_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("[read_xml_file] Không thể mở file: %s\n", filename);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);

    if (filesize == 0) {
        printf("[read_xml_file] File rỗng: %s\n", filename);
        fclose(file);
        return 0;
    }

    char* buffer = (char*)malloc(filesize + 1);
    if (buffer == NULL) {
        printf("[read_xml_file] Lỗi cấp phát bộ nhớ.\n");
        fclose(file);
        return 0;
    }

    size_t bytesRead = fread(buffer, 1, filesize, file);
    buffer[bytesRead] = '\0';
    fclose(file);

    if (bytesRead != filesize) {
        printf("[read_xml_file] Lỗi đọc file: %s\n", filename);
        free(buffer);
        return 0;
    }

    TagStack stack;
    init_stack(&stack);

    char* pos = buffer;
    int valid = 1;
    int line_number = 1;

    while ((pos = strchr(pos, '<')) != NULL) {
        // Count newlines for better error reporting
        for (char* p = buffer; p < pos; p++) {
            if (*p == '\n') line_number++;
        }

        // Skip XML declaration
        if (*(pos + 1) == '?') {
            pos = strchr(pos, '>');
            if (pos == NULL) {
                printf("[read_xml_file] Lỗi: Thẻ khai báo XML không đóng ở dòng %d\n", line_number);
                valid = 0;
                break;
            }
            pos++;
            continue;
        }

        // Handle closing tag
        if (*(pos + 1) == '/') {
            char* end_tag = strchr(pos, '>');
            if (end_tag == NULL) {
                printf("[read_xml_file] Lỗi: Thẻ đóng không hợp lệ ở dòng %d\n", line_number);
                valid = 0;
                break;
            }

            size_t tag_length = end_tag - pos - 2;
            char* tag_name = (char*)malloc(tag_length + 1);
            if (tag_name == NULL) {
                printf("[read_xml_file] Lỗi cấp phát bộ nhớ cho tên thẻ đóng.\n");
                free(buffer);
                return 0;
            }
            strncpy(tag_name, pos + 2, tag_length);
            tag_name[tag_length] = '\0';

            // Trim whitespace
            char* end = tag_name + strlen(tag_name) - 1;
            while (end > tag_name && isspace(*end)) *end-- = '\0';
            while (isspace(*tag_name)) tag_name++;

            if (!is_valid_tag(tag_name)) {
                printf("[read_xml_file] Tên thẻ đóng không hợp lệ ở dòng %d: </%s>\n", line_number, tag_name);
                free(tag_name);
                valid = 0;
                break;
            }

            if (is_empty(&stack)) {
                printf("[read_xml_file] Lỗi: Thẻ đóng </%s> không có thẻ mở tương ứng ở dòng %d\n", tag_name, line_number);
                free(tag_name);
                valid = 0;
                break;
            }

            char* popped_tag = pop(&stack);
            if (strcmp(popped_tag, tag_name) != 0) {
                printf("[read_xml_file] Lỗi: Thẻ đóng </%s> không khớp với thẻ mở <%s> ở dòng %d\n", 
                       tag_name, popped_tag, line_number);
                free(tag_name);
                free(popped_tag);
                valid = 0;
                break;
            }

            free(tag_name);
            free(popped_tag);
            pos = end_tag + 1;
            continue;
        }

        // Handle opening tag
        char* end_tag = strchr(pos, '>');
        if (end_tag == NULL) {
            printf("[read_xml_file] Lỗi: Thẻ mở không đóng ở dòng %d\n", line_number);
            valid = 0;
            break;
        }

        size_t tag_length = end_tag - pos - 1;
        char* tag_name = (char*)malloc(tag_length + 1);
        if (tag_name == NULL) {
            printf("[read_xml_file] Lỗi cấp phát bộ nhớ cho tên thẻ.\n");
            free(buffer);
            return 0;
        }
        strncpy(tag_name, pos + 1, tag_length);
        tag_name[tag_length] = '\0';

        // Trim whitespace and handle attributes
        char* space_pos = strchr(tag_name, ' ');
        if (space_pos != NULL) *space_pos = '\0';

        // Trim any remaining whitespace
        char* end = tag_name + strlen(tag_name) - 1;
        while (end > tag_name && isspace(*end)) *end-- = '\0';
        while (isspace(*tag_name)) tag_name++;

        if (!is_valid_tag(tag_name)) {
            printf("[read_xml_file] Tên thẻ không hợp lệ ở dòng %d: <%s>\n", line_number, tag_name);
            free(tag_name);
            valid = 0;
            break;
        }

        if (!push(&stack, tag_name)) {
            printf("[read_xml_file] Lỗi đẩy thẻ vào stack ở dòng %d.\n", line_number);
            free(tag_name);
            free(buffer);
            return 0;
        }
        free(tag_name);
        pos = end_tag + 1;
    }

    free(buffer);
    if (!is_empty(&stack)) {
        printf("[read_xml_file] Lỗi: Còn thẻ chưa đóng: <%s>\n", peek(&stack));
        valid = 0;
    }

    free_stack(&stack);
    if (valid) {
        printf("[read_xml_file] File XML hợp lệ.\n");
    } else {
        printf("[read_xml_file] File XML không hợp lệ.\n");
    }
    return valid;
}

TreeNode* create_node(const char* tag_name) {
    TreeNode* newNode = (TreeNode*)malloc(sizeof(TreeNode));
    if (newNode == NULL) {
        printf("[create_node] Lỗi cấp phát bộ nhớ động.\n");
        return NULL;
    }
    newNode->tag_name = strdup(tag_name);
    newNode->attributes = NULL;
    newNode->text = NULL;
    newNode->first_child = NULL;
    newNode->next_sibling = NULL;
    newNode->parent = NULL;
    return newNode;
}

void add_tag(TreeNode* parent, const char* tag_name) {
    if (parent == NULL || tag_name == NULL) {
        printf("[add_tag] Thẻ cha hoặc tên thẻ không hợp lệ.\n");
        return;
    }

    TreeNode* newNode = create_node(tag_name);
    if (newNode == NULL) return;

    newNode->parent = parent;

    if (parent->first_child == NULL) {
        parent->first_child = newNode;
    } else {
        TreeNode* sibling = parent->first_child;
        while (sibling->next_sibling != NULL) {
            sibling = sibling->next_sibling;
        }
        sibling->next_sibling = newNode;
    }
}

// Xoá node con đầu tiên có tag_name khớp khỏi danh sách con của parent
void delete_child_by_tag_name(TreeNode* parent, const char* tag_name) {
    if (!parent || !tag_name) return;
    TreeNode* prev = NULL;
    TreeNode* curr = parent->first_child;
    while (curr) {
        if (strcmp(curr->tag_name, tag_name) == 0) {
            // Ngắt liên kết node cần xoá khỏi danh sách
            if (prev) {
                prev->next_sibling = curr->next_sibling;
            } else {
                parent->first_child = curr->next_sibling;
            }
            curr->next_sibling = NULL; // Ngắt liên kết để tránh xoá nhầm các node khác
            delete_tag(curr);
            return;
        }
        prev = curr;
        curr = curr->next_sibling;
    }
}

void delete_tag(TreeNode* node) {
    if (node == NULL) return;

    // Giải phóng thuộc tính
    Attribute* attr = node->attributes;
    while (attr != NULL) {
        Attribute* next_attr = attr->next;
        if (attr->name != NULL) free(attr->name);
        if (attr->value != NULL) free(attr->value);
        free(attr);
        attr = next_attr;
    }
    node->attributes = NULL;

    // Giải phóng text
    if (node->text != NULL) {
        free(node->text);
        node->text = NULL;
    }

    // Lưu lại con trỏ trước khi giải phóng
    TreeNode* child = node->first_child;
    TreeNode* sibling = node->next_sibling;
    node->first_child = NULL;
    node->next_sibling = NULL;

    // Giải phóng con và anh em
    delete_tag(child);
    delete_tag(sibling);

    // Giải phóng node hiện tại
    if (node->tag_name != NULL) {
        free(node->tag_name);
        node->tag_name = NULL;
    }
    free(node);
}

void change_attribute(TreeNode* node, const char* attr_name, const char* new_value) {
    if (node == NULL || attr_name == NULL || new_value == NULL) return;

    Attribute* attr = node->attributes;
    while (attr != NULL) {
        if (strcmp(attr->name, attr_name) == 0) {
            free(attr->value);
            attr->value = strdup(new_value);
            return;
        }
        attr = attr->next;
    }
}

void add_attribute(TreeNode* node, const char* attr_name, const char* value) {
    if (node == NULL || attr_name == NULL || value == NULL) {
        printf("[add_attribute] Node, tên thuộc tính hoặc giá trị không hợp lệ.\n");
        return;
    }

    Attribute* new_attr = (Attribute*)malloc(sizeof(Attribute));
    if (new_attr == NULL) {
        printf("[add_attribute] Lỗi cấp phát bộ nhớ cho thuộc tính.\n");
        return;
    }
    new_attr->name = strdup(attr_name);
    new_attr->value = strdup(value);
    new_attr->next = node->attributes;
    node->attributes = new_attr;
}

int search_and_print_tag(TreeNode* root, const char* tag_name) {
    if (root == NULL || tag_name == NULL) {
        printf("[search_and_print_tag] Node gốc hoặc tên thẻ không hợp lệ.\n");
        return 0;
    }

    if (strcmp(root->tag_name, tag_name) == 0) {
        printf("[search_and_print_tag] Tìm thấy thẻ: <%s>\n", root->tag_name);
        return 1;
    }

    // Tìm kiếm trong các con
    if (search_and_print_tag(root->first_child, tag_name)) {
        return 1;
    }

    // Tìm kiếm trong các anh em
    return search_and_print_tag(root->next_sibling, tag_name);
}

int search_and_print(TreeNode* root, const char* tag_name) {
    if (root == NULL || tag_name == NULL) return 0;

    if (strcmp(root->tag_name, tag_name) == 0) {
        printf("[search_and_print] Nội dung của thẻ <%s>: %s\n", root->tag_name, root->text ? root->text : "Không có nội dung");
        return 1;
    }

    // Tìm kiếm trong các con
    if (search_and_print(root->first_child, tag_name)) {
        return 1;
    }

    // Tìm kiếm trong các anh em
    return search_and_print(root->next_sibling, tag_name);
}


void free_xml_tree(TreeNode* root) {
    if (root == NULL) return;

    // Gọi đến hàm delete_tag để giải phóng bộ nhớ
    delete_tag(root);
    root = NULL; // Đặt con trỏ về NULL để tránh truy cập vào vùng nhớ đã giải phóng
    printf("[free_xml_tree] Cây XML đã được giải phóng.\n");
}

void write_tag(FILE* file, TreeNode* node, int indent) {
    if (!node) return;

    // In thụt lề
    for (int i = 0; i < indent; i++) fprintf(file, "    ");

    // In thẻ mở và thuộc tính
    fprintf(file, "<%s", node->tag_name);
    Attribute* attr = node->attributes;
    while (attr) {
        fprintf(file, " %s=\"%s\"", attr->name, attr->value);
        attr = attr->next;
    }

    // Nếu có con hoặc có text
    if (node->first_child || (node->text && strlen(node->text) > 0)) {
        fprintf(file, ">");
        if (node->text && strlen(node->text) > 0) {
            fprintf(file, "%s", node->text);
        }
        if (node->first_child) {
            fprintf(file, "\n");
            TreeNode* child = node->first_child;
            while (child) {
                write_tag(file, child, indent + 1);
                child = child->next_sibling;
            }
            for (int i = 0; i < indent; i++) fprintf(file, "    ");
        }
        fprintf(file, "</%s>\n", node->tag_name);
    } else {
        // Không có con, không có text => thẻ tự đóng
        fprintf(file, "/>\n");
    }
}

void write_xml_file(const char* filename, TreeNode* root) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("[write_xml_file] Không thể mở file để ghi!\n");
        return;
    }
    fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    write_tag(file, root, 0);
    fclose(file);
    printf("[write_xml_file] Đã ghi file XML ra \"%s\"\n", filename);
}