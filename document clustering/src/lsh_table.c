#include "lsh_table.h"
#include <stdlib.h> // Cho malloc, free, realloc, calloc
#include <string.h> // Cho memcpy (không dùng trực tiếp trong code này nhưng hữu ích)
#include <stdio.h>  // Cho printf (debug/thông báo lỗi)
// Các thư viện C++ như <cstdint> <iostream> không còn được dùng

// Bao gồm các header C cần thiết
#include "data_structures.h" // Chứa định nghĩa các struct như LSHTable, Document, Signature
#include "hash_functions.h"  // Chứa định nghĩa các hàm hash như murmur_hash

// Hàm hỗ trợ: Hash một band vào bucket
uint32_t hash_band(uint32_t* band_values, int band_size) {
    // Sử dụng MurmurHash2 để hash toàn bộ band (mảng các uint32_t)
    // Coi band_values như một mảng byte để đưa vào murmur_hash
    // Seed cho murmur_hash có thể là một giá trị cố định hoặc được tính toán từ các band_values
    // Ở đây dùng một seed cố định đơn giản.
    return murmur_hash((const char*)band_values, band_size * sizeof(uint32_t), 42); // 42 là seed cố định
}

// Hàm hỗ trợ: Thêm document ID vào bucket
int add_to_bucket(LSHBucket* bucket, int doc_id) {
    if (!bucket) return 0;

    // Kiểm tra xem doc_id đã tồn tại chưa trong bucket này
    for (int i = 0; i < bucket->count; i++) {
        if (bucket->doc_ids[i] == doc_id) {
            return 1; // Đã tồn tại, không cần thêm lại
        }
    }

    // Mở rộng capacity nếu cần
    if (bucket->count >= bucket->capacity) {
        int new_capacity = bucket->capacity == 0 ? 4 : bucket->capacity * 2; // Bắt đầu với 4, sau đó gấp đôi
        int* new_doc_ids = (int*)realloc(bucket->doc_ids, new_capacity * sizeof(int));
        if (!new_doc_ids) {
            fprintf(stderr, "Lỗi: Không đủ bộ nhớ để mở rộng bucket.\n");
            return 0; // Thất bại
        }
        bucket->doc_ids = new_doc_ids;
        bucket->capacity = new_capacity;
    }

    // Thêm doc_id mới
    bucket->doc_ids[bucket->count++] = doc_id;
    return 1; // Thành công
}

// Tạo bảng LSH
LSHTable* create_lsh_table(int num_buckets, int band_size, int num_bands) {
    if (num_buckets <= 0 || band_size <= 0 || num_bands <= 0) return NULL;

    LSHTable* table = (LSHTable*)malloc(sizeof(LSHTable));
    if (!table) return NULL;

    table->num_buckets = num_buckets;
    table->band_size = band_size;
    table->num_bands = num_bands;

    // Khởi tạo các bucket (sử dụng calloc để đảm bảo các giá trị được khởi tạo về 0/NULL)
    table->buckets = (LSHBucket*)calloc(num_buckets, sizeof(LSHBucket));
    if (!table->buckets) {
        free(table);
        return NULL;
    }

    // (Vòng lặp khởi tạo từng bucket đã được calloc xử lý các trường về 0/NULL)
    // for (int i = 0; i < num_buckets; i++) {
    //     table->buckets[i].doc_ids = NULL;
    //     table->buckets[i].count = 0;
    //     table->buckets[i].capacity = 0;
    // }
    
    return table;
}

// Thêm văn bản vào LSH table
void add_document_to_lsh(LSHTable* table, Document* doc) {
    if (!table || !doc || !doc->signature) return;
    
    // Kiểm tra xem signature có đủ số lượng hash để tạo các band không
    if (doc->signature->num_hashes < table->band_size * table->num_bands) {
        fprintf(stderr, "Cảnh báo: Signature của tài liệu %d quá ngắn để tạo đủ các band. Bỏ qua.\n", doc->id);
        return;
    }

    // Chia signature thành các band và hash từng band
    for (int band = 0; band < table->num_bands; band++) {
        int start_idx = band * table->band_size;
        
        // Lấy con trỏ đến band hiện tại
        uint32_t* current_band_values = &(doc->signature->values[start_idx]);
        
        // Hash band để tìm bucket
        uint32_t hash = hash_band(current_band_values, table->band_size);
        int bucket_idx = hash % table->num_buckets; // Lấy chỉ số bucket
        
        // Đảm bảo chỉ số bucket hợp lệ (trường hợp hash âm, mặc dù uint32_t không âm)
        if (bucket_idx < 0) bucket_idx = -bucket_idx; 
        
        // Thêm document vào bucket tương ứng
        if (!add_to_bucket(&(table->buckets[bucket_idx]), doc->id)) {
            fprintf(stderr, "Lỗi: Không thể thêm tài liệu %d vào bucket %d.\n", doc->id, bucket_idx);
        }
    }
}

