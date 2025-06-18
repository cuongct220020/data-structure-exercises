#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include "mem_alloc.h"

// Hàm khởi tạo một vùng nhớ trống ban đầu
void initialize_memory_manager(MemoryManagement *manager, void *base_addr, size_t total_size, FILE *out) {
    if (manager == NULL) {
        fprintf(out, "[initialize_memory_manager] Lỗi: Con trỏ manager là NULL.\n");
        return;
    }

    // Đảm bảo rằng free_list trống trước khi khởi tạo
    if (manager->free_list != NULL) {
        MemoryBlock *current = manager->free_list;
        while (current != NULL) {
            MemoryBlock* temp = current;
            current = current->next;
            free(temp); // Giải phóng các node MemoryBlock
        }
        manager->free_list = NULL;
    }

    manager->base_memory_address = base_addr;
    manager->total_memory_size = total_size;
    manager->last_memory_address = (char*) base_addr + total_size;
    manager->allocated_memory_size = 0; // Khởi tạo bộ nhớ đã cấp phát là 0
    manager->next_fit_last_block = NULL; // Khởi tạo cho next fit

    MemoryBlock* initial_block = (MemoryBlock*)malloc(sizeof(MemoryBlock));
    if (initial_block == NULL) {
        fprintf(out, "[initialize_memory_manager] Lỗi: cấp phát bộ nhớ động cho initial_block.\n");
        return;
    }
    initial_block->start_addr = base_addr;
    initial_block->size = total_size;
    initial_block->next = NULL;
    manager->free_list = initial_block; // Gán cho free_list của manager

    fprintf(out, "[initialize_memory_manager] Vùng trống ban đầu (initial memory pool) có địa chỉ cơ sở: %p, kích thước: %zu.\n", base_addr, total_size);
}

// Giải phóng cấu trúc MemoryManagement
void cleanup_memory_manager(MemoryManagement *manager, FILE *out) {
    if (manager == NULL) {
        fprintf(out, "[cleanup_memory_manager] Lỗi: Con trỏ manager là NULL.\n");
        return;
    }

    MemoryBlock *current = manager->free_list;
    while (current != NULL) {
        MemoryBlock *temp = current;
        current = current->next;
        free(temp); // Giải phóng các node MemoryBlock
    }
    manager->free_list = NULL;

    // Giải phóng vùng bộ nhớ lớn ban đầu nếu nó được cấp phát bởi malloc
    if (manager->base_memory_address != NULL) {
        free(manager->base_memory_address);
        manager->base_memory_address = NULL;
        manager->last_memory_address = NULL;
        manager->total_memory_size = 0;
        manager->allocated_memory_size = 0;
        fprintf(out, "[clean_memory_manager] Đã giải phóng vùng bộ nhớ ban đầu được quản lý.\n");
    }
    // Giải phóng chính cấu trúc MemoryManagement nếu nó được cấp phát động
    free(manager);
}

void print_free_list(MemoryManagement *manager, FILE *out) {
    if (manager == NULL) {
        fprintf(out, "[print_free_list] Lỗi: Con trỏ manager là NULL.\n");
        return;
    }
    fprintf(out, "\n================= Danh sách vùng trống dự do =================");
    fprintf(out, "\nKích thường vùng bộ nhớ quản lý: %zu bytes, Đã cấp phát: %zu bytes\n", manager->total_memory_size, manager->allocated_memory_size);
    
    if (manager->free_list == NULL) {
        fprintf(out, "[print_free_list] Free list is empty.\n");
        return;
    }
    MemoryBlock *current = manager->free_list;
    int i = 0;
    while (current != NULL) {
        fprintf(out, "Block %d: Start Addr: %p, Size: %zu bytes\n", i++, current->start_addr, current->size);
        current = current->next;
    }
    fprintf(out, "==============================================================\n\n");
}

// Hàm hợp nhất các khối trống liền kề
void merge_free_blocks(MemoryManagement *manager, FILE *out) {
    if (manager == NULL || manager->free_list == NULL) {
        // Không có gì để gộp hoặc manager là NULL
        return;
    }

    MemoryBlock *current = manager->free_list;
    int merged_happened; // Biến cờ để kiểm tra xem có sự gộp nào xảy ra trong một lần duyệt không

    do {
        merged_happened = 0; // Đặt lại cờ cho mỗi lần duyệt
        current = manager->free_list; // Bắt đầu lại từ đầu danh sách
        MemoryBlock *prev = NULL;

        while (current != NULL && current->next != NULL) {
            // Kiểm tra xem vùng trống hiện tại và vùng trống kế tiếp có liền kề nhau không
            if ((char*)current->start_addr + current->size == (char*)current->next->start_addr) {
                // Hợp nhất hai khối
                current->size += current->next->size; // Tăng kích thước của khối hiện tại
                MemoryBlock *temp = current->next; // Giữ con trỏ đến khối kế tiếp để giải phóng
                current->next = temp->next; // Bỏ khối kế tiếp ra khỏi danh sách
                free(temp); // Giải phóng node MemoryBlock của khối đã gộp
                merged_happened = 1; // Đánh dấu rằng đã có sự gộp
                fprintf(out, "[merge_free_blocks] Đã gộp khối tại %p (kích thước %zu) với khối tại %p.\n", current->start_addr, current->size, temp->start_addr);
            } else {
                // Nếu không liền kề, di chuyển tới khối tiếp theo
                prev = current;
                current = current->next;
            }
        }
    } while (merged_happened); // Lặp lại cho đến khi không có sự gộp nào xảy ra trong một lần duyệt
}

