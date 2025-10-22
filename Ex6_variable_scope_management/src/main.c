#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "scopetree.h"

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

// Hàm kiểm tra xem có phải là khai báo hàm không
int is_function_declaration(const char* line) {
    if (strchr(line, '(') != NULL && strchr(line, ')') != NULL) {
        if (strstr(line, "void ") == line || strstr(line, "int ") == line || 
            strstr(line, "char ") == line || strstr(line, "float ") == line || 
            strstr(line, "double ") == line) {
            return 1;
        }
    }
    return 0;
}

// Hàm kiểm tra xem một từ có phải là tên biến hợp lệ không
int is_valid_varname(const char* word) {
    if (!isalpha(word[0]) && word[0] != '_') return 0;
    for (int i = 1; word[i]; ++i) {
        if (!isalnum(word[i]) && word[i] != '_') return 0;
    }
    // Loại trừ các từ khóa kiểu dữ liệu
    if (strcmp(word, "int") == 0 || strcmp(word, "char") == 0 || strcmp(word, "float") == 0 || strcmp(word, "double") == 0 || strcmp(word, "void") == 0 || strcmp(word, "return") == 0)
        return 0;
    return 1;
}

// Remove string literals from a line (replace with spaces, keep quotes)
void remove_string_literals(char* line) {
    int in_string = 0;
    for (int i = 0; line[i]; ++i) {
        if (line[i] == '"') in_string = !in_string;
        else if (in_string) line[i] = ' ';
    }
}

// Improved undefined variable checker
void check_undefined_vars(Scope* current_scope, const char* line) {
    char buf[256];
    strncpy(buf, line, sizeof(buf));
    buf[sizeof(buf)-1] = '\0';
    remove_string_literals(buf);

    // If this is a function call, only check variables after the first comma (after format string)
    char* paren = strchr(buf, '(');
    if (paren != NULL) {
        char* first_comma = strchr(paren, ',');
        if (first_comma != NULL) {
            // Only check after the first comma
            char* args = first_comma + 1;
            char* token = strtok(args, " ,;()=+-*/%\t\n'[]{}");
            while (token != NULL) {
                if (is_valid_varname(token)) {
                    printf("[is_undefined_variable] Lỗi: undefined variable. Biến %s chưa được định nghĩa.\n", token);
                }
                token = strtok(NULL, " ,;()=+-*/%\t\n'[]{}");
            }
            return;
        }
    }
    // Otherwise, check the whole line (with string literals removed)
    char* token = strtok(buf, " ,;()=+-*/%\t\n'[]{}");
    while (token != NULL) {
        if (is_valid_varname(token)) {
            printf("[is_undefined_variable] Lỗi: undefined variable. Biến %s chưa được định nghĩa.\n", token);
        }
        token = strtok(NULL, " ,;()=+-*/%\t\n'[]{}");
    }
}

// Hàm phân tích file .c và kiểm tra tính hợp lệ 
int parse_file_c(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Cannot open file %s\n", filename);
        return -1;
    }
    Scope* global_scope = create_scope(NULL, SCOPE_GLOBAL, "Global");
    if (global_scope == NULL) {
        fclose(file);
        return -1;
    }
    Scope* current_scope = global_scope;
    char line[256];
    int line_number = 0;
    char last_function_line[256] = ""; // Lưu dòng khai báo hàm
    int skip_next_brace = 0; // Đánh dấu nếu đã xử lý { trên dòng khai báo hàm

    while (fgets(line, sizeof(line), file)) {
        line_number++;
        char* trimmed_line = trim(line);
        if (strlen(trimmed_line) == 0 || trimmed_line[0] == '/' || trimmed_line[0] == '#') {
            continue;
        }

        // Kiểm tra khai báo hàm
        if (is_function_declaration(trimmed_line)) {
            if (strchr(trimmed_line, '{') != NULL) {
                char func_name[64];
                char* open_paren = strchr(trimmed_line, '(');
                if (open_paren != NULL) {
                    int name_len = open_paren - trimmed_line;
                    if (name_len >= sizeof(func_name)) continue;
                    strncpy(func_name, trimmed_line, name_len);
                    func_name[name_len] = '\0';
                    char* last_space = strrchr(func_name, ' ');
                    if (last_space != NULL && (last_space + 1) < (func_name + sizeof(func_name))) {
                        memmove(func_name, last_space + 1, strlen(last_space + 1) + 1);
                    }
                    current_scope = create_scope(current_scope, SCOPE_FUNCTION, func_name);
                    if (current_scope == NULL) continue;
                    skip_next_brace = 1; // Đánh dấu đã xử lý {
                }
            } else {
                strcpy(last_function_line, trimmed_line);
                continue;
            }
        } else {
            // Kiểm tra khai báo biến (không phải hàm)
            if ((strstr(trimmed_line, "int ") != NULL || strstr(trimmed_line, "char ") != NULL || 
                 strstr(trimmed_line, "float ") != NULL || strstr(trimmed_line, "double ") != NULL) &&
                !is_function_declaration(trimmed_line)) {
                char type[32], name[64], value[64] = "";
                if (sscanf(trimmed_line, "%s %s = %[^;];", type, name, value) >= 2) {
                    Variable* new_var = create_variable(type, name, value);
                    if (new_var != NULL) {
                        Variable* existing_var = find_variable_in_current_scope(current_scope, name);
                        if (existing_var == NULL) {
                            add_variable_to_current_scope(current_scope, new_var);
                        } else {
                            printf("Error at line %d: Variable '%s' is already defined in current scope\n", line_number, name);
                            free_variable(new_var);
                        }
                    }
                }
            } else {
                // Kiểm tra biến sử dụng nhưng chưa khai báo (chỉ với các dòng không phải khai báo biến)
                check_undefined_vars(current_scope, trimmed_line);
            }
        }

        // Kiểm tra bắt đầu phạm vi mới
        if (strstr(trimmed_line, "{") != NULL) {
            if (skip_next_brace) {
                skip_next_brace = 0;
            } else if (strlen(last_function_line) > 0) {
                char func_name[64];
                char* open_paren = strchr(last_function_line, '(');
                if (open_paren != NULL) {
                    int name_len = open_paren - last_function_line;
                    if (name_len >= sizeof(func_name)) { last_function_line[0] = '\0'; continue; }
                    strncpy(func_name, last_function_line, name_len);
                    func_name[name_len] = '\0';
                    char* last_space = strrchr(func_name, ' ');
                    if (last_space != NULL && (last_space + 1) < (func_name + sizeof(func_name))) {
                        memmove(func_name, last_space + 1, strlen(last_space + 1) + 1);
                    }
                    current_scope = create_scope(current_scope, SCOPE_FUNCTION, func_name);
                    if (current_scope == NULL) { last_function_line[0] = '\0'; continue; }
                    last_function_line[0] = '\0';
                }
            } else {
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
    is_shadowing(global_scope);
    printf("\nScope Tree Structure:\n");
    printf("===================\n");
    print_scope_tree(global_scope, 0);
    return 0;
}

int main() {
    parse_file_c("../input/input1.c");
    return 0;
}