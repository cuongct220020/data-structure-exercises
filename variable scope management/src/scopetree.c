#include <stdio.h>
#include <stdlib.h>
#include "scopetree.h"
#include "helper.h"

// Hàm tạo biến mới
Variable* create_variable(const char* type, const char* name, const char* value) {
    Variable* new_var = (Variable*)malloc(sizeof(Variable));
    if (new_var == NULL) {
        perror("[create_variable] Lỗi cấp phát động.\n");
        return NULL;
    }
    new_var->type = strdup(type);
    new_var->name = strdup(name);
    new_var->value = (value != NULL) ? strdup(value) : NULL;
    new_var->next = NULL;
    return new_var;
}

// Hàm thêm biến vào phạm vi
void add_variable_to_current_scope(Scope* current_scope, Variable* new_var) {
    if (current_scope == NULL || new_var == NULL) {
        printf("[add_variable_to_current_scope] Phạm vi biến và biến mới không hợp lệ.\n");
        return;
    }
    printf("[DEBUG] Add variable: %s (%p) to scope %s\n", new_var->name, (void*)new_var, current_scope->scope_name);

    // Add to current scope
    if (current_scope->list_var == NULL) {
        current_scope->list_var = new_var;
    } else {
        Variable* current = current_scope->list_var;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_var;
    }
    current_scope->num_variable++;
}

// Tìm biến trong phạm vi hiện tại
Variable* find_variable_in_current_scope(Scope* scope, const char* var_name) {
    if (scope == NULL) {
        printf("[find_variale_in_current_scope] Phạm vi của biến không hợp lệ");
        return NULL;
    }
    Variable* current_var = scope->list_var;
    while (current_var != NULL) {
        if (strcmp(current_var->name, var_name) == 0) {
            return current_var;
        }
        current_var = current_var->next;
    }
    return NULL;
}

// Kiểm tra tính hợp lệ của biến
int is_undefined_variable(Scope* current_scope, const char* var_name) {
    Scope* temp_scope = current_scope;
    while (temp_scope != NULL) {
        Variable* found_var = find_variable_in_current_scope(temp_scope, var_name);
        if (found_var != NULL) {
            printf("[is_undefined_variable] Biến %s đã được định nghĩa trước đó.\n", var_name);
            return 0; // Hợp lệ, đã tìm thấy biến
        }
        temp_scope = temp_scope->parent; // Đi lên nút cha 
    }
    printf("[is_undefined_variable] Lỗi: undefined variable. Biến %s chưa được định nghĩa.\n", var_name);
    return 1; // Lỗi, biến không được định nghĩa ở bất kỳ scope nào
}

void is_shadowing(Scope* current_scope) {
    if (current_scope == NULL || current_scope->parent == NULL) {
        printf("[is_shadowing] Phạm vi hiện tại hoặc phạm vi cha không hợp lệ.\n");
        return;
    }

    Variable* current_var = current_scope->list_var;
    while (current_var != NULL) {
        // Tìm biến cùng tên trong các phạm vi cha
        Scope* temp_parent_scope = current_scope->parent;
        while (temp_parent_scope != NULL) {
            Variable* parent_var = find_variable_in_current_scope(temp_parent_scope, current_var->name);
            if (parent_var != NULL) {
                // Phát hiện shadowing nếu một biến ở phạm vi con trùng tên với phạm vi cha 
                printf("[is_shadowing] Cảnh báo: Phát hiện che khuất (shadowing)! Biến '%s' trong phạm vi '%s' che khuất biến trong phạm vi '%s'.\n", current_var->name, current_scope->scope_name, temp_parent_scope->scope_name);
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

// Hàm tạo một phạm vi mới
Scope* create_scope(Scope* parent_scope, ScopeType scope_type, const char* scope_name) {
    Scope* new_scope = (Scope*)malloc(sizeof(Scope));
    if (new_scope == NULL) {
        printf("[create_scope] Lỗi cấp phát động.\n");
        return NULL;
    }
    new_scope->scope_name = strdup(scope_name);
    new_scope->scope_type = scope_type;
    new_scope->num_variable = 0;
    new_scope->list_var = NULL;
    new_scope->first_child = NULL;
    new_scope->next_sibling = NULL;

    if (parent_scope != NULL) {
        new_scope->parent = parent_scope;
    }
    
    // Gắn vào danh sách con của parent
    if (parent_scope != NULL) {
        if (parent_scope->first_child == NULL) {
            parent_scope->first_child = new_scope;
        } else {
            Scope* last_child = parent_scope->first_child;
            while (last_child->next_sibling != NULL) {
                last_child = last_child->next_sibling;
            }
            last_child->next_sibling = new_scope;
        }
    }

    return new_scope;
}

void print_scope_tree(Scope* scope, int level) {
    if (scope == NULL) {
        return;
    }

    print_indentation(level);
    printf("[%s] (Scope: %s)\n", scope->scope_name, scope->scope_type == SCOPE_GLOBAL ? "Global" : (scope->scope_type == SCOPE_FUNCTION ? "Function" : "Local"));

    // In các biến trong phạm vi hiện tại
    Variable* current_var = scope->list_var;
    while (current_var != NULL) {
        print_indentation(level + 1); // Thụt lề thêm cho biến
        printf("- %s = %s (Type: %s)\n",
               current_var->name,
               current_var->value ? current_var->value : "N/A",
               current_var->type);
        current_var = current_var->next;
    }

    // In các phạm vi con
    Scope* current_child = scope->first_child;
    while (current_child != NULL) {
        print_scope_tree(current_child, level + 1);
        current_child = current_child->next_sibling;
    }
}











