#ifndef MINHASH_H
#define MINHASH_H

#include "data_structures.h" // Chứa định nghĩa ShingleSet và MinHashSignature
#include <stdint.h>          // Cho kiểu dữ liệu uint32_t

// ========== HÀM MINHASH ==========

// Tạo MinHash signature cho một tập shingles.
// Sử dụng mảng hash_seeds để tạo num_hashes hàm băm khác nhau.
// Tham số shingles: Con trỏ tới tập shingles đầu vào.
// Tham số hash_seeds: Mảng các giá trị seed (uint32_t) cho các hàm băm. Kích thước mảng này phải ít nhất là num_hashes.
// Tham số num_hashes: Số lượng hàm băm (và cũng là kích thước của signature).
// Trả về: Con trỏ tới MinHashSignature đã được cấp phát, hoặc NULL nếu lỗi.
MinHashSignature* create_minhash_signature(ShingleSet* shingles,
                                          uint32_t* hash_seeds,
                                          int num_hashes);

// Ước lượng Jaccard similarity giữa hai MinHash signature.
// Độ chính xác của ước lượng phụ thuộc vào num_hashes.
// Tham số sig1: Con trỏ tới MinHashSignature thứ nhất.
// Tham số sig2: Con trỏ tới MinHashSignature thứ hai.
// Trả về: Giá trị ước lượng Jaccard similarity (từ 0.0 đến 1.0).
double estimate_jaccard_similarity(MinHashSignature* sig1,
                                  MinHashSignature* sig2);

// Giải phóng bộ nhớ của một MinHash signature.
// Tham số sig: Con trỏ tới MinHashSignature cần giải phóng.
void free_minhash_signature(MinHashSignature* sig);

#endif // MINHASH_H