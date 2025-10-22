#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "TagStack.h"
#include "XMLTree.h"

int main() {
    printf("Starting XML validation...\n");
    
    // Đọc file XML và kiểm tra tính hợp lệ
    if (is_valid_xml_file("../input/xml_input.txt")) {
        printf("File XML là hợp lệ.\n");
    } else {
        printf("File XML không hợp lệ.\n");
    }

    // Tạo một cây XML mẫu
    printf("\nTạo cây XML mẫu...\n");
    TreeNode* root = create_node("library");
    add_tag(root, "book");
    add_tag(root, "magazine");
    add_tag(root->first_child, "title");
    add_tag(root->first_child, "author");
    add_tag(root->first_child->next_sibling, "title");
    add_tag(root->first_child->next_sibling, "editor");

    // Thêm thuộc tính cho các node
    add_attribute(root, "location", "Hanoi");
    add_attribute(root->first_child, "id", "b001");
    add_attribute(root->first_child->next_sibling, "id", "m001");
    add_attribute(root->first_child->first_child, "lang", "en");
    add_attribute(root->first_child->next_sibling->first_child, "lang", "vn");

    // Thêm nội dung cho các node
    root->first_child->first_child->text = strdup("C Programming");
    root->first_child->first_child->next_sibling->text = strdup("Nguyen Van A");
    root->first_child->next_sibling->first_child->text = strdup("Tech Magazine");
    root->first_child->next_sibling->first_child->next_sibling->text = strdup("Le Thi B");

    // Thay đổi giá trị thuộc tính
    change_attribute(root->first_child, "id", "b002");

    // Tìm kiếm và in giá trị của 1 key (tag_name)
    printf("\nTìm kiếm và in giá trị của thẻ 'author':\n");
    search_and_print_tag(root, "author");

    // Tìm kiếm và in nội dung của 1 thẻ
    printf("\nTìm kiếm và in nội dung của thẻ 'title':\n");
    search_and_print(root, "title");

    //Xoá một node (ví dụ: xoá magazine)
    printf("\nXoá thẻ 'magazine'...\n");
    delete_child_by_tag_name(root, "magazine");

    // Ghi cây XML ra file
    printf("\nGhi cây XML ra file...\n");
    write_xml_file("../output/output.xml", root);

    // Giải phóng bộ nhớ
    printf("\nGiải phóng bộ nhớ...\n");
    free_xml_tree(root);
    return 0;
}

