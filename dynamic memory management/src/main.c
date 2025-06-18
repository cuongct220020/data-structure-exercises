#include <stdio.h>
#include <stdlib.h>
#include <stddef.h> 
#include "mem_alloc.h"
#include "buddy_alloc.h"

int main() {
    FILE *logfile = fopen("../output/output.txt", "w");
    if (!logfile) {
        printf("[main] Lỗi: Không thể mở file log.\n");
        return 1;
    }
    // Cấp phát một instance của MemoryManagement
    MemoryManagement *global_mem_manager = (MemoryManagement*)malloc(sizeof(MemoryManagement));
    if (global_mem_manager == NULL) {
        fprintf(logfile, "[main] Lỗi: Không thể cấp phát cấu trúc MemoryManagement.\n");
        fclose(logfile);
        return 1;
    }
    // Khởi tạo các trường của global_mem_manager về NULL/0 trước khi dùng
    global_mem_manager->free_list = NULL;
    global_mem_manager->base_memory_address = NULL;
    global_mem_manager->total_memory_size = 0;
    global_mem_manager->allocated_memory_size = 0;
    global_mem_manager->next_fit_last_block = NULL;

    // Giảm kích thước vùng nhớ ban đầu
    size_t pool_size = 1024; // 1KB
    void *memory_pool = malloc(pool_size);
    if (memory_pool == NULL) {
        fprintf(logfile, "[main] Lỗi: Không thể cấp phát vùng nhớ ban đầu.\n");
        free(global_mem_manager);
        fclose(logfile);
        return 1;
    }

    initialize_memory_manager(global_mem_manager, memory_pool, pool_size, logfile);
    print_free_list(global_mem_manager, logfile);

    fprintf(logfile, "\n=== Bắt đầu tạo phân mảnh ban đầu bằng allocate_at_address ===\n");
    void *base_addr = global_mem_manager->base_memory_address;

    void *fixed_ptr_A = allocate_at_address(global_mem_manager, base_addr, 50, logfile);
    void *fixed_ptr_B = allocate_at_address(global_mem_manager, (char*)base_addr + 100, 100, logfile);
    void *fixed_ptr_C = allocate_at_address(global_mem_manager, (char*)base_addr + 300, 100, logfile);
    void *fixed_ptr_D = allocate_at_address(global_mem_manager, (char*)base_addr + 500, 100, logfile);
    void *fixed_ptr_E = allocate_at_address(global_mem_manager, (char*)base_addr + 700, 100, logfile);
    print_free_list(global_mem_manager, logfile);

    fprintf(logfile, "\n=== Giải phóng một số khối để tạo thêm phân mảnh và gộp ===\n");
    free_mem(global_mem_manager, fixed_ptr_A, 50, logfile);
    free_mem(global_mem_manager, fixed_ptr_C, 100, logfile);
    print_free_list(global_mem_manager, logfile);

    fprintf(logfile, "\n=== Test First Fit ===\n");
    void *ff_alloc1 = firstfit_malloc(global_mem_manager, 80, logfile);
    print_free_list(global_mem_manager, logfile);

    void *ff_alloc2 = firstfit_malloc(global_mem_manager, 150, logfile);
    print_free_list(global_mem_manager, logfile);

    fprintf(logfile, "\n--- Giải phóng các khối FF để reset trạng thái cho test tiếp theo ---\n");
    free_mem(global_mem_manager, ff_alloc1, 80, logfile);
    free_mem(global_mem_manager, ff_alloc2, 150, logfile);
    print_free_list(global_mem_manager, logfile);

    fprintf(logfile, "\n=== Test Best Fit ===\n");
    void *bf_alloc1 = bestfit_malloc(global_mem_manager, 90, logfile);
    print_free_list(global_mem_manager, logfile);

    void *bf_alloc2 = bestfit_malloc(global_mem_manager, 180, logfile);
    print_free_list(global_mem_manager, logfile);

    fprintf(logfile, "\n--- Giải phóng các khối BF để reset trạng thái cho test tiếp theo ---\n");
    free_mem(global_mem_manager, bf_alloc1, 90, logfile);
    free_mem(global_mem_manager, bf_alloc2, 180, logfile);
    print_free_list(global_mem_manager, logfile);

    fprintf(logfile, "\n=== Test Worst Fit ===\n");
    void *wf_alloc1 = worstfit_malloc(global_mem_manager, 50, logfile);
    print_free_list(global_mem_manager, logfile);

    void *wf_alloc2 = worstfit_malloc(global_mem_manager, 80, logfile);
    print_free_list(global_mem_manager, logfile);

    fprintf(logfile, "\n--- Giải phóng các khối WF để reset trạng thái cho test tiếp theo ---\n");
    free_mem(global_mem_manager, wf_alloc1, 50, logfile);
    free_mem(global_mem_manager, wf_alloc2, 80, logfile);
    print_free_list(global_mem_manager, logfile);
    global_mem_manager->next_fit_last_block = NULL;

    fprintf(logfile, "\n=== Test Next Fit ===\n");
    void *nf_alloc1 = nextfit_malloc(global_mem_manager, 70, logfile);
    print_free_list(global_mem_manager, logfile);

    void *nf_alloc2 = nextfit_malloc(global_mem_manager, 40, logfile);
    print_free_list(global_mem_manager, logfile);

    void *nf_alloc3 = nextfit_malloc(global_mem_manager, 150, logfile);
    print_free_list(global_mem_manager, logfile);

    void *nf_alloc4 = nextfit_malloc(global_mem_manager, 100, logfile);
    print_free_list(global_mem_manager, logfile);

    fprintf(logfile, "\n--- Giải phóng tất cả các khối còn lại ---\n");
    free_mem(global_mem_manager, fixed_ptr_B, 100, logfile);
    free_mem(global_mem_manager, fixed_ptr_D, 100, logfile);
    free_mem(global_mem_manager, fixed_ptr_E, 100, logfile);

    free_mem(global_mem_manager, nf_alloc1, 70, logfile);
    free_mem(global_mem_manager, nf_alloc2, 40, logfile);
    free_mem(global_mem_manager, nf_alloc3, 150, logfile);
    free_mem(global_mem_manager, nf_alloc4, 100, logfile);

    print_free_list(global_mem_manager, logfile);

    // Cleanup cho MemoryManagement
    cleanup_memory_manager(global_mem_manager, logfile);

    // Test Buddy System
    fprintf(logfile, "\n=== Test Buddy System ===\n");
    
    // Tạo vùng nhớ mới cho buddy system
    void *buddy_memory_pool = malloc(pool_size);
    if (buddy_memory_pool == NULL) {
        fprintf(logfile, "[main] Lỗi: Không thể cấp phát vùng nhớ cho buddy system.\n");
        fclose(logfile);
        return 1;
    }

    // Khởi tạo buddy system
    BuddySystem* buddy_system = create_buddy_system(buddy_memory_pool, pool_size, logfile);
    if (buddy_system == NULL) {
        fprintf(logfile, "[main] Lỗi: Không thể khởi tạo buddy system.\n");
        free(buddy_memory_pool);
        fclose(logfile);
        return 1;
    }

    print_buddy_free_lists(buddy_system, logfile);

    // Test cấp phát các khối có kích thước khác nhau
    fprintf(logfile, "\n--- Test cấp phát các khối có kích thước khác nhau ---\n");
    void *buddy_alloc1 = buddy_malloc(buddy_system, 64, logfile);  // 2^6
    void *buddy_alloc2 = buddy_malloc(buddy_system, 128, logfile); // 2^7
    void *buddy_alloc3 = buddy_malloc(buddy_system, 256, logfile); // 2^8
    print_buddy_free_lists(buddy_system, logfile);

    // Test giải phóng và hợp nhất
    fprintf(logfile, "\n--- Test giải phóng và hợp nhất ---\n");
    buddy_free(buddy_system, buddy_alloc1, 64, logfile);
    buddy_free(buddy_system, buddy_alloc2, 128, logfile);
    print_buddy_free_lists(buddy_system, logfile);

    // Test cấp phát lại sau khi giải phóng
    fprintf(logfile, "\n--- Test cấp phát lại sau khi giải phóng ---\n");
    void *buddy_alloc4 = buddy_malloc(buddy_system, 192, logfile); // 2^7 + 2^6
    print_buddy_free_lists(buddy_system, logfile);

    // Giải phóng tất cả
    fprintf(logfile, "\n--- Giải phóng tất cả các khối buddy ---\n");
    buddy_free(buddy_system, buddy_alloc3, 256, logfile);
    buddy_free(buddy_system, buddy_alloc4, 192, logfile);
    print_buddy_free_lists(buddy_system, logfile);

    // Cleanup cho buddy system
    cleanup_buddy_system(buddy_system, logfile);
    free(buddy_memory_pool);

    fclose(logfile);
    return 0;
}