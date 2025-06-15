#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    SCOPE_GLOBAL, // Phạm vi toàn cục
    SCOPE_LOCAL,  // Phạm vi cục bộ
    SCOPE_FUNCTION, // Phạm vi hàm
    SCOPE_CLASS // Phạm vi class
} ScopeType;

// Định nghĩa struct cho một biến (variable)
typedef struct Variable {
    char* type; // Kiểu của biến
    char* name; // Tên của biến
    char* value; // Giá trị của biến
    int is_constant; // Biến có phải là hằng số hay không
    int is_static; // Biến có phải tĩnh hay không
    struct Variable* next; // Con trỏ đến biến tiếp theo trong danh sách
} Variable;

// Định nghĩa struct cho một phạm vi (scope)
typedef struct Scope {
    char* scope_name; // Tên của phạm vi
    ScopeType scope_type; // Kiểu phạm vi (toàn cục, cục bộ, hàm)
    int num_variable; // Số lượng biến trong phạm vi
    struct Variable* var;
    struct Scope* first_child;
    struct Scope* next_sibling;
    struct Scope* parent;
} Scope;

Variable* create_variable(const char* name, const char* value, int is_constant, int is_static);

// Tạo phạm vi 
Scope* create_scope(ScopeType type, const char* scope_name, Scope* parent);

// Thêm biến vào phạm vi
void add_variable_to_scope(Scope* scope, Variable* new_var);

// Hàm tìm biến trong phạm vi hiện tại
Variable* find_variable_in_current_scope(Scope* scope, const char* var_name);


int is_valid_variable(Scope* current_scope, const char* var_name);


// Phát hiện lỗi phạm vi (undefined variabel)
int is_undefined_variable(Scope* current_scope, const char* var_name);

// Phát hiện shadowing (biến bị che khuất)
// Kiểm tra tất cả các biến trong phạm vi 'current_scope' và trong phạm vi cha
int is_shadowing(Scope* current_scope);

// In ra cấu trúc cây phạm vi của biến.
void print_scope_tree(Scope* scope, int level);


