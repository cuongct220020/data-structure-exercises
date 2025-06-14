#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Định nghĩa struct cho thuộc tính của thẻ XML
typedef struct Attribute {
    char* name;                   // Tên thuộc tính
    char* value;                  // Giá trị thuộc tính
    struct Attribute* next;       // Con trỏ tới thuộc tính tiếp theo (danh sách liên kết)
} Attribute;

// Định nghĩa struct cho một node trong cây XML
typedef struct TreeNode {
    char* tag_name;               // Tên thẻ (tag)
    Attribute* attributes;        // Danh sách thuộc tính của thẻ
    char* text;                   // Nội dung văn bản bên trong thẻ (nếu có)
    struct TreeNode* first_child; // Con đầu tiên (dùng cho cây đa phân)
    struct TreeNode* next_sibling;// Anh em kế tiếp (dùng cho cây đa phân)
    struct TreeNode* parent;      // Con trỏ tới node cha
} TreeNode;

// // Hàm thay thế strndup nếu hệ thống không hỗ trợ
// #ifndef _POSIX_C_SOURCE
// char* strndup(const char* s, size_t n) {
//     size_t len = strnlen(s, n);
//     char* new = (char*)malloc(len + 1);
//     if (!new) return NULL;
//     memcpy(new, s, len);
//     new[len] = '\0';
//     return new;
// }
// #endif

// Tạo một node mới với tên thẻ cho trước
TreeNode* createNode(const char* tag_name) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    if (!node) return NULL;
    node->tag_name = strdup(tag_name); // Sao chép tên thẻ
    node->attributes = NULL;           // Chưa có thuộc tính
    node->text = NULL;                 // Chưa có nội dung
    node->first_child = NULL;          // Chưa có con
    node->next_sibling = NULL;         // Chưa có anh em
    node->parent = NULL;               // Chưa có cha
    return node;
}

// Thêm một node con vào node cha
void addChild(TreeNode* parent, TreeNode* child) {
    if (parent->first_child == NULL) {
        parent->first_child = child; // Nếu chưa có con thì gán luôn
    } else {
        TreeNode* temp = parent->first_child;
        while (temp->next_sibling != NULL) {
            temp = temp->next_sibling; // Duyệt đến anh em cuối cùng
        }
        temp->next_sibling = child; // Thêm vào cuối danh sách anh em
    }
    child->parent = parent; // Gán cha cho node con
}

// Loại bỏ khoảng trắng đầu và cuối chuỗi
char* trim(char* str) {
    while (isspace((unsigned char)*str)) str++; // Bỏ khoảng trắng đầu
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) *end-- = '\0'; // Bỏ khoảng trắng cuối
    return str;
}

// Giải phóng bộ nhớ của một node và toàn bộ cây con của nó
void freeNode(TreeNode* node) {
    if (!node) return;

    // Giải phóng danh sách thuộc tính
    Attribute* attr = node->attributes;
    while (attr) {
        Attribute* next = attr->next;
        free(attr->name);
        free(attr->value);
        free(attr);
        attr = next;
    }

    // Giải phóng các node con
    TreeNode* child = node->first_child;
    while (child) {
        TreeNode* next_child = child->next_sibling;
        freeNode(child);
        child = next_child;
    }

    free(node->tag_name);
    free(node->text);
    free(node);
}