// Thêm vùng trống tự do vào danh sách vùng trống, hợp nhất các vùng trống nếu có thể. 
void add_free_mem_block(MemoryManagement *manager, void *start_addr, size_t size, FILE *out) {
    if (manager == NULL) {
        fprintf(out, "[add_free_mem_block] Lỗi: Con trỏ manager là NULL.\n");
        return;
    }

    MemoryBlock *new_block = (MemoryBlock *)malloc(sizeof(MemoryBlock));
    if (new_block == NULL) {
        fprintf(out, "[add_free_mem_block] Lỗi cấp phát bộ nhớ động cho new_block.\n");
        return;
    }
    new_block->start_addr = start_addr;
    new_block->size = size;
    new_block->next = NULL;

    // Nếu danh sách trống hoặc khối mới nên ở đầu
    if (manager->free_list == NULL || new_block->start_addr < manager->free_list->start_addr) {
        new_block->next = manager->free_list;
        manager->free_list = new_block;
    } else {
        // Duyệt qua danh sách để tìm vị trí thích hợp
        MemoryBlock* current = manager->free_list;
        while (current->next != NULL && current->next->start_addr < new_block->start_addr) {
            current = current->next;
        }
        // Chèn new_block vào vị trí
        new_block->next = current->next;
        current->next = new_block;
    }
    
    fprintf(out, "[add_free_mem_block] Đã thêm block mới tại địa chỉ %p, kích thước %zu. Bắt đầu gộp...\n", start_addr, size);
    merge_free_blocks(manager, out); // Gọi hàm gộp sau khi thêm block mới
}

// Chiến lược cấp phát bộ nhớ First Fit
void *firstfit_malloc(MemoryManagement *manager, size_t size, FILE *out) {
    if (manager == NULL) {
        fprintf(out, "[firstfit_malloc] Lỗi: Con trỏ manager là NULL.\n");
        return NULL;
    }
    if (size == 0) {
        fprintf(out, "[firstfit_malloc] Kích thước khối nhớ xin cấp phát không hợp lệ.\n");
        return NULL;
    }

    MemoryBlock *current = manager->free_list;
    MemoryBlock *prev = NULL;
    void *allocated_addr = NULL;

    while (current != NULL) {
        if (current->size >= size) { // Tìm thấy vùng trống fit
            allocated_addr = current->start_addr; // Lấy địa chỉ để cấp phát

            // Trường hợp 1: Vùng trống vừa vặn (exact fit)
            if (current->size == size) {
                if (prev == NULL) { // Là node đầu tiên
                    manager->free_list = current->next;
                } else {
                    prev->next = current->next;
                }
                free(current); // Giải phóng MemoryBlock node
                manager->allocated_memory_size += size; // Cập nhật bộ nhớ đã cấp phát
                fprintf(out, "[firstfit_malloc] Cấp phát %zu bytes tại địa chỉ %p (exact fit).\n", size, allocated_addr);
                return allocated_addr; // Trả về địa chỉ và kết thúc
            }
            // Trường hợp 2: Vùng trống lớn hơn (split)
            else { // current->size > size
                // Cập nhật lại thông tin của block hiện tại
                current->start_addr = (char*)current->start_addr + size;
                current->size -= size;
                manager->allocated_memory_size += size; // Cập nhật bộ nhớ đã cấp phát
                fprintf(out, "[firstfit_malloc] Cấp phát %zu bytes tại địa chỉ %p (split, còn lại %zu bytes tại %p).\n", size, allocated_addr, current->size, current->start_addr);
                return allocated_addr; // Trả về địa chỉ và kết thúc
            }
        }
        // Nếu không fit, chuyển sang khối tiếp theo
        prev = current;
        current = current->next;
    }

    // Duyệt hết danh sách mà không tìm thấy
    fprintf(out, "Không tồn tại vùng trống tự do đáp ứng yêu cầu (%zu bytes).\n", size);
    return NULL;
}


