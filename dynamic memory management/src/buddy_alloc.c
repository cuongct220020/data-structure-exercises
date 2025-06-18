#include <stdio.h>
#include <stdlib.h>
#include "buddy_alloc.h"

// Hàm tính order (log2) của một kích thước
int get_order(size_t size) {
    int order = 0;
    size_t current_size = 1;
    
    while (current_size < size) {
        current_size *= 2;
        order++;
    }
    
    return order;
}

// Tạo và khởi tạo buddy system
BuddySystem* create_buddy_system(void *base_addr, size_t total_size, FILE *out) {
    if (base_addr == NULL || total_size == 0) {
        fprintf(out, "[create_buddy_system] Lỗi: Tham số không hợp lệ.\n");
        return NULL;
    }

    BuddySystem* system = (BuddySystem*)malloc(sizeof(BuddySystem));
    if (system == NULL) {
        fprintf(out, "[create_buddy_system] Lỗi: Không thể cấp phát BuddySystem.\n");
        return NULL;
    }

    // Tính max_order dựa trên tổng kích thước bộ nhớ
    system->max_order = get_order(total_size);
    system->base_memory_address = base_addr;
    system->total_memory_size = total_size;
    system->last_memory_address = (char*)base_addr + total_size;
    system->allocated_memory_size = 0;
    
    // Cấp phát mảng các danh sách free blocks
    system->free_lists = (BuddyBlock**)calloc(system->max_order + 1, sizeof(BuddyBlock*));
    if (system->free_lists == NULL) {
        fprintf(out, "[create_buddy_system] Lỗi: Không thể cấp phát free_lists.\n");
        free(system);
        return NULL;
    }

    // Tạo block ban đầu với kích thước lớn nhất
    BuddyBlock* initial_block = (BuddyBlock*)malloc(sizeof(BuddyBlock));
    if (initial_block == NULL) {
        fprintf(out, "[create_buddy_system] Lỗi: Không thể cấp phát initial_block.\n");
        free(system->free_lists);
        free(system);
        return NULL;
    }

    initial_block->start_addr = base_addr;
    initial_block->size = total_size;
    initial_block->is_free = 1;
    initial_block->buddy = NULL;
    initial_block->next = NULL;

    // Thêm block ban đầu vào danh sách free blocks có kích thước lớn nhất
    system->free_lists[system->max_order] = initial_block;

    fprintf(out, "[create_buddy_system] Đã khởi tạo buddy system với max_order = %d\n", system->max_order);
    return system;
}

// Giải phóng buddy system
void cleanup_buddy_system(BuddySystem *system, FILE *out) {
    if (system == NULL) {
        fprintf(out, "[cleanup_buddy_system] Con trỏ system trỏ tới NULL.\n");
        return;
    }

    if (system->free_lists != NULL) {
        // Giải phóng tất cả các blocks trong các danh sách free
        for (int i = 0; i <= system->max_order; i++) {
            BuddyBlock* current = system->free_lists[i];
            while (current != NULL) {
                BuddyBlock* temp = current;
                current = current->next;
                free(temp);
            }
        }

        // Giải phóng mảng free_lists
        free(system->free_lists);
    }

    free(system);
}

// Tách một block thành hai buddies
BuddyBlock* split_block(BuddySystem *system, BuddyBlock* block, int current_order, FILE *out) {
    if (block == NULL || current_order <= 0) {
        return NULL;
    }

    // Tính kích thước mới cho các buddies
    size_t new_size = block->size / 2;
    int new_order = current_order - 1;

    // Tạo buddy mới
    BuddyBlock* buddy = (BuddyBlock*)malloc(sizeof(BuddyBlock));
    if (buddy == NULL) {
        fprintf(out, "[split_block] Lỗi: Không thể cấp phát buddy block.\n");
        return NULL;
    }

    // Cập nhật thông tin cho buddy mới
    buddy->start_addr = (char*)block->start_addr + new_size;
    buddy->size = new_size;
    buddy->is_free = 1;
    buddy->buddy = block;
    buddy->next = NULL;

    // Cập nhật thông tin cho block hiện tại
    block->size = new_size;
    block->buddy = buddy;

    // Thêm buddy vào danh sách free blocks mới
    buddy->next = system->free_lists[new_order];
    system->free_lists[new_order] = buddy;

    fprintf(out, "[split_block] Đã tách block tại %p thành hai buddies kích thước %zu\n", 
            block->start_addr, new_size);
    
    return block;
}

