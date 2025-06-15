#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Định nghĩa cấu trúc cho một khối bộ nhớ (block)
typedef struct MemoryBlock {
    void* start_addr;
    size_t size;
    struct MemoryBlock* next;
} MemoryBlock;

// Hàm khởi tạo một vùng nhớ trống ban đầu
void initialize_memory(void* base_addr, size_t total_size);

// Hàm in ra danh sách các khối đang trống
void print_free_list();

// Chiến lược cấp phát bộ nhớ First Fit
void *firstfit_malloc(size_t size);

// Chiến lược cấp phát bộ nhớ Worst Fit
void *worstfit_malloc(size_t size);

// Chiến lược cấp phát bộ nhớ Next Fit
void *nextfit_malloc(size_t size);

// Chiến lược cấp phát Buddy Allocation
void *buddysystem_malloc(size_t size);

// Hàm giải phóng bộ nhớ và giải phóng khối liền kề (nếu có)
void free_mem(void* addr, size_t size);