#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "TagStack.h"

// Khởi tạo stack
void init_stack(TagStack* stack) {
    stack->top = NULL;
}

// Đẩy tên thẻ vào stack
int push(TagStack* stack, const char* tag) {
    TagNode* node = (TagNode*)malloc(sizeof(TagNode));
    if (!node) {
        printf("[push] Lỗi cấp phát bộ nhớ cho node.\n");
        return 0; // Lỗi cấp phát bộ nhớ
    }
    node->tag = strdup(tag);
    node->next = stack->top;
    stack->top = node;
    return 1;
}

// Lấy tên thẻ trên đỉnh stack ra (và giải phóng bộ nhớ)
char* pop(TagStack* stack) {
    if (!stack->top) {
        printf("[pop] Lỗi: Stack rỗng.\n");
        return NULL; // Stack rỗng
    }
    TagNode* node = stack->top;
    char* tag = node->tag;
    stack->top = node->next;
    free(node);
    return tag;
}

// Xem tên thẻ trên đỉnh stack mà không lấy ra
char* peek(TagStack* stack) {
    if (!stack->top) {
        printf("[peek] Lỗi: Stack rỗng.\n");
        return NULL; // Stack rỗng
    }
    return stack->top->tag;
}

// Kiểm tra stack rỗng
int is_empty(TagStack* stack) {
    return stack->top == NULL;
}

// Giải phóng toàn bộ stack
void free_stack(TagStack* stack) {
    while (!is_empty(stack)) {
        free(pop(stack));
    }
}