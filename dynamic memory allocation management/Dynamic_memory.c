#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Định nghĩa cấu trúc cho một khối bộ nhớ (block)
typedef struct MemoryBlock {
    void* start_addr;               // Địa chỉ bắt đầu của khối bộ nhớ
    size_t size;                    // Kích thước của khối bộ nhớ (đơn vị byte)
    struct MemoryBlock* next;       // Con trỏ tới khối bộ nhớ tiếp theo trong danh sách liên kết
    int is_free;                    // Đánh dấu block còn trống (1) hay đã cấp phát (0)
} MemoryBlock;

// Danh sách liên kết các khối bộ nhớ còn trống (free)
MemoryBlock* free_list = NULL;      // Danh sách các block còn trống
MemoryBlock* last_allocated = NULL; // Block cuối cùng được cấp phát (dùng cho next fit)

// Hàm tìm lũy thừa của 2 lớn nhất >= size
size_t next_power_of_two(size_t size) {
    size_t n = 1;
    while (n < size) n <<= 1;
    return n;
}

// Khởi tạo vùng nhớ với một block duy nhất
void initialize_memory(void* base_addr, size_t total_size){
    free_list = (MemoryBlock*)malloc(sizeof(MemoryBlock));
    free_list->start_addr = base_addr;
    free_list->size = total_size;
    free_list->next = NULL;
    free_list->is_free = 1;
}

// In ra danh sách các block bộ nhớ còn trống
void print_free_list(){
    MemoryBlock* current = free_list;
    printf("Free Memory Blocks:\n");
    while (current != NULL){
        if (current->is_free)
            printf("  Address: %p, Size: %zu\n", current->start_addr, current->size);
        current = current->next;
    }
}

// Thuật toán cấp phát First Fit
void* firstfit_malloc(size_t size) {
    MemoryBlock* current = free_list;
    MemoryBlock* prev = NULL;

    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            void* allocated_addr = current->start_addr;

            if (current->size == size) {
                current->is_free = 0;
                // Không xóa khỏi danh sách, chỉ đánh dấu đã cấp phát
            } else {
                // Tạo block mới cho phần đã cấp phát
                MemoryBlock* new_block = (MemoryBlock*)malloc(sizeof(MemoryBlock));
                new_block->start_addr = current->start_addr;
                new_block->size = size;
                new_block->is_free = 0;
                new_block->next = current;
                if (prev == NULL) {
                    free_list = new_block;
                } else {
                    prev->next = new_block;
                }
                current->start_addr = (char*)current->start_addr + size;
                current->size -= size;
                return new_block->start_addr;
            }
            return allocated_addr;
        }
        prev = current;
        current = current->next;
    }
    return NULL;
}

// Thuật toán cấp phát Best Fit
void* bestfit_malloc(size_t size) {
    MemoryBlock* best = NULL;
    MemoryBlock* best_prev = NULL;
    MemoryBlock* current = free_list;
    MemoryBlock* prev = NULL;

    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            if (best == NULL || current->size < best->size) {
                best = current;
                best_prev = prev;
            }
        }
        prev = current;
        current = current->next;
    }

    if (best == NULL) return NULL;

    void* allocated_addr = best->start_addr;

    if (best->size == size) {
        best->is_free = 0;
        // Không xóa khỏi danh sách, chỉ đánh dấu đã cấp phát
    } else {
        // Tạo block mới cho phần đã cấp phát
        MemoryBlock* new_block = (MemoryBlock*)malloc(sizeof(MemoryBlock));
        new_block->start_addr = best->start_addr;
        new_block->size = size;
        new_block->is_free = 0;
        new_block->next = best;
        if (best_prev == NULL) {
            free_list = new_block;
        } else {
            best_prev->next = new_block;
        }
        best->start_addr = (char*)best->start_addr + size;
        best->size -= size;
        return new_block->start_addr;
    }
    return allocated_addr;
}

// Thuật toán cấp phát Worst Fit
void* worstfit_malloc(size_t size) {
    MemoryBlock* current = free_list;
    MemoryBlock* prev = NULL;
    MemoryBlock* worst_block = NULL;
    MemoryBlock* worst_prev = NULL;

    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            if (worst_block == NULL || current->size > worst_block->size) {
                worst_block = current;
                worst_prev = prev;
            }
        }
        prev = current;
        current = current->next;
    }

    if (worst_block == NULL) return NULL;

    void* allocated_addr = worst_block->start_addr;

    if (worst_block->size == size) {
        worst_block->is_free = 0;
        // Không xóa khỏi danh sách, chỉ đánh dấu đã cấp phát
    } else {
        // Tạo block mới cho phần đã cấp phát
        MemoryBlock* new_block = (MemoryBlock*)malloc(sizeof(MemoryBlock));
        new_block->start_addr = worst_block->start_addr;
        new_block->size = size;
        new_block->is_free = 0;
        new_block->next = worst_block;
        if (worst_prev == NULL) {
            free_list = new_block;
        } else {
            worst_prev->next = new_block;
        }
        worst_block->start_addr = (char*)worst_block->start_addr + size;
        worst_block->size -= size;
        return new_block->start_addr;
    }
    return allocated_addr;
}

