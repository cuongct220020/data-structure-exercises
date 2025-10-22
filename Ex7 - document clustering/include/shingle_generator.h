#ifndef SHINGLE_GENERATOR_H
#define SHINGLE_GENERATOR_H

#include <stdint.h> // Cho uint32_t nếu cần
#include <stdbool.h> // Cho kiểu dữ liệu bool trong C99 trở lên
#include <stdlib.h> // Cho malloc, free
#include <string.h> // Cho strlen, strcpy, strcmp, strncpy, strcat
#include <stdio.h>  // Cho NULL
#include <ctype.h>  // Cho tolower, isalpha, isalnum

#include "data_structures.h" // Bao gồm các cấu trúc dữ liệu C của bạn

// ========== SHINGLE FUNCTIONS ==========

// Tạo ShingleSet mới với dung lượng ban đầu
// Trả về con trỏ tới ShingleSet đã cấp phát, hoặc NULL nếu lỗi
ShingleSet* create_shingle_set(int initial_capacity);

// Kiểm tra shingle có tồn tại trong tập hợp không
// Trả về true (1) nếu có, false (0) nếu không
bool contains_shingle(const ShingleSet* set, const char* shingle);

// Thêm shingle vào tập hợp. Chỉ thêm nếu shingle chưa tồn tại.
// Trả về 1 nếu thành công, 0 nếu thất bại (ví dụ: không đủ bộ nhớ)
int add_shingle(ShingleSet* set, const char* shingle);

// Tạo character shingles (k-shingles) từ một chuỗi văn bản
// Trả về con trỏ ShingleSet đã cấp phát, hoặc NULL nếu lỗi
// Người gọi có trách nhiệm giải phóng bộ nhớ bằng free_shingle_set
ShingleSet* create_character_shingles(const char* text, int k);

// Tách từ từ text, chuyển về chữ thường và loại bỏ dấu câu cơ bản
// Trả về mảng con trỏ char* (các từ), và lưu số lượng từ vào num_tokens
// Người gọi có trách nhiệm giải phóng bộ nhớ cho từng từ và mảng con trỏ
char** tokenize_text(const char* text, int* num_tokens);

// Tạo word n-grams từ một chuỗi văn bản
// Trả về con trỏ ShingleSet đã cấp phát, hoặc NULL nếu lỗi
// Người gọi có trách nhiệm giải phóng bộ nhớ bằng free_shingle_set
ShingleSet* create_word_ngrams(const char* text, int n);

// Giải phóng toàn bộ bộ nhớ của ShingleSet
void free_shingle_set(ShingleSet* set);

#endif // SHINGLE_GENERATOR_H