// Hợp nhất các buddies
void merge_buddies(BuddySystem *system, BuddyBlock* block, int order, FILE *out) {
    if (block == NULL || order >= system->max_order) {
        return;
    }

    BuddyBlock* buddy = block->buddy;
    if (buddy == NULL || !buddy->is_free) {
        return;
    }

    // Xóa cả hai blocks khỏi danh sách free hiện tại
    BuddyBlock* current = system->free_lists[order];
    BuddyBlock* prev = NULL;
    
    // Xóa block đầu tiên
    while (current != NULL) {
        if (current == block) {
            if (prev == NULL) {
                system->free_lists[order] = current->next;
            } else {
                prev->next = current->next;
            }
            break;
        }
        prev = current;
        current = current->next;
    }

    // Xóa buddy
    current = system->free_lists[order];
    prev = NULL;
    while (current != NULL) {
        if (current == buddy) {
            if (prev == NULL) {
                system->free_lists[order] = current->next;
            } else {
                prev->next = current->next;
            }
            break;
        }
        prev = current;
        current = current->next;
    }

    // Tạo block mới với kích thước gấp đôi
    BuddyBlock* merged_block = (BuddyBlock*)malloc(sizeof(BuddyBlock));
    if (merged_block == NULL) {
        fprintf(out, "[merge_buddies] Lỗi: Không thể cấp phát merged block.\n");
        return;
    }

    // Xác định địa chỉ bắt đầu của block mới (block có địa chỉ nhỏ hơn)
    merged_block->start_addr = (block->start_addr < buddy->start_addr) ? 
                              block->start_addr : buddy->start_addr;
    merged_block->size = block->size * 2;
    merged_block->is_free = 1;
    merged_block->buddy = NULL;
    merged_block->next = NULL;

    // Thêm block mới vào danh sách free blocks có kích thước lớn hơn
    merged_block->next = system->free_lists[order + 1];
    system->free_lists[order + 1] = merged_block;

    // Giải phóng các blocks cũ
    free(block);
    free(buddy);

    fprintf(out, "[merge_buddies] Đã hợp nhất hai buddies thành block mới tại %p kích thước %zu\n",
            merged_block->start_addr, merged_block->size);

    // Thử hợp nhất tiếp nếu có thể
    merge_buddies(system, merged_block, order + 1, out);
}

// Cấp phát bộ nhớ sử dụng buddy system
void *buddy_malloc(BuddySystem *system, size_t size, FILE *out) {
    if (system == NULL || size == 0) {
        fprintf(out, "[buddy_malloc] Lỗi: Tham số không hợp lệ.\n");
        return NULL;
    }

    // Tính order cần thiết cho kích thước yêu cầu
    int required_order = get_order(size);
    if (required_order > system->max_order) {
        fprintf(out, "[buddy_malloc] Lỗi: Kích thước yêu cầu quá lớn.\n");
        return NULL;
    }

    // Tìm block phù hợp
    int current_order = required_order;
    BuddyBlock* block = NULL;

    // Tìm block có kích thước phù hợp nhất
    while (current_order <= system->max_order) {
        if (system->free_lists[current_order] != NULL) {
            block = system->free_lists[current_order];
            break;
        }
        current_order++;
    }

    if (block == NULL) {
        fprintf(out, "[buddy_malloc] Lỗi: Không tìm thấy block phù hợp.\n");
        return NULL;
    }

    // Tách block nếu cần thiết
    while (current_order > required_order) {
        block = split_block(system, block, current_order, out);
        if (block == NULL) {
            fprintf(out, "[buddy_malloc] Lỗi: Không thể tách block.\n");
            return NULL;
        }
        current_order--;
    }

    // Đánh dấu block đã được cấp phát
    block->is_free = 0;
    system->allocated_memory_size += block->size;

    fprintf(out, "[buddy_malloc] Đã cấp phát %zu bytes tại địa chỉ %p\n", 
            block->size, block->start_addr);

    return block->start_addr;
}

// Giải phóng bộ nhớ sử dụng buddy system
void buddy_free(BuddySystem *system, void *ptr, size_t size, FILE *out) {
    if (system == NULL || ptr == NULL || size == 0) {
        fprintf(out, "[buddy_free] Lỗi: Tham số không hợp lệ.\n");
        return;
    }

    // Tìm block trong các danh sách free
    int order = get_order(size);
    BuddyBlock* block = NULL;
    BuddyBlock* current = system->free_lists[order];
    BuddyBlock* prev = NULL;

    while (current != NULL) {
        if (current->start_addr == ptr) {
            block = current;
            break;
        }
        prev = current;
        current = current->next;
    }

    if (block == NULL) {
        fprintf(out, "[buddy_free] Lỗi: Không tìm thấy block tại địa chỉ %p\n", ptr);
        return;
    }

    // Đánh dấu block là free
    block->is_free = 1;
    system->allocated_memory_size -= block->size;

    fprintf(out, "[buddy_free] Đã giải phóng block tại %p kích thước %zu\n", 
            block->start_addr, block->size);

    // Thử hợp nhất với buddy
    merge_buddies(system, block, order, out);
}

// In ra danh sách các free blocks trong buddy system
void print_buddy_free_lists(BuddySystem *system, FILE *out) {
    if (system == NULL || system->free_lists == NULL) {
        fprintf(out, "[print_buddy_free_lists] Lỗi: Buddy system chưa được khởi tạo.\n");
        return;
    }

    fprintf(out, "\n================= Danh sách free blocks trong buddy system =================");
    fprintf(out, "\nTổng kích thước bộ nhớ: %zu bytes, Đã cấp phát: %zu bytes\n", 
            system->total_memory_size, system->allocated_memory_size);

    for (int i = 0; i <= system->max_order; i++) {
        size_t block_size = 1 << i; // 2^i
        fprintf(out, "\nOrder %d (Size: %zu bytes):\n", i, block_size);
        
        BuddyBlock* current = system->free_lists[i];
        if (current == NULL) {
            fprintf(out, "  Không có block nào\n");
            continue;
        }

        int count = 0;
        while (current != NULL) {
            fprintf(out, "  Block %d: Start Addr: %p, Size: %zu bytes\n", 
                    count++, current->start_addr, current->size);
            current = current->next;
        }
    }
    fprintf(out, "=======================================================================\n\n");
}