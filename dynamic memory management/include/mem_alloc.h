#ifndef MEM_BLOCK_H
#define MEM_BLOCK_H

#include <stdio.h>

// Định nghĩa cấu trúc cho một khối bộ nhớ (block)
typedef struct MemoryBlock {
    void* start_addr;
    size_t size;
    struct MemoryBlock* next;
} MemoryBlock;

typedef struct MemoryManagement {
    MemoryBlock *free_list; // Danh sách quản lý các vùng trống tự do trong vùng trống ban đầu
    void *base_memory_address; // Con trỏ giữ địa chỉ bắt đầu của vùng trống tự do mà ta quản lý
    void *last_memory_address; // Con trỏ giữ địa chỉ kết thúc của vùng trống tự do mà ta quản lý
    size_t total_memory_size; // Tổng vùng trống tự do ban đầu
    size_t allocated_memory_size; // Tổng vùng trống đã cấp phát
    MemoryBlock *next_fit_last_block; // Con trỏ lưu lại vị trí tìm kiếm khối trước đó, phục vụ cho chiến lược next fit.
} MemoryManagement;

// Hàm khởi tạo một vùng nhớ trống ban đầu
void initialize_memory_manager(MemoryManagement *manager, void *base_addr, size_t total_size, FILE *out);

// Giải phóng cấu trúc MemoryManagement
void cleanup_memory_manager(MemoryManagement *manager, FILE *out);

// Hàm in ra danh sách các khối đang trống
void print_free_list(MemoryManagement *manager, FILE *out);

// Hàm hợp nhất các khối trống liền kề
void merge_free_blocks(MemoryManagement *manager, FILE *out);

// Thêm vùng trống vào danh sách vùng trống tự do
void add_free_mem_block(MemoryManagement *manager, void *start_addr, size_t size, FILE *out);

// Hàm cấp phát tại địa chỉ cụ thể
void *allocate_at_address(MemoryManagement *manager, void *start_addr_request, size_t size, FILE *out);

// Các chiến lược cấp phát bộ nhớ
void *firstfit_malloc(MemoryManagement *manager, size_t size, FILE *out); // First Fit
void *bestfit_malloc(MemoryManagement *manager, size_t size, FILE *out); // Best Fit
void *worstfit_malloc(MemoryManagement *manager, size_t size, FILE *out); // Worst Fit
void *nextfit_malloc(MemoryManagement *manager, size_t size, FILE *out); // Next Fit

// Hàm giải phóng
void free_mem(MemoryManagement *manager, void *ptr, size_t size, FILE *out);

#endif


