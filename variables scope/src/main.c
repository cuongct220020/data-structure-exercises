#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "scopetree.h"
#include "helper.h"

char* trim(char* str) {
    char* end;
    // Bỏ khoảng trắng đầu
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0)  // Chuỗi rỗng
        return str;
    // Bỏ khoảng trắng cuối
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    // Kết thúc chuỗi
    *(end+1) = 0;
    return str;
}

// Hàm phân tích file .c và kiểm tra tính hợp lệ 
int parse_file_c(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Cannot open file %s\n", filename);
        return -1;
    }

    // Tạo phạm vi toàn cục
    Scope* global_scope = create_scope(NULL, SCOPE_GLOBAL, "Global");
    Scope* current_scope = global_scope;
    char line[256];
    int line_number = 0;

    while (fgets(line, sizeof(line), file)) {
        line_number++;
        char* trimmed_line = trim(line);
        if (strlen(trimmed_line) == 0 || trimmed_line[0] == '/' || trimmed_line[0] == '#') {
            continue; // Bỏ qua comment và preprocessor directives
        }

        // Kiểm tra khai báo biến
        if (strstr(trimmed_line, "int ") != NULL || strstr(trimmed_line, "char ") != NULL || 
            strstr(trimmed_line, "float ") != NULL || strstr(trimmed_line, "double ") != NULL) {
            
            char type[32], name[64], value[64] = "";
            if (sscanf(trimmed_line, "%s %s = %[^;];", type, name, value) >= 2) {
                // Tạo biến mới
                Variable* new_var = create_variable(type, name, value);
                if (new_var != NULL) {
                    // Kiểm tra biến hợp lệ
                    if (!is_undefined_variable(current_scope, name)) {
                        add_variable_to_current_scope(current_scope, new_var);
                    } else {
                        printf("Error at line %d: Variable '%s' is already defined\n", line_number, name);
                        free(new_var);
                    }
                }
            }
        }

        // Kiểm tra bắt đầu phạm vi mới
        if (strstr(trimmed_line, "{") != NULL) {
            if (strstr(trimmed_line, "void") != NULL || strstr(trimmed_line, "int") != NULL) {
                // Phạm vi hàm
                char func_name[64];
                if (sscanf(trimmed_line, "%*s %s", func_name) == 1) {
                    current_scope = create_scope(current_scope, SCOPE_FUNCTION, func_name);
                }
            } else {
                // Phạm vi cục bộ
                char scope_name[64];
                snprintf(scope_name, sizeof(scope_name), "Local_%d", line_number);
                current_scope = create_scope(current_scope, SCOPE_LOCAL, scope_name);
            }
        }

        // Kiểm tra kết thúc phạm vi
        if (strstr(trimmed_line, "}") != NULL) {
            if (current_scope != global_scope) {
                current_scope = current_scope->parent;
            }
        }
    }

    fclose(file);

    // Kiểm tra shadowing
    is_shadowing(global_scope);

    // In cây phạm vi
    printf("\nScope Tree Structure:\n");
    printf("===================\n");
    print_scope_tree(global_scope, 0);

    return 0;
}

int main() {
    parse_file_c("../input/input1.c");
    return 0;
}