// Phân tích file XML, xây dựng cây DOM
TreeNode* parseXML(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Khong the mo file.\n");
        return NULL;
    }

    // Đọc toàn bộ file vào buffer
    char* buffer = NULL;
    size_t buffer_size = 0;
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        buffer = realloc(buffer, buffer_size + len + 1);
        if (!buffer) {
            printf("Loi cap phat bo nho.\n");
            fclose(file);
            return NULL;
        }
        strcpy(buffer + buffer_size, line);
        buffer_size += len;
    }
    fclose(file);
    if (!buffer) {
        printf("File trong.\n");
        return NULL;
    }
    buffer[buffer_size] = '\0';

    TreeNode* root = NULL;    // Node gốc của cây
    TreeNode* current = NULL; // Node hiện tại đang xử lý
    char* pos = buffer;       // Con trỏ duyệt buffer

    while (*pos) {
        while (isspace((unsigned char)*pos)) pos++; // Bỏ khoảng trắng
        if (!*pos) break;

        // Bỏ qua phần khai báo XML (<? ... ?>)
        if (strncmp(pos, "<?", 2) == 0) {
            pos = strchr(pos, '>');
            if (pos) pos++;
            continue;
        }

        // Nếu gặp thẻ đóng
        if (*pos == '<') {
            if (*(pos + 1) == '/') {
                // Đọc tên thẻ đóng
                char tag[256];
                sscanf(pos, "</%[^>]", tag);
                pos = strchr(pos, '>');
                if (pos) pos++;
                else break;

                // Kiểm tra thẻ đóng có khớp với node hiện tại không
                if (!current || strcmp(current->tag_name, tag) != 0) {
                    printf("Sai the dong </%s>\n", tag);
                    free(buffer);
                    freeNode(root);
                    return NULL;
                }
                current = current->parent; // Quay lại node cha
            } else {
                // Đọc tên thẻ mở
                char tag[256] = {0};
                pos++;
                int i = 0;
                while (*pos && *pos != ' ' && *pos != '>' && *pos != '/') {
                    if (i < 255) tag[i++] = *pos;
                    pos++;
                }
                tag[i] = '\0';

                // Tạo node mới cho thẻ này
                TreeNode* new_node = createNode(tag);
                if (!new_node) {
                    printf("Loi cap phat node.\n");
                    free(buffer);
                    freeNode(root);
                    return NULL;
                }

                // Đọc các thuộc tính của thẻ (nếu có)
                while (*pos && *pos != '>' && *pos != '/') {
                    while (*pos == ' ' || *pos == ',' || *pos == '\'') pos++; // Bỏ khoảng trắng, dấu phẩy, dấu nháy đơn

                    if (*pos == '>' || *pos == '/') break;

                    // Chỉ nhận thuộc tính bắt đầu bằng chữ cái hoặc dấu gạch dưới
                    if (!isalpha((unsigned char)*pos) && *pos != '_') {
                        pos++;
                        continue;
                    }

                    char attr_name[256] = {0}, attr_value[256] = {0};
                    int j = 0;
                    while (*pos && *pos != '=' && *pos != ' ' && *pos != '>' && *pos != '/' && *pos != ',' && *pos != '\'') {
                        if (j < 255) attr_name[j++] = *pos;
                        pos++;
                    }
                    attr_name[j] = '\0';

                    while (*pos == ' ' || *pos == ',' || *pos == '\'') pos++; // Bỏ ký tự lạ

                    if (*pos == '=') {
                        pos++;
                        while (*pos == ' ' || *pos == ',' || *pos == '\'') pos++; // Bỏ ký tự lạ
                        if (*pos == '"') {
                            pos++;
                            j = 0;
                            while (*pos && *pos != '"') {
                                if (j < 255) attr_value[j++] = *pos;
                                pos++;
                            }
                            attr_value[j] = '\0';
                            if (*pos == '"') pos++;
                        }
                    }
                    // Nếu có thuộc tính thì thêm vào node
                    if (strlen(attr_name) > 0) {
                        Attribute* new_attr = (Attribute*)malloc(sizeof(Attribute));
                        if (!new_attr) {
                            printf("Loi cap phat thuoc tinh.\n");
                            freeNode(new_node);
                            free(buffer);
                            freeNode(root);
                            return NULL;
                        }
                        new_attr->name = strdup(attr_name);
                        new_attr->value = strdup(attr_value);
                        new_attr->next = new_node->attributes;
                        new_node->attributes = new_attr;
                    }
                }

                // Kiểm tra thẻ tự đóng (self-closing)
                int is_self_closing = (*pos == '/');
                if (*pos == '/') pos++;
                pos++;

                // Đọc nội dung văn bản bên trong thẻ (nếu có)
                char* text_start = pos;
                char* text_end = pos;
                while (*text_end && *text_end != '<') text_end++;
                if (text_end > text_start) {
                    char* text = (char*)malloc(text_end - text_start + 1);
                    strncpy(text, text_start, text_end - text_start);
                    text[text_end - text_start] = '\0';
                    char* trimmed = trim(text);
                    new_node->text = strdup(trimmed);
                    free(text);
                }
                pos = text_end;

                // Thêm node mới vào cây
                if (!root) {
                    root = new_node;
                } else {
                    addChild(current, new_node);
                }

                // Nếu không phải thẻ tự đóng thì chuyển current sang node mới
                if (!is_self_closing) {
                    current = new_node;
                }
            }
        }
    }

    free(buffer);
    printf("Doc file XML thanh cong!\n");
    return root;
}

// Thêm một thẻ mới (new_tag) vào dưới thẻ cha có tên parent_tag
void addTag(TreeNode* root, const char* parent_tag, const char* new_tag, const char* text) {
    if (!root) return;
    if (strcmp(root->tag_name, parent_tag) == 0) {
        TreeNode* new_node = createNode(new_tag);
        new_node->text = text ? strdup(text) : NULL;
        addChild(root, new_node);
        printf("Da them <%s>%s</%s> vao <%s>\n", new_tag, text ? text : "", new_tag, parent_tag);
        return;
    }

    TreeNode* child = root->first_child;
    while (child) {
        addTag(child, parent_tag, new_tag, text);
        child = child->next_sibling;
    }
}

