#include <stdio.h>

// Định nghĩa struct cho thuộc tính của thẻ XML
typedef struct Attribute {
    char* name; // Tên thuộc tính                  
    char* value; // Giá trị thuộc tính                  
    struct Attribute* next;
} Attribute;

// Định nghĩa struct cho một node trong cây XML
typedef struct TreeNode {
    char* tag_name;  // key của thẻ XML            
    Attribute* attributes; 
    char* text;
    struct TreeNode* first_child;
    struct TreeNode* next_sibling;
    struct TreeNode* parent;
} TreeNode;

// ==== Các hàm kiểm tra file XML đầu vào ===
int is_valid_tag(const char* tag_name); // Kiểm tra tính hợp lệ của các thẻ
int is_valid_xml_file(const char* filename); // Hàm kiểm tra tính hợp lệ của file XML

// ==== Các hàm thao tác với cây XML === 
// 1. Tạonode mới
TreeNode* create_node(const char* tag_name);

// 2. Thêm và xoá node trong cây XML
void add_tag(TreeNode* parent, const char* tag_name); // Thêm thẻ
void delete_tag(TreeNode* node); // Xoá thẻ

// 3. Thay đổi giá trị thuộc tính của node
void change_attribute(TreeNode* node, const char* attr_name, const char* new_value);

// 4. Thêm thuộc tính vào node
void add_attribute(TreeNode* node, const char* attr_name, const char* value);

// 5. Tìm kiếm/in ra giá trị của 1 key (tag_name). Trả về 1 nếu tìm thấy, 0 nếu không tìm thấy.
int search_and_print_tag(TreeNode* root, const char* tag_name);

// 6. Tìm kiếm và in ra nội dung của 1 thẻ. Trả về 1 nếu tìm thấy, 0 nếu không tìm thấy.
int search_and_print(TreeNode* root, const char* tag_name);

// 7. Giải phóng bộ nhớ của cây XML, xoá toàn bộ các node
void delete_child_by_tag_name(TreeNode* parent, const char* tag_name);
void free_xml_tree(TreeNode* root);

// === Ghi cấu trúc của cây XML ra file ===
void write_tag(FILE* file, TreeNode* node, int indent);
void write_xml_file(const char* filename, TreeNode* root);