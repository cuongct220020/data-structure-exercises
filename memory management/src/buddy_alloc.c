#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "buddy_alloc.h"

#define MIN_BLOCK_SIZE 1  // KB

size_t next_power_of_two(size_t n) {
    size_t power = 1;
    while (power < n)
        power <<= 1;
    return power;
}

// Tạo và khởi tạo buddy system
BuddySystem* create_buddy_system(void *base_addr, size_t total_size, FILE *out) {
    if (base_addr == NULL || total_size <= MIN_BLOCK_SIZE) {
        fprintf(out, "[create_buddy_system] Lỗi: Tham số khởi tạo hệ thống Buddy không hợp lệ.\n");
        return NULL;
    }

    BuddySystem* system = (BuddySystem*)malloc(sizeof(BuddySystem));
    if (system == NULL) {
        fprintf(out, "[create_buddy_system] Lỗi: Không thể cấp phát cho Buddy System.\n");
        return NULL;
    }

    // Khởi tạo các trường thông tin của Budđy System
    system->root = NULL;
    system->max_level = (int)log2(total_size / MIN_BLOCK_SIZE);
    system->base_memory_address = base_addr;
    system->total_memory_size = total_size;
    system->last_memory_address = (char*)base_addr + total_size;
    system->allocated_memory_size = 0;
    
    // Tạo block ban đầu với kích thước lớn nhất
    BuddyBlock* root_block = (BuddyBlock*)malloc(sizeof(BuddyBlock));
    if (root_block == NULL) {
        fprintf(out, "[create_buddy_system] Lỗi: Không thể cấp phát initial_block.\n");
        free(system);
        return NULL;
    }

    // Khởi tạo các trường thông tin của vùng trống ban đầu
    root_block->size = total_size;
    root_block->start_addr = base_addr;
    root_block->is_free = 1;
    root_block->level = 0;
    root_block->parent = NULL;
    root_block->leftChild = NULL;
    root_block->rightChild = NULL;

    // Gán root trỏ vào vùng trống lớn nhất vừa khởi tạo
    system->root = root_block;

    fprintf(out, "[create_buddy_system] Đã khởi tạo buddy system");
    return system;
}

// Hàm chia thành hai buddy
BuddyBlock* split_block(BuddyBlock* block, FILE *out) {
    if (block == NULL) {
        fprintf(out, "[split_block] Lỗi: Block không hợp lệ.\n");
        return NULL;
    }

    size_t half_size = block->size / 2;
    // Tạo hai buddy mới
    BuddyBlock* left_child = (BuddyBlock*)malloc(sizeof(BuddyBlock));
    BuddyBlock* right_child = (BuddyBlock*)malloc(sizeof(BuddyBlock));
    if (left_child == NULL || right_child == NULL) {
        fprintf(out, "[split_block] Lỗi: Không thể cấp phát cho các buddy con.\n");
        free(left_child);
        free(right_child);
    }

    // Cập nhật thông tin cho buddy con bên trái
    left_child->start_addr = block->start_addr;
    left_child->size = half_size;
    left_child->is_free = 1;
    left_child->level = block->level + 1;
    left_child->parent = block;
    left_child->leftChild = NULL;
    left_child->rightChild = NULL;

    // Cập nhật thông tin cho buddy con bên phải
    right_child->start_addr = (char*)block->start_addr + half_size;
    right_child->size = half_size;
    right_child->is_free = 1;
    right_child->level = block->level + 1;
    right_child->parent = block;
    right_child->leftChild = NULL;
    right_child->rightChild = NULL;

    // Cập nhật thông tin cho block hiện tại
    block->leftChild = left_child; // Gán buddy con bên trái
    block->rightChild = right_child; // Gán buddy con bên phải

    return block; // Trả về block đã chia
}