// Thuật toán cấp phát Next Fit
void* nextfit_malloc(size_t size) {
    if (free_list == NULL) return NULL;

    MemoryBlock* current;
    MemoryBlock* prev = NULL;
    MemoryBlock* start_point = NULL;

    if (last_allocated == NULL || last_allocated->next == NULL) {
        current = free_list;
    } else {
        current = last_allocated->next;
    }

    start_point = current;
    int wrapped = 0;

    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            void* allocated_addr = current->start_addr;

            if (current->size == size) {
                current->is_free = 0;
                last_allocated = current;
            } else {
                // Tạo block mới cho phần đã cấp phát
                MemoryBlock* new_block = (MemoryBlock*)malloc(sizeof(MemoryBlock));
                new_block->start_addr = current->start_addr;
                new_block->size = size;
                new_block->is_free = 0;
                new_block->next = current;
                if (prev == NULL) {
                    free_list = new_block;
                } else {
                    prev->next = new_block;
                }
                current->start_addr = (char*)current->start_addr + size;
                current->size -= size;
                last_allocated = new_block;
                return new_block->start_addr;
            }
            return allocated_addr;
        }

        prev = current;
        current = current->next;

        if (current == NULL && wrapped == 0) {
            current = free_list;
            prev = NULL;
            wrapped = 1;
            if (current == start_point) break;
        }

        if (current == start_point) break;
    }

    return NULL;
}

// Thuật toán Buddy System
void* buddysystem_malloc(size_t size) {
    size_t req_size = next_power_of_two(size);
    MemoryBlock* prev = NULL;
    MemoryBlock* current = free_list;

    while (current) {
        if (current->is_free && current->size >= req_size) {
            // Chia nhỏ block cho đến khi vừa đủ
            while (current->size > req_size) {
                size_t half = current->size / 2;
                MemoryBlock* buddy = (MemoryBlock*)malloc(sizeof(MemoryBlock));
                buddy->start_addr = (char*)current->start_addr + half;
                buddy->size = half;
                buddy->is_free = 1;
                buddy->next = current->next;

                current->size = half;
                current->next = buddy;
            }
            current->is_free = 0;
            return current->start_addr;
        }
        prev = current;
        current = current->next;
    }
    return NULL;
}

// Giải phóng một block bộ nhớ, hợp nhất nếu cần
void free_mem(void* addr, size_t size) {
    MemoryBlock* prev = NULL;
    MemoryBlock* current = free_list;

    // Tìm block đã cấp phát tương ứng với addr
    while (current != NULL) {
        if (!current->is_free && current->start_addr == addr && current->size == size) {
            current->is_free = 1;
            break;
        }
        prev = current;
        current = current->next;
    }
    // Nếu không tìm thấy block đã cấp phát, tạo block mới (trường hợp đặc biệt)
    if (current == NULL) {
        MemoryBlock* new_block = (MemoryBlock*)malloc(sizeof(MemoryBlock));
        new_block->start_addr = addr;
        new_block->size = size;
        new_block->is_free = 1;
        new_block->next = NULL;

        // Chèn vào đầu danh sách
        new_block->next = free_list;
        free_list = new_block;
        current = new_block;
    }

    // Hợp nhất với block sau nếu liền kề và cùng free
    while (current->next && current->is_free && current->next->is_free &&
           (char*)current->start_addr + current->size == (char*)current->next->start_addr) {
        MemoryBlock* next_block = current->next;
        current->size += next_block->size;
        current->next = next_block->next;
        free(next_block);
    }

    // Hợp nhất với block trước nếu liền kề và cùng free
    prev = NULL;
    MemoryBlock* iter = free_list;
    while (iter && iter != current) {
        if (iter->is_free &&
            (char*)iter->start_addr + iter->size == (char*)current->start_addr) {
            iter->size += current->size;
            iter->next = current->next;
            free(current);
            current = iter;
            break;
        }
        prev = iter;
        iter = iter->next;
    }
}

// Hàm main để kiểm thử các thuật toán cấp phát và giải phóng bộ nhớ
int main() {
    char memory_pool[2048];
    initialize_memory(memory_pool, 2048);
    print_free_list();

    void* ptr1 = firstfit_malloc(256);
    printf("\nAfter allocating 256 bytes (First Fit):\n");
    print_free_list();

    void* ptr2 = bestfit_malloc(128);
    printf("\nAfter allocating 128 bytes (Best Fit):\n");
    print_free_list();

    void* ptr3 = worstfit_malloc(512);
    printf("\nAfter allocating 512 bytes (Worst Fit):\n");
    print_free_list();

    void* ptr4 = nextfit_malloc(128);
    printf("\nAfter allocating 128 bytes (Next Fit):\n");
    print_free_list();

    void* ptr5 = buddysystem_malloc(300);
    printf("\nAfter allocating 300 bytes (Buddy System):\n");
    print_free_list();

    free_mem(ptr1, 256);
    printf("\nAfter freeing 256 bytes:\n");
    print_free_list();

    free_mem(ptr2, 128);
    printf("\nAfter freeing 128 bytes:\n");
    print_free_list();

    free_mem(ptr3, 512);
    printf("\nAfter freeing 512 bytes:\n");
    print_free_list();

    free_mem(ptr4, 128);
    printf("\nAfter freeing 128 bytes:\n");
    print_free_list();

    free_mem(ptr5, next_power_of_two(300));
    printf("\nAfter freeing 300 bytes (buddy block):\n");
    print_free_list();

    return 0;
}