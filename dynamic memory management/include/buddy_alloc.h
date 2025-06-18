#ifndef BUDDY_ALLOC_H
#define BUDDY_ALLOC_H

#include <stdio.h>

// Cấu trúc cho buddy system
typedef struct BuddyBlock {
    void* start_addr;
    size_t size;
    int is_free;
    struct BuddyBlock* buddy;
    struct BuddyBlock* next;
} BuddyBlock;

// Cấu trúc riêng cho buddy system
typedef struct BuddySystem {
    BuddyBlock** free_lists; // Mảng các danh sách free blocks theo kích thước (power of 2)
    int max_order; // Số lượng các danh sách free blocks (log2 của kích thước lớn nhất)
    size_t total_memory_size; // Tổng vùng trống tự do ban đầu
    size_t allocated_memory_size; // Tổng vùng trống đã cấp phát
    void *base_memory_address; // Con trỏ giữ địa chỉ bắt đầu của vùng trống tự do
    void *last_memory_address; // Con trỏ giữ địa chỉ kết thúc của vùng trống tự do
} BuddySystem;

// Các hàm mới cho buddy system
BuddySystem* create_buddy_system(void *base_addr, size_t total_size, FILE *out);
void cleanup_buddy_system(BuddySystem *system, FILE *out);
void* buddy_malloc(BuddySystem *system, size_t size, FILE *out);
void buddy_free(BuddySystem *system, void *ptr, size_t size, FILE *out);
void print_buddy_free_lists(BuddySystem *system, FILE *out);
int get_order(size_t size);
BuddyBlock* split_block(BuddySystem *system, BuddyBlock* block, int current_order, FILE *out);
void merge_buddies(BuddySystem *system, BuddyBlock* block, int order, FILE *out);

#endif