// BuddyBlock* find_and_allocate(BuddyBlock *root, size_t request_size, FILE* out) {
//     // // Nếu block đã được cấp phát thì bỏ qua
//     // if (block->is_free == 0) return NULL;

//     // // Nếu block có kích thước nhỏ hơn yêu cầu thì bỏ qua
//     // if (block->size < request_size) return NULL;

//     // Nếu block có kích thước vừa vặn, tự do và là nút lá trong cây hiện tại
//     if (root->size == request_size && root->is_free == 1 && root->leftChild == NULL && root->rightChild == NULL) {
//         return block;
//     }

//     // Nếu các điều kiện trên không thoả mãn thì chia khối hiện tại
//     if (root->leftChild == NULL && root->rightChild == NULL) {
//         root = split_block(block, out);
//     }

//     // Sau khi chia khối, đệ quy tìm vùng trống tự do ở nút
//     BuddyBlock *free_block = NULL;
    
//     // Tìm vùng trống trên cây con trái trước
//     if (block->leftChild != NULL) {
//         free_block = find_and_allocate(block->leftChild, request_size, out);
//     }

//     // Nếu tìm thấy trả về vùng trống thoả mãn
//     if (free_block != NULL) return free_block;

//     // Nếu không tìm thấy thì tiếp tục tìm trên cây con phải
//     return find_and_allocate(block->rightChild, request_size, out);
// }

BuddyBlock* find_and_allocate(BuddyBlock *root, size_t request_size, FILE* out) {
    if (root == NULL) return NULL;

    // Nếu block đã được cấp phát hoặc nhỏ hơn yêu cầu => bỏ qua
    if (!root->is_free || root->size < request_size) return NULL;

    // Nếu là nút lá, chưa bị chia, và kích thước vừa vặn => dùng luôn
    if (root->size == request_size && root->is_free && root->leftChild == NULL && root->rightChild == NULL) {
        return root;
    }

    // Nếu là nút lá, nhưng kích thước lớn hơn => cần chia nhỏ
    if (root->leftChild == NULL && root->rightChild == NULL) {
        root = split_block(root, out);
        if (root == NULL) return NULL; // Tránh null khi chia lỗi
    }

    // Ưu tiên tìm trong cây con trái
    BuddyBlock *free_block = find_and_allocate(root->leftChild, request_size, out);
    if (free_block != NULL) return free_block;

    // Nếu không có bên trái, thử bên phải
    return find_and_allocate(root->rightChild, request_size, out);
}

// Cấp phát bộ nhớ sử dụng buddy system
BuddyBlock* buddy_malloc(BuddySystem *system, size_t request_size, FILE *out) {
    if (system == NULL || system->root == NULL) {
        fprintf(out, "[buddy_malloc] Lỗi: Hệ thống Buddy không tồn tại.\n");
        return NULL;
    }

    if (request_size < MIN_BLOCK_SIZE || request_size >= system->total_memory_size) {
        fprintf(out, "[buddy_malloc] Lỗi: Yêu cầu cấp phát vùng trống không hợp lệ.\n");
        return NULL;
    }

    // Làm tròn lên size theo luỹ thừa 2 gần nhất
    size_t actual_size = next_power_of_two(request_size);
    
    // Tìm vùng trống tự do phù hợp và cấp phát
    BuddyBlock *allocated_block = find_and_allocate(system->root, actual_size, out);
    if (allocated_block == NULL) {
        fprintf(out, "[buddy_malloc] Không tìm thấy block phù hợp.\n");
        return NULL;
    }

    
    allocated_block->is_free = 0;
    allocated_block->actual_allocated_size = request_size;
    system->allocated_memory_size += request_size;
    
    return allocated_block;
}

// Khi một khối nhớ giải phóng, sẽ được đánh dấu is_free = 1, nếu anh em của nó mà cũng is_free thì sẽ merge hai vùng trống lại với nhau (nghĩa là free toàn bộ hai con), tiếp tục đệ quy và so sánh nút hiện tại với anh em của nó nếu cả hai đều tự do thì merge lại. Chỉ dừng lại khi một nút có is_free = 1 và nút còn lại có is_free = 0. 