// Cấp phát tại địa chỉ cụ thể
void *allocate_at_address(MemoryManagement *manager, void *start_addr_request, size_t size, FILE *out) {
    if (manager == NULL) {
        fprintf(out, "[allocate_at_address] Lỗi: Con trỏ manager là NULL.\n");
        return NULL;
    }
    if (size == 0) {
        fprintf(out, "[allocate_at_address] Kích thước khối nhớ xin cấp phát không hợp lệ.\n");
        return NULL;
    }
    if (start_addr_request == NULL) {
        fprintf(out, "[allocate_at_address] Địa chỉ yêu cầu cấp phát là NULL.\n");
        return NULL;
    }

    // Kiểm tra xem địa chỉ và kích thước yêu cầu có nằm hoàn toàn trong vùng bộ nhớ quản lý không
    if (start_addr_request < manager->base_memory_address || (char*)start_addr_request + size > (char*)manager->last_memory_address) {
        fprintf(out, "[allocate_at_address] Yêu cầu cấp phát (%p, %zu bytes) nằm ngoài vùng bộ nhớ được quản lý (%p, %zu bytes).\n",
               start_addr_request, size, manager->base_memory_address, manager->total_memory_size);
        return NULL;
    }

    fprintf(out, "[allocate_at_address] Thực hiện cấp phát bộ nhớ tại địa chỉ %p, kích thước %zu.\n", start_addr_request, size);

    MemoryBlock *current = manager->free_list;
    MemoryBlock *prev = NULL;

    while (current != NULL) {
        // Kiểm tra xem khối trống hiện tại có chứa hoàn toàn vùng nhớ yêu cầu không
        if (start_addr_request >= current->start_addr &&
            (char*)start_addr_request + size <= (char*)current->start_addr + current->size) {

            // Đã tìm thấy một khối trống đủ lớn và chứa vùng yêu cầu
            void *allocated_addr = start_addr_request;

            // Xử lý các trường hợp cắt khối:

            // 1. Vùng yêu cầu khớp chính xác với khối trống
            if (current->start_addr == start_addr_request && current->size == size) {
                if (prev == NULL) {
                    manager->free_list = current->next;
                } else {
                    prev->next = current->next;
                }
                free(current); // Giải phóng node MemoryBlock
            }
            // 2. Vùng yêu cầu nằm ở đầu khối trống (cần cắt phần cuối)
            else if (current->start_addr == start_addr_request) {
                current->start_addr = (char*)current->start_addr + size;
                current->size -= size;
            }
            // 3. Vùng yêu cầu nằm ở cuối khối trống (cần cắt phần đầu)
            else if ((char*)current->start_addr + current->size == (char*)start_addr_request + size) {
                current->size -= size;
            }
            // 4. Vùng yêu cầu nằm ở giữa khối trống (cần cắt cả hai đầu) - trường hợp phức tạp hơn
            else {
                // Tạo một khối mới cho phần cuối của khối trống
                MemoryBlock *new_block_after = (MemoryBlock*)malloc(sizeof(MemoryBlock));
                if (new_block_after == NULL) {
                    fprintf(out, "[allocate_at_address] Lỗi: Cấp phát bộ nhớ cho phần còn lại sau khi cắt.\n");
                    return NULL; // Cấp phát thất bại
                }
                new_block_after->start_addr = (char*)start_addr_request + size;
                new_block_after->size = (char*)current->start_addr + current->size - ((char*)start_addr_request + size);
                new_block_after->next = current->next; // Chèn vào sau current

                // Cập nhật kích thước của khối trống hiện tại (phần đầu)
                current->size = (char*)start_addr_request - (char*)current->start_addr;
                current->next = new_block_after; // Liên kết khối hiện tại với khối mới
            }

            manager->allocated_memory_size += size;
            fprintf(out, "[allocate_at_address] Đã thực hiện cấp phát %zu bytes tại địa chỉ %p.\n\n", size, allocated_addr);
            return allocated_addr;
        }

        prev = current;
        current = current->next;
    }

    fprintf(out, "[allocate_at_address] Không tìm thấy vùng trống tự do chứa yêu cầu cấp phát tại địa chỉ %p với kích thước %zu bytes.\n", start_addr_request, size);
    return NULL;
}

// Chiến lược cấp phát bộ nhớ Best Fit
void *bestfit_malloc(MemoryManagement *manager, size_t size, FILE *out) {
    if (manager == NULL) {
        fprintf(out, "[bestfit_malloc] Lỗi: Con trỏ manager là NULL.\n");
        return NULL;
    }
    if (size == 0) {
        fprintf(out, "[bestfit_malloc] Kích thước khối nhớ xin cấp phát không hợp lệ.\n");
        return NULL;
    }

    MemoryBlock *current = manager->free_list;
    MemoryBlock *best_fit_block = NULL;
    size_t min_diff = SIZE_MAX; 

    while (current != NULL) {
        if (current->size >= size) {
            size_t diff = current->size - size;
            if (diff < min_diff) {
                min_diff = diff;
                best_fit_block = current;
            }
        }
        current = current->next;
    }

    if (best_fit_block != NULL) {
        fprintf(out, "[bestfit_malloc] Tìm thấy khối phù hợp nhất tại %p (kích thước %zu).\n", best_fit_block->start_addr, best_fit_block->size);
        return allocate_at_address(manager, best_fit_block->start_addr, size, out);
    }

    fprintf(out, "[bestfit_malloc] Không tồn tại vùng trống tự do đáp ứng yêu cầu (%zu bytes).\n", size);
    return NULL;
}

