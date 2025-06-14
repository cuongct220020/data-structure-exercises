#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME 64
#define MAX_VARS 100
#define MAX_CHILDREN 10
#define MAX_LINE 1000

typedef struct Scope {
    char name[MAX_NAME];                // Tên của scope (phạm vi)
    struct Scope* parent;               // Con trỏ đến scope cha (phạm vi cha)
    struct Scope* children[MAX_CHILDREN]; // Danh sách các scope con (phạm vi con)
    int child_count;                    // Số lượng scope con
    Scope* next;                  // Con trỏ đến scope tiếp theo trong danh sách
    char variables[MAX_VARS][MAX_NAME]; // Danh sách các biến đã khai báo trong scope này
    int var_count;                      // Số lượng biến đã khai báo trong scope này
} Scope;


// Tạo một scope mới, gán tên và liên kết với scope cha (nếu có)
Scope* create_scope(Scope* parent, const char* name);

// Thêm biến vào scope hiện tại, kiểm tra ghi đè biến ở scope cha
void add_variable(Scope* scope, const char* var);

// Kiểm tra biến đã được khai báo trong scope hiện tại hoặc các scope cha chưa
int is_defined(Scope* scope, char* var);

// In ra cấu trúc cây phạm vi (scope) và các biến trong từng scope, thụt lề theo cấp độ
void print_scope(Scope* scope, int level);

// Phân tích file mã nguồn, xây dựng cây phạm vi, kiểm tra khai báo và sử dụng biến
void parse_file(const char* filename);

// Tạo một scope mới, gán tên và liên kết với scope cha (nếu có)
Scope* create_scope(Scope* parent, const char* name) {
    Scope* scope = (Scope*)malloc(sizeof(Scope));
    strcpy(scope->name, name);
    scope->parent = parent;
    scope->child_count = 0;
    scope->var_count = 0;

    if (parent) {
        parent->children[parent->child_count++] = scope;
    }
    return scope;
}

// Thêm biến vào scope hiện tại, kiểm tra ghi đè biến ở scope cha
void add_variable(Scope* scope, const char* var) {
    Scope* temp = scope->parent;
    while (temp) {
        for (int i = 0; i < temp->var_count; i++) {
            if (strcmp(temp->variables[i], var) == 0) {
                printf("ERROR: Bien '%s' trong scope '%s' ghi de bien trong scope '%s'\n",
                       var, scope->name, temp->name);
            }
        }
        temp = temp->parent;
    }
    strcpy(scope->variables[scope->var_count++], var);
}

// Kiểm tra biến đã được khai báo trong scope hiện tại hoặc các scope cha chưa
int is_defined(Scope* scope, char* var) {
    Scope* temp = scope;
    while (temp) {
        for (int i = 0; i < temp->var_count; i++) {
            if (strcmp(temp->variables[i], var) == 0)
                return 1;
        }
        temp = temp->parent;
    }
    return 0;
}

// In ra cấu trúc cây phạm vi (scope) và các biến trong từng scope, thụt lề theo cấp độ
void print_scope(Scope* scope, int level) {
    for (int i = 0; i < level; i++) printf(" ");
    printf("Scope: %s\n", scope->name);

    for (int i = 0; i < scope->var_count; i++) {
        for (int j = 0; j < level + 1; j++) printf(" ");
        printf("%s\n", scope->variables[i]);
    }

    for (int i = 0; i < scope->child_count; i++) {
        print_scope(scope->children[i], level + 1);
    }
}

// Phân tích file mã nguồn, xây dựng cây phạm vi, kiểm tra khai báo và sử dụng biến
void parse_file(const char* filename) {
    // Mở file mã nguồn để đọc
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Khong the mo file %s\n", filename);
        return;
    }

    // Khởi tạo scope toàn cục (global scope)
    Scope* global_scope = create_scope(NULL, "Global");
    Scope* current = global_scope; // Scope hiện tại đang xử lý
    int scope_id = 0;              // Đếm số lượng scope để đặt tên
    int has_error = 0;             // Cờ kiểm tra có lỗi khai báo/sử dụng biến không
    
    char line[MAX_LINE];
    // Đọc từng dòng trong file
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Xóa ký tự xuống dòng ở cuối

        // Kiểm tra khai báo biến (int, float, char)
        char type[16], name[64];
        if (sscanf(line, "%s %[^;];", type, name) == 2) {
            if (strcmp(type, "int") == 0 || strcmp(type, "float") == 0 || strcmp(type, "char") == 0) {
                // Thêm biến vào scope hiện tại
                add_variable(current, name);
            }
        }

        // Nếu gặp dấu '{', tạo scope mới (ví dụ: vào hàm, vào khối lệnh if, for, ...)
        if (strchr(line, '{')) {
            char scope_name[32];
            sprintf(scope_name, "Scope_%d", ++scope_id);
            current = create_scope(current, scope_name);
        }

        // Nếu gặp dấu '}', quay lại scope cha (kết thúc một khối lệnh)
        if (strchr(line, '}')) {
            if (current->parent)
                current = current->parent;
        }

        // Kiểm tra sử dụng biến trong lệnh printf
        if (strstr(line, "printf")) {
            // Loại bỏ comment nếu có trong dòng
            char* comment = strstr(line, "//");
            if (comment) *comment = '\0';

            // Tìm phần giữa dấu '(' và ')'
            char* open = strchr(line, '(');
            char* close = strrchr(line, ')');
            if (open && close && close > open) {
                char inside[128];
                strncpy(inside, open + 1, close - open - 1);
                inside[close - open - 1] = '\0';

                // Tìm dấu ',' để lấy tên biến được in ra
                char* comma = strchr(inside, ',');
                if (comma) {
                    char* varname = comma + 1;
                    while (*varname == ' ') varname++; // Bỏ khoảng trắng đầu

                    // Cắt bỏ các ký tự lạ sau tên biến
                    varname[strcspn(varname, " );\n\t")] = '\0';

                    // Kiểm tra biến đã được khai báo trong scope hiện tại hoặc cha chưa
                    if (!is_defined(current, varname)) {
                        printf("Loi: Bien '%s' duoc dung trong scope '%s' nhung chua duoc khai bao!\n",
                               varname, current->name);
                        has_error = 1;
                    }
                }
            }
        }
    }

    fclose(file);

    // Nếu không có lỗi, in ra cây phạm vi
    if (!has_error) {
        printf("\nKhong co loi, cay pham vi:\n");
        print_scope(global_scope, 0);
    } else {
        printf("\nPhat hien loi.\n");
    }
}

int main() {
    parse_file("vs_input.txt");
    return 0;
}
