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

// Tạo biến mới
Variable* create_variable(const char *type, const char* name, const char* value);

// Tạo phạm vi mới
Scope* create_scope(Scope *parent_scope, ScopeType scope_type, const char* scope_name);

// Thêm biến vào phạm vi hiện tại
void add_variable_to_current_scope(Scope* current_scope, Variable* new_var);

// Hàm tìm biến trong phạm vi hiện tại
Variable* find_variable_in_current_scope(Scope* scope, const char* var_name);

// Hàm phát hiện lỗi phạm vi (undefined variable). Trả về 1 nếu biến không được định nghĩa, 0 nếu ngược lại
int is_undefined_variable(Scope* current_scope, const char* var_name);

// Hàm phát hiện shadowing (biến bị che khuất). Kiểm tra tất cả các biến trong phạm vi 'current_scope' và trong phạm vi cha. 
void is_shadowing(Scope* current_scope);

// In ra cấu trúc cây phạm vi của biến.
void print_scope_tree(Scope* scope, int level);