// Chiến lược cấp phát bộ nhớ Worst Fit
void *worstfit_malloc(MemoryManagement *manager, size_t size, FILE *out) {
    if (manager == NULL) {
        fprintf(out, "[worstfit_malloc] Lỗi: Con trỏ manager là NULL.\n");
        return NULL;
    }
    if (size == 0) {
        fprintf(out, "[worstfit_malloc] Kích thước khối nhớ xin cấp phát không hợp lệ.\n");
        return NULL;
    }

    MemoryBlock *current = manager->free_list;
    MemoryBlock *worst_fit_block = NULL;
    size_t max_size = 0; 

    while (current != NULL) {
        if (current->size >= size) {
            if (current->size > max_size) {
                max_size = current->size;
                worst_fit_block = current;
            }
        }
        current = current->next;
    }

    if (worst_fit_block != NULL) {
        fprintf(out, "[worstfit_malloc] Tìm thấy khối tệ nhất tại %p (kích thước %zu).\n", worst_fit_block->start_addr, worst_fit_block->size);
        return allocate_at_address(manager, worst_fit_block->start_addr, size, out);
    }

    fprintf(out, "[worstfit_malloc] Không tồn tại vùng trống tự do đáp ứng yêu cầu (%zu bytes).\n", size);
    return NULL;
}

void *nextfit_malloc(MemoryManagement *manager, size_t size, FILE *out) {
    if (manager == NULL) {
        fprintf(out, "[nextfit_malloc] Lỗi: Con trỏ manager là NULL.\n");
        return NULL;
    }
    if (size == 0) {
        fprintf(out, "[nextfit_malloc] Kích thước khối nhớ xin cấp phát không hợp lệ.\n");
        return NULL;
    }

    if (manager->next_fit_last_block == NULL) {
        manager->next_fit_last_block = manager->free_list;
    }

    MemoryBlock *start_search_block = manager->next_fit_last_block;
    MemoryBlock *current = start_search_block;
    void *allocated_addr = NULL;
    int wrapped_around = 0;

    do {
        while (current != NULL) {
            if (current->size >= size) {
                allocated_addr = current->start_addr;
                fprintf(out, "[nextfit_malloc] Tìm thấy khối phù hợp từ điểm cuối tại %p (kích thước %zu).\n", current->start_addr, current->size);
                
                // Cập nhật next_fit_last_block tới vị trí tiếp theo sau khi cấp phát
                if (current->size == size) { // Exact fit
                    manager->next_fit_last_block = current->next;
                } else { // Split
                    manager->next_fit_last_block = current; // Vẫn trỏ vào khối hiện tại đã bị cắt
                }
                return allocate_at_address(manager, allocated_addr, size, out);
            }
            current = current->next;
        }

        if (!wrapped_around) {
            current = manager->free_list; 
            wrapped_around = 1; 
        } else {
            break;
        }
    } while (current != start_search_block); 

    fprintf(out, "[nextfit_malloc] Không tồn tại vùng trống tự do đáp ứng yêu cầu (%zu bytes).\n", size);
    manager->next_fit_last_block = NULL; 
    return NULL;
}

// Hàm giải phóng
void free_mem(MemoryManagement *manager, void *ptr, size_t size, FILE *out) {
    if (manager == NULL) {
        fprintf(out, "[free_mem] Lỗi: Con trỏ manager là NULL.\n");
        return;
    }
    if (ptr == NULL || size == 0) {
        fprintf(out, "[free_mem] Không thể giải phóng địa chỉ NULL hoặc kích thước 0.\n");
        return;
    }

    if (ptr < manager->base_memory_address || (char*)ptr + size > (char*)manager->base_memory_address + manager->total_memory_size) {
        fprintf(out, "[free_mem] Địa chỉ %p nằm ngoài vùng bộ nhớ được quản lý.\n", ptr);
        return;
    }
    fprintf(out, "\n[free_mem] Thực hiện giải %zu bytes tại địa chỉ %p.\n", size, ptr);

    manager->allocated_memory_size -= size;
    
    add_free_mem_block(manager, ptr, size, out); 
    
    fprintf(out, "[free_mem] Đã giải phóng %zu bytes tại địa chỉ %p.\n", size, ptr);
}
