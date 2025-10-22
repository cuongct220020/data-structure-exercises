#ifndef LSH_TABLE_H
#define LSH_TABLE_H

#include <stdint.h> // Cho kiểu dữ liệu uint32_t
#include <stdbool.h> // Cho kiểu dữ liệu bool (C99 trở lên)

// Bao gồm các cấu trúc dữ liệu C của bạn (đảm bảo data_structures.h dùng char* thuần túy)
#include "data_structures.h" 

// ========== HÀM LSH ==========

// Tạo bảng LSH mới.
// Tham số num_buckets: Tổng số bucket trong bảng LSH.
// Tham số band_size: Kích thước của mỗi band (số lượng giá trị hash trong một band).
// Tham số num_bands: Số lượng band.
// Trả về: Con trỏ tới LSHTable đã cấp phát, hoặc NULL nếu lỗi.
LSHTable* create_lsh_table(int num_buckets, int band_size, int num_bands);

// Hash một band (một mảng các giá trị uint32_t) vào một giá trị hash 32-bit.
// Tham số band_values: Mảng các giá trị uint32_t của band.
// Tham số band_size: Số lượng phần tử trong band_values.
// Trả về: Giá trị hash 32-bit của band.
uint32_t hash_band(uint32_t* band_values, int band_size);

// Hàm hỗ trợ: Thêm một ID tài liệu vào một bucket cụ thể.
// Tham số bucket: Con trỏ tới LSHBucket cần thêm.
// Tham số doc_id: ID của tài liệu cần thêm.
// Trả về: 1 nếu thành công, 0 nếu thất bại (ví dụ: không đủ bộ nhớ).
int add_to_bucket(LSHBucket* bucket, int doc_id);

// Thêm một tài liệu vào bảng LSH.
// Tài liệu sẽ được chia thành các band, mỗi band được băm vào một bucket tương ứng.
// Tham số table: Con trỏ tới LSHTable.
// Tham số doc: Con trỏ tới Document cần thêm (chứa MinHash signature).
void add_document_to_lsh(LSHTable* table, Document* doc);

// Tìm các ID văn bản ứng viên tương tự với một văn bản truy vấn.
// Các ứng viên là những tài liệu nằm trong cùng một bucket với văn bản truy vấn ở ít nhất một band.
// Tham số table: Con trỏ tới LSHTable.
// Tham số query_doc: Con trỏ tới Document truy vấn (chứa MinHash signature).
// Tham số num_candidates: Con trỏ tới biến sẽ lưu số lượng ứng viên tìm được.
// Trả về: Mảng các ID tài liệu ứng viên (cần được giải phóng bằng free() bởi người gọi, hoặc NULL nếu không tìm thấy ứng viên nào hoặc lỗi.
int* find_similar_candidates(LSHTable* table, Document* query_doc, int* num_candidates);

// Giải phóng bộ nhớ của một LSHBucket.
// Tham số bucket: Con trỏ tới LSHBucket cần giải phóng.
void free_bucket(LSHBucket* bucket);

// Giải phóng toàn bộ bộ nhớ của LSHTable.
// Tham số table: Con trỏ tới LSHTable cần giải phóng.
void free_lsh_table(LSHTable* table);


// int are_documents_in_same_bucket(LSHTable* table, int doc_id1, int doc_id2);
// void print_lsh_table_stats(LSHTable* table);

#endif // LSH_TABLE_H