// Tìm các văn bản ứng viên tương tự
int* find_similar_candidates(LSHTable* table, Document* query_doc, int* num_candidates) {
    if (!table || !query_doc || !query_doc->signature || !num_candidates) {
        if (num_candidates) *num_candidates = 0;
        return NULL;
    }
    
    // Kiểm tra xem signature của query_doc có đủ độ dài không
    if (query_doc->signature->num_hashes < table->band_size * table->num_bands) {
        fprintf(stderr, "Cảnh báo: Signature của tài liệu truy vấn quá ngắn. Không thể tìm ứng viên.\n");
        *num_candidates = 0;
        return NULL;
    }

    // Sử dụng một mảng để đánh dấu các document đã gặp
    // Kích thước mảng dựa trên MAX_DOCUMENTS để tránh tràn bộ nhớ
    int* seen = (int*)calloc(MAX_DOCUMENTS, sizeof(int));
    // Cấp phát ban đầu cho candidates, sẽ realloc sau
    int* candidates = (int*)malloc(MAX_DOCUMENTS * sizeof(int)); 
    int count = 0;
    
    if (!seen || !candidates) {
        fprintf(stderr, "Lỗi: Không đủ bộ nhớ cho mảng 'seen' hoặc 'candidates'.\n");
        free(seen);
        free(candidates);
        *num_candidates = 0;
        return NULL;
    }
    
    // Duyệt qua từng band của query document
    for (int band = 0; band < table->num_bands; band++) {
        int start_idx = band * table->band_size;
        
        // Lấy con trỏ đến band hiện tại của query document
        uint32_t* current_band_values = &(query_doc->signature->values[start_idx]);
        
        // Hash band để tìm bucket
        uint32_t hash = hash_band(current_band_values, table->band_size);
        int bucket_idx = hash % table->num_buckets;
        if (bucket_idx < 0) bucket_idx = -bucket_idx;

        // Lấy tất cả documents trong bucket này
        LSHBucket* bucket = &(table->buckets[bucket_idx]);
        for (int i = 0; i < bucket->count; i++) {
            int doc_id = bucket->doc_ids[i];
            
            // Bỏ qua chính query document
            if (doc_id == query_doc->id) continue;
            
            // Kiểm tra xem đã thấy document này chưa và ID có nằm trong giới hạn MAX_DOCUMENTS không
            if (doc_id >= 0 && doc_id < MAX_DOCUMENTS && !seen[doc_id]) {
                seen[doc_id] = 1; // Đánh dấu đã thấy
                if (count < MAX_DOCUMENTS) { // Đảm bảo không vượt quá kích thước mảng candidates ban đầu
                    candidates[count++] = doc_id;
                } else {
                    // Nếu số lượng ứng viên vượt quá MAX_DOCUMENTS, cần mở rộng mảng candidates
                    // hoặc chấp nhận một giới hạn cứng. Ở đây, tôi sẽ in cảnh báo và bỏ qua phần còn lại.
                    fprintf(stderr, "Cảnh báo: Số lượng ứng viên vượt quá MAX_DOCUMENTS (%d). Một số ứng viên có thể bị bỏ qua.\n", MAX_DOCUMENTS);
                    break; // Thoát khỏi vòng lặp inner để tránh tràn mảng candidates
                }
            }
        }
    }
    
    free(seen); // Giải phóng mảng đánh dấu
    
    // Resize mảng candidates theo số lượng thực tế
    if (count == 0) {
        free(candidates);
        *num_candidates = 0;
        return NULL;
    }
    
    int* result = (int*)realloc(candidates, count * sizeof(int));
    if (!result) {
        fprintf(stderr, "Lỗi: Không thể thay đổi kích thước mảng ứng viên cuối cùng.\n");
        free(candidates); // Vẫn giải phóng candidates gốc
        *num_candidates = 0;
        return NULL;
    }
    
    *num_candidates = count;
    return result;
}

// Giải phóng bộ nhớ của một LSHBucket
void free_bucket(LSHBucket* bucket) {
    if (bucket && bucket->doc_ids) {
        free(bucket->doc_ids);
        bucket->doc_ids = NULL;
        bucket->count = 0;
        bucket->capacity = 0;
    }
}

// Giải phóng toàn bộ bộ nhớ của LSHTable
void free_lsh_table(LSHTable* table) {
    if (!table) return;
    
    if (table->buckets) {
        // Giải phóng từng bucket trong mảng
        for (int i = 0; i < table->num_buckets; i++) {
            free_bucket(&(table->buckets[i]));
        }
        free(table->buckets); // Giải phóng mảng các bucket
    }
    
    free(table); // Giải phóng cấu trúc bảng LSH
}