// Xóa tất cả các thẻ có tên tag_name dưới node parent
void removeTag(TreeNode* parent, const char* tag_name) {
    if (!parent) return;

    TreeNode* prev = NULL;
    TreeNode* current = parent->first_child;

    while (current) {
        TreeNode* next = current->next_sibling;

        if (strcmp(current->tag_name, tag_name) == 0) {
            // Nếu là node cần xóa
            if (prev == NULL) {
                parent->first_child = next;
            } else {
                prev->next_sibling = next;
            }
            printf("Đã xóa thẻ <%s>\n", tag_name);
            freeNode(current);
        } else {
            // Đệ quy xóa trong các node con
            removeTag(current, tag_name);
            prev = current;
        }

        current = next;
    }
}

// Thay đổi hoặc thêm thuộc tính cho thẻ có tên tag_name
void changeAttribute(TreeNode* root, const char* tag_name, const char* attr_name, const char* new_value) {
    if (!root) return;

    if (strcmp(root->tag_name, tag_name) == 0) {
        Attribute* attr = root->attributes;
        while (attr) {
            if (strcmp(attr->name, attr_name) == 0) {
                free(attr->value);
                attr->value = strdup(new_value);
                printf("Da thay doi %s=\"%s\" trong <%s>\n", attr_name, new_value, tag_name);
                return;
            }
            attr = attr->next;
        }
        // Nếu chưa có thuộc tính thì thêm mới
        Attribute* new_attr = (Attribute*)malloc(sizeof(Attribute));
        new_attr->name = strdup(attr_name);
        new_attr->value = strdup(new_value);
        new_attr->next = root->attributes;
        root->attributes = new_attr;
        printf("Da them thuoc tinh %s=\"%s\" trong <%s>\n", attr_name, new_value, tag_name);
        return;
    }

    // Đệ quy tìm trong các node con và anh em
    changeAttribute(root->first_child, tag_name, attr_name, new_value);
    changeAttribute(root->next_sibling, tag_name, attr_name, new_value);
}

// Ghi một node và các node con ra file XML với thụt lề
void writeNodeToFile(TreeNode* node, FILE* file, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; i++) {
        fprintf(file, "    "); // In thụt lề
    }

    fprintf(file, "<%s", node->tag_name);

    // In các thuộc tính
    Attribute* attr = node->attributes;
    while (attr) {
        fprintf(file, " %s=\"%s\"", attr->name, attr->value);
        attr = attr->next;
    }

    // Nếu không có con và không có text thì tự đóng
    if (!node->first_child && (!node->text || strlen(node->text) == 0)) {
        fprintf(file, "/>\n");
    } else {
        fprintf(file, ">");
        if (node->text) {
            fprintf(file, "%s", node->text);
        }
        if (node->first_child) {
            fprintf(file, "\n");
            TreeNode* child = node->first_child;
            while (child) {
                writeNodeToFile(child, file, indent + 1);
                child = child->next_sibling;
            }
            for (int i = 0; i < indent; i++) {
                fprintf(file, "    ");
            }
        }
        fprintf(file, "</%s>\n", node->tag_name);
    }
}

// Ghi toàn bộ cây XML ra file
void writeXMLToFile(TreeNode* root, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Khong the mo file de ghi!\n");
        return;
    }

    fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    writeNodeToFile(root, file, 0);
    fclose(file);
    printf("Da ghi cay XML ra file \"%s\"\n", filename);
}

// Tìm và in giá trị thuộc tính có tên key trong toàn bộ cây
void findAttributeValue(TreeNode* node, const char* key) {
    if (!node) return;

    Attribute* attr = node->attributes;
    while (attr) {
        if (strcmp(attr->name, key) == 0) {
            printf("The <%s> co thuoc tinh '%s' = \"%s\"\n", node->tag_name, key, attr->value);
        }
        attr = attr->next;
    }

    findAttributeValue(node->first_child, key);
    findAttributeValue(node->next_sibling, key);
}

// Tìm và in nội dung của tất cả các thẻ có tên tag
void findTagContent(TreeNode* node, const char* tag) {
    if (!node) return;

    if (strcmp(node->tag_name, tag) == 0 && node->text != NULL) {
        printf("Noi dung the <%s>: %s\n", tag, node->text);
    }

    findTagContent(node->first_child, tag);
    findTagContent(node->next_sibling, tag);
}

// Hàm main: thực hiện các thao tác với cây XML
int main() {
    const char* filename = "XML_input.txt";
    TreeNode* root = parseXML(filename);
    if (!root) {
        printf("Loi khi doc file XML\n");
        return 1;
    }
    //printf("Doc file thanh cong.\n");

    // Thêm <age> vào <author>
    addTag(root, "author", "age", "50");

    // Sửa thuộc tính
    changeAttribute(root, "book", "id", "2");

    // Tìm thuộc tính
    findAttributeValue(root, "id");

    // Tìm nội dung thẻ
    findTagContent(root, "title");

    // Xóa thẻ <author>
    //removeTag(root, "author");

    // Ghi lại XML ra file
    writeXMLToFile(root, "XML_output.xml");

    // Giải phóng bộ nhớ
    freeNode(root);

    return 0;
}