// Hợp nhất các buddies
BuddyBlock* merge_buddies(BuddyBlock* request_block, FILE *out) {
    if (request_block == NULL || request_block->parent == NULL) {
        fprintf(out, "[merge_buddies] Khối yêu cầu hiện tại không tồn tại hoặc khối hiện tại không có cha.\n");
        return request_block; // Trả về khối buddy hiện tại
    }

    BuddyBlock* parent = request_block->parent;
    BuddyBlock* buddy = (request_block == parent->leftChild) ? parent->rightChild : parent->leftChild;

    // Nếu buddy không tồn tại hoặc đang được cấp phát thì không thể gộp
    if (buddy == NULL || !buddy->is_free) return request_block;

    // Giải phóng bộ nhớ buddy và chính block hiện tại
    free(parent->leftChild);
    free(parent->rightChild);
    parent->leftChild = NULL;
    parent->rightChild = NULL;
    parent->is_free = 1;

    fprintf(out, "[merge_buddies] Gộp thành công block size %zu tại level %d\n",
            parent->size, parent->level);

    // Đệ quy lên parent
    return merge_buddies(parent, out);
}


void free_buddy(BuddySystem *system, BuddyBlock* request_block, FILE *out) {
    if (system == NULL || system->root == NULL || request_block == NULL) {
        fprintf(out, "[buddy_free] Lỗi: Hệ thống Buddy hoặc khối nhớ không tồn tại.\n");
        return;
    }

    request_block->is_free = 1;
    system->allocated_memory_size -= request_block->actual_allocated_size;
    fprintf(out, "[buddy_free] Đã đánh dấu block size %zu tại địa chỉ %p là tự do.\n",
            request_block->size, request_block->start_addr);

    // Gộp nếu có thể
    merge_buddies(request_block, out);
}


void free_buddy_tree(BuddyBlock* node) {
    if (node == NULL) return;

    // Giải phóng đệ quy cây con trái và phải
    free_buddy_tree(node->leftChild);
    free_buddy_tree(node->rightChild);

    // Giải phóng chính node hiện tại
    free(node);
}

void cleanup_buddy_system(BuddySystem *system, FILE *out) {
    if (system == NULL) {
        fprintf(out, "[cleanup_buddy_system] Con trỏ system trỏ tới NULL.\n");
        return;
    }

    // Giải phóng toàn bộ cây buddy
    free_buddy_tree(system->root);

    // Giải phóng system
    free(system);

    fprintf(out, "[cleanup_buddy_system] Đã giải phóng toàn bộ bộ nhớ của Buddy System.\n");
}


void print_free_blocks(BuddyBlock* node, FILE* out) {
    if (node == NULL) return;

    // Duyệt cây trái trước
    print_free_blocks(node->leftChild, out);

    // In block nếu nó đang free và là nút lá
    if (node->is_free && node->leftChild == NULL && node->rightChild == NULL) {
        fprintf(out, "Free block | Start: %p | Size: %zu KB | Level: %d\n",
                node->start_addr, node->size, node->level);
    }

    // Duyệt cây phải sau
    print_free_blocks(node->rightChild, out);
}

void print_buddy_system(BuddySystem *system, FILE *out) {
    if (system == NULL || system->root == NULL) {
        fprintf(out, "[print_buddy_system] Lỗi: Buddy system chưa được khởi tạo.\n");
        return;
    }

    fprintf(out, "\n================== Buddy System Free Blocks ==================\n");
    fprintf(out, "Total memory: %zu KB | Allocated: %zu KB | Max level: %d\n\n",
            system->total_memory_size, system->allocated_memory_size, system->max_level);

    print_free_blocks(system->root, out);

    fprintf(out, "===============================================================\n");
}