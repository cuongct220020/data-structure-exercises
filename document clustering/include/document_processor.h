#ifndef DOCUMENT_PROCESSOR_H
#define DOCUMENT_PROCESSOR_H

#include <stdio.h>   // Cho NULL, size_t
#include <stdlib.h>  // Cho malloc, free, realloc
#include <string.h>  // Cho strlen, strcpy, strcmp, strncpy, memmove, strcat
#include <ctype.h>   // Cho tolower, isalnum, isspace, isdigit
#include <stdbool.h> // Cho kiểu dữ liệu bool (C99 trở lên)

// Bao gồm các cấu trúc dữ liệu C của bạn
// (Đảm bảo data_structures.h không chứa các include C++ và dùng char* thuần túy)
#include "data_structures.h"

// Chuyển văn bản thành chữ thường.
// Tham số str: Chuỗi cần chuyển đổi (được sửa đổi tại chỗ).
void to_lowercase(char* str);

// Loại bỏ các ký tự không phải chữ cái hoặc số và thay thế bằng khoảng trắng.
// Tham số str: Chuỗi cần làm sạch (được sửa đổi tại chỗ).
void remove_non_alphanumeric(char* str);

// Loại bỏ khoảng trắng thừa (đầu, cuối và nhiều khoảng trắng liên tiếp).
// Tham số str: Chuỗi cần làm sạch (được sửa đổi tại chỗ).
void trim_and_reduce_whitespace(char* str);

// Tách từ (tokenization) và loại bỏ stop words, số nguyên và từ có 1 ký tự.
// Tham số text: Chuỗi văn bản đầu vào (không bị sửa đổi).
// Tham số stopwords: Mảng các chuỗi stopword.
// Tham số num_stopwords: Số lượng stopword trong mảng.
// Trả về: Một chuỗi mới đã được lọc và chuẩn hóa (cần được free bởi người gọi).
char* tokenize_and_filter(const char* text, const char** stopwords, int num_stopwords);

// Quy trình tiền xử lý tài liệu hoàn chỉnh.
// Thực hiện chuyển chữ thường, loại bỏ ký tự đặc biệt, chuẩn hóa khoảng trắng,
// và lọc stop words, số nguyên, từ 1 ký tự.
// Tham số content: Nội dung gốc của tài liệu (không bị sửa đổi).
// Tham số stopwords: Mảng các chuỗi stopword.
// Tham số num_stopwords: Số lượng stopword trong mảng.
// Trả về: Một chuỗi mới là nội dung đã được tiền xử lý (cần được free bởi người gọi).
char* preprocess_document_pipeline(const char* content, const char** stopwords, int num_stopwords);

#endif // DOCUMENT_PROCESSOR_H