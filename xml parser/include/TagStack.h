#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Node cho từng phần tử stack
typedef struct TagNode {
    char* tag;
    struct TagNode* next;
} TagNode;

// Stack dùng danh sách liên kết đơn
typedef struct {
    TagNode* top;
} TagStack;

// Khởi tạo stack
void init_stack(TagStack* stack);

// Đẩy tên thẻ vào stack
int push(TagStack* stack, const char* tag);

// Lấy tên thẻ trên đỉnh stack ra (và giải phóng bộ nhớ)
char* pop(TagStack* stack);

// Xem tên thẻ trên đỉnh stack mà không lấy ra
char* peek(TagStack* stack);

// Kiểm tra stack rỗng
int is_empty(TagStack* stack);

// Giải phóng toàn bộ stack
void free_stack(TagStack* stack);