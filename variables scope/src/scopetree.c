#include <stdio.h>
#include <stdlib.h>
#include "scopetree.h"

// Hàm tạo biến mới
Variable* create_variable(const char* name, const char* value, int is_constant, int is_static) {
    Variable* new_var = (Variable*)malloc(sizeof(Variable));
    if (new_var == NULL) {
        perror("Lỗi cấp phát động");
        return NULL;
    }
    new_var->name = strdup(name);
    new_var->value = (value != NULL) ? strdup(value) : NULL;
    new_var->is_constant = is_constant;
    new_var->is_static = is_static;
    new_var->next = NULL;
    return new_var;
}

// Hàm hỗ trợ: Tạo một phạm vi mới
Scope* create_scope(ScopeType type, const char* scope_name, Scope* parent) {
    Scope* new_scope = (Scope*)malloc(sizeof(Scope));
    if (new_scope == NULL) {
        perror("Failed to allocate memory for Scope");
        exit(EXIT_FAILURE);
    }
    new_scope->scope_name = strdup(scope_name);
    new_scope->num_variable = 0;
    new_scope->var = NULL;
    new_scope->first_child = NULL;
    new_scope->next_sibling = NULL;
    new_scope->parent = parent;

    // Thêm phạm vi mới vào danh sách con của cha
    if (parent != NULL) {
        if (parent->first_child == NULL) {
            parent->first_child = new_scope;
        } else {
            Scope* sibling = parent->first_child;
            while (sibling->next_sibling != NULL) {
                sibling = sibling->next_sibling;
            }
            sibling->next_sibling = new_scope;
        }
    }
    return new_scope;
}

// Hàm hỗ trợ: Thêm biến vào phạm vi
void add_variable_to_scope(Scope* scope, Variable* new_var) {
    if (scope == NULL || new_var == NULL) {
        return;
    }
    new_var->next = scope->var;
    scope->var = new_var;
    scope->num_variable++;
}

// Hàm hỗ trợ: Tìm biến trong phạm vi hiện tại
Variable* find_variable_in_current_scope(Scope* scope, const char* var_name) {
    if (scope == NULL) {
        return NULL;
    }
    Variable* current_var = scope->var;
    while (current_var != NULL) {
        if (strcmp(current_var->name, var_name) == 0) {
            return current_var;
        }
        current_var = current_var->next;
    }
    return NULL;
}

// Hàm hỗ trợ: Giải phóng bộ nhớ của biến
void free_variable(Variable* var) {
    if (var == NULL) return;
    free(var->name);
    if (var->value != NULL) {
        free(var->value);
    }
    free(var);
}

// Hàm hỗ trợ: Giải phóng bộ nhớ của phạm vi (và các con của nó)
void free_scope(Scope* scope) {
    if (scope == NULL) return;

    // Giải phóng biến trong phạm vi hiện tại
    Variable* current_var = scope->var;
    while (current_var != NULL) {
        Variable* next_var = current_var->next;
        free_variable(current_var);
        current_var = next_var;
    }

    // Giải phóng các phạm vi con
    Scope* current_child = scope->first_child;
    while (current_child != NULL) {
        Scope* next_child = current_child->next_sibling;
        free_scope(current_child);
        current_child = next_child;
    }

    free(scope->scope_name);
    free(scope);
}

// Kiểm tra tính hợp lệ của biến trong phạm vi (Variable Resolution)
// Trả về con trỏ đến biến nếu tìm thấy, ngược lại trả về NULL.
Variable* is_valid_variable(Scope* current_scope, const char* var_name) {
    Scope* temp_scope = current_scope;
    while (temp_scope != NULL) {
        Variable* found_var = find_variable_in_current_scope(temp_scope, var_name);
        if (found_var != NULL) {
            return found_var; // Tìm thấy biến 
        }
        temp_scope = temp_scope->parent; // Đi lên nút cha 
    }
    return NULL; // Không tìm thấy biến ở bất kỳ phạm vi cha nào 
}

// Phát hiện lỗi phạm vi (undefined variable)
// Trả về 1 nếu biến không được định nghĩa, 0 nếu tìm thấy.
int is_undefined_variable(Scope* current_scope, const char* var_name) {
    if (is_valid_variable(current_scope, var_name) == NULL) {
        return 1; // Biến không tồn tại 
    }
    return 0;
}


void is_shadowing(Scope* current_scope) {
    if (current_scope == NULL || current_scope->parent == NULL) {
        return; // Không thể có shadowing nếu không có phạm vi cha
    }

    Variable* current_var = current_scope->var;
    while (current_var != NULL) {
        // Tìm biến cùng tên trong các phạm vi cha
        Scope* temp_parent_scope = current_scope->parent;
        while (temp_parent_scope != NULL) {
            Variable* parent_var = find_variable_in_current_scope(temp_parent_scope, current_var->name);
            if (parent_var != NULL) {
                // Phát hiện shadowing nếu một biến ở phạm vi con trùng tên với phạm vi cha 
                printf("Warning: Shadowing detected! Variable '%s' in scope '%s' shadows variable in scope '%s'.\n",
                       current_var->name, current_scope->scope_name, temp_parent_scope->scope_name);
                break; // Đã tìm thấy shadowing cho biến này, chuyển sang biến tiếp theo
            }
            temp_parent_scope = temp_parent_scope->parent;
        }
        current_var = current_var->next;
    }

    // Đệ quy kiểm tra shadowing cho các phạm vi con
    Scope* child_scope = current_scope->first_child;
    while (child_scope != NULL) {
        is_shadowing(child_scope);
        child_scope = child_scope->next_sibling;
    }
}

// Hàm hỗ trợ: In thụt lề cho cấu trúc cây
void print_indentation(int level) {
    for (int i = 0; i < level; i++) {
        printf("  "); // 2 khoảng trắng cho mỗi cấp độ
    }
}

void print_scope_tree(Scope* scope, int level) {
    if (scope == NULL) {
        return;
    }

    print_indentation(level);
    //printf("[%s] (Scope: %s)\n", scope->scope_name, scope->type == SCOPE_GLOBAL ? "Global" : (scope->type == SCOPE_FUNCTION ? "Function" : "Local"));

    // In các biến trong phạm vi hiện tại
    Variable* current_var = scope->var;
    while (current_var != NULL) {
        print_indentation(level + 1); // Thụt lề thêm cho biến
        printf("- %s = %s (Type: %d, Const: %d, Static: %d)\n",
               current_var->name,
               current_var->value ? current_var->value : "N/A",
               current_var->type,
               current_var->is_constant,
               current_var->is_static);
        current_var = current_var->next;
    }

    // In các phạm vi con
    Scope* current_child = scope->first_child;
    while (current_child != NULL) {
        print_scope_tree(current_child, level + 1);
        current_child = current_child->next_sibling;
    }
}