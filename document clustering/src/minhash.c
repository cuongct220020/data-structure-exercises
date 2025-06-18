#include "minhash.h"     // Bao gồm header của chính nó
#include "data_structures.h" // Chứa định nghĩa ShingleSet và MinHashSignature
#include <stdlib.h>      // Cho malloc, free
#include <string.h>      // Cho strlen (nếu cần cho hàm hash)
#include <limits.h>      // Cho UINT32_MAX

// Hàm hash đơn giản (static để giới hạn phạm vi trong file này)
// Sử dụng seed để tạo ra các biến thể khác nhau của hàm hash.
static uint32_t simple_hash(const char* str, uint32_t seed) {
    uint32_t hash = seed;
    // Thuật toán hash polynomial rolling hash
    for (int i = 0; str[i] != '\0'; i++) { // Duyệt qua từng ký tự trong chuỗi
        hash = hash * 31 + (unsigned char)str[i]; // Phép nhân và cộng
    }
    return hash;
}

// Tạo MinHash signature từ ShingleSet
MinHashSignature* create_minhash_signature(ShingleSet* shingles, uint32_t* hash_seeds, int num_hashes) {
    // Kiểm tra đầu vào hợp lệ
    if (!shingles || !hash_seeds || num_hashes <= 0 || shingles->count == 0) {
        return NULL;
    }

    MinHashSignature* sig = (MinHashSignature*)malloc(sizeof(MinHashSignature));
    if (!sig) return NULL; // Lỗi cấp phát bộ nhớ cho MinHashSignature struct

    sig->values = (uint32_t*)malloc(num_hashes * sizeof(uint32_t));
    if (!sig->values) {
        free(sig); // Giải phóng MinHashSignature struct nếu không cấp phát được mảng values
        return NULL; // Lỗi cấp phát bộ nhớ cho mảng values
    }

    sig->num_hashes = num_hashes;

    // Khởi tạo tất cả các giá trị hash trong signature với giá trị lớn nhất (UINT32_MAX)
    // Điều này đảm bảo rằng bất kỳ giá trị hash thực nào từ shingle đều nhỏ hơn
    for (int i = 0; i < num_hashes; i++) {
        sig->values[i] = UINT32_MAX;
    }

    // Duyệt qua từng shingle trong tập shingles
    for (int s_idx = 0; s_idx < shingles->count; s_idx++) {
        const char* current_shingle = shingles->items[s_idx]; // Truy cập shingle từ mảng shingles

        // Nếu shingle rỗng, bỏ qua để tránh lỗi hoặc hash không mong muốn
        if (!current_shingle || strlen(current_shingle) == 0) {
            continue;
        }

        // Tính giá trị hash cho shingle hiện tại với từng hàm hash (dựa trên hash_seeds)
        for (int h_idx = 0; h_idx < num_hashes; h_idx++) {
            uint32_t current_hash_seed = hash_seeds[h_idx];
            uint32_t hash_val = simple_hash(current_shingle, current_hash_seed);

            // Cập nhật giá trị MinHash tại vị trí h_idx nếu hash_val nhỏ hơn
            if (hash_val < sig->values[h_idx]) {
                sig->values[h_idx] = hash_val;
            }
        }
    }

    return sig;
}

// Ước lượng Jaccard similarity từ hai MinHash signatures
// Đổi tên hàm từ jaccard_similarity_from_minhash sang estimate_jaccard_similarity
double estimate_jaccard_similarity(MinHashSignature* sig1, MinHashSignature* sig2) {
    // Kiểm tra con trỏ NULL và đảm bảo hai signature có cùng kích thước
    if (!sig1 || !sig2 || sig1->num_hashes != sig2->num_hashes || sig1->num_hashes == 0) {
        return 0.0;
    }

    int matches = 0;
    // Đếm số lượng vị trí mà hai signature có cùng giá trị hash
    for (int i = 0; i < sig1->num_hashes; i++) {
        if (sig1->values[i] == sig2->values[i]) {
            matches++;
        }
    }

    // Jaccard similarity ước lượng bằng (số lượng matches) / (tổng số hàm hash)
    return (double)matches / sig1->num_hashes;
}

// Giải phóng bộ nhớ MinHash signature
void free_minhash_signature(MinHashSignature* sig) {
    if (!sig) return; // Không làm gì nếu con trỏ NULL

    if (sig->values) {
        free(sig->values); // Giải phóng mảng các giá trị hash
    }

    free(sig); // Giải phóng cấu trúc MinHashSignature
}