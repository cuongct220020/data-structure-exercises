#ifndef HASH_FUNCTIONS_H
#define HASH_FUNCTIONS_H

#include <stdint.h> // Cho kiểu dữ liệu uint32_t
#include <stdbool.h> // Cho kiểu dữ liệu bool (C99 trở lên)

// ========== HÀM HASH ==========

// Hàm hash đơn giản (ví dụ: djb2) cho chuỗi.
// Tham số str: Chuỗi cần băm.
// Trả về: Giá trị hash 32-bit.
uint32_t simple_hash(const char* str);

// Hàm hash với seed (ví dụ: djb2 biến đổi) để tạo nhiều hàm hash khác nhau.
// Tham số str: Chuỗi cần băm.
// Tham số seed: Giá trị seed ban đầu cho hàm hash.
// Trả về: Giá trị hash 32-bit.
uint32_t hash_with_seed(const char* str, uint32_t seed);

// Tạo một mảng các seed ngẫu nhiên hoặc giả ngẫu nhiên để sử dụng cho các hàm hash.
// Các seed này cần được dùng để khởi tạo nhiều phiên bản khác nhau của hàm hash.
// Tham số num_hashes: Số lượng seed cần tạo.
// Trả về: Con trỏ tới một mảng các giá trị uint32_t (cần được giải phóng bằng free() sau khi sử dụng), hoặc NULL nếu lỗi cấp phát bộ nhớ.
uint32_t* generate_hash_seeds(int num_hashes);

// Hàm hash MurmurHash2 (phiên bản 32-bit). Đây là một hàm hash phi mã hóa chất lượng cao.
// Tham số key: Con trỏ tới dữ liệu cần băm (chuỗi byte).
// Tham số len: Độ dài của dữ liệu.
// Tham số seed: Giá trị seed ban đầu cho hàm hash.
// Trả về: Giá trị hash 32-bit.
uint32_t murmur_hash(const char* key, int len, uint32_t seed);

#endif // HASH_FUNCTIONS_H