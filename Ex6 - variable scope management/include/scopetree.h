#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    SCOPE_GLOBAL, // Phạm vi toàn cục
    SCOPE_LOCAL,  // Phạm vi cục bộ
    SCOPE_FUNCTION, // Phạm vi hàm
} ScopeType;

// Định nghĩa struct cho một biến (variable)
typedef struct Variable {
    char* type; // Kiểu của biến
    char* name; // Tên của biến
    char* value; // Giá trị của biến
    struct Variable* next; // Con trỏ đến biến tiếp theo trong danh sách
} Variable;

// Định nghĩa struct cho một phạm vi (scope)
typedef struct Scope {
    char* scope_name; // Tên của phạm vi
    ScopeType scope_type; // Kiểu phạm vi (toàn cục, cục bộ, hàm)
    int num_variable; // Số lượng biến trong phạm vi
    struct Variable* list_var;
    struct Scope* first_child;
    struct Scope* next_sibling;
    struct Scope* parent;
} Scope;


Variable* create_variable(const char* type, const char* name, const char* value);

void add_variable_to_current_scope(Scope* current_scope, Variable* new_var);
Variable* find_variable_in_current_scope(Scope* scope, const char* var_name);

Scope* create_scope(Scope* parent_scope, ScopeType scope_type, const char* scope_name);
int is_undefined_variable(Scope* current_scope, const char* var_name);
void is_shadowing(Scope* current_scope);

void print_scope_tree(Scope* scope, int level);

void free_variable(Variable* var);


