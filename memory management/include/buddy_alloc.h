#ifndef BUDDY_ALLOC_H
#define BUDDY_ALLOC_H

#include <stdio.h>

// Cấu trúc cho Buddy Block
typedef struct BuddyBlock {
    void* start_addr;
    size_t size;
    int is_free;
    int level; // Giữ thông tin mức của buddy (log2(size))
    size_t actual_allocated_size;

    struct BuddyBlock* parent;
    struct BuddyBlock* leftChild;
    struct BuddyBlock* rightChild;
} BuddyBlock;

// Cấu trúc cho Buddy System 
typedef struct BuddySystem {
    BuddyBlock* root;
    size_t total_memory_size;
    size_t allocated_memory_size;
    int max_level;
    void* base_memory_address;
    void* last_memory_address;
} BuddySystem;

// Hàm cấp phát hệ thống Buddy
BuddySystem* create_buddy_system(void *base_addr, size_t total_size, FILE *out);

// Hàm cấp phát bộ nhớ
BuddyBlock* buddy_malloc(BuddySystem *system, size_t size, FILE *out);

// Tìm và cấp phát một block buddy thoả mãn yêu cầu
BuddyBlock* find_and_allocate(BuddyBlock *root, size_t request_size, FILE* out);

// Hàm chia thành hai buddy
BuddyBlock* split_block(BuddyBlock* block, FILE *out);

// Hàm hợp nhất hai buddy
BuddyBlock* merge_buddies(BuddyBlock* block, FILE *out);

// Hàm giải phóng một khối buddy
void free_buddy(BuddySystem *system, BuddyBlock* block, FILE *out);

// Hàm giải phóng toàn bộ hệ thống Buddy
void cleanup_buddy_system(BuddySystem *system, FILE *out);

// Hàm in ra thông tin hệ thống
void print_buddy_system(BuddySystem *system, FILE *out);


#endif