/*
 * DOCUMENT CLUSTERING WORKFLOW - HƯỚNG DẪN CÁCH TRIỂN KHAI
 * 
 * Quy trình tổng quát:
 * 1. Đọc dữ liệu (Document Reader)
 * 2. Tiền xử lý văn bản (Document Processor)
 * 3. Tạo Shingles (Shingle Generator)
 * 4. Tạo MinHash Signature (MinHash)
 * 5. Xây dựng LSH Table (LSH Table)
 * 6. Phát hiện ứng viên tương tự (LSH)
 * 7. Tính toán độ tương đồng chính xác (Similarity Calculator)
 * 8. Thực hiện phân cụm (Clustering Engine)
 * 9. Lưu và hiển thị kết quả (Utils)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "all_headers.h"

#define MAX_STOPWORDS 2048
#define MAX_STOPWORD_LEN 64

// Đọc stopwords từ file vào mảng động
int load_stopwords(const char* filepath, char*** stopwords_out) {
    FILE* f = fopen(filepath, "r");
    if (!f) return 0;
    char** stopwords = (char**)malloc(MAX_STOPWORDS * sizeof(char*));
    int count = 0;
    char buf[MAX_STOPWORD_LEN];
    while (fgets(buf, sizeof(buf), f)) {
        // Loại bỏ ký tự xuống dòng
        buf[strcspn(buf, "\r\n")] = 0;
        // Bỏ qua dòng trống hoặc comment
        if (buf[0] == 0 || buf[0] == '/' || buf[0] == '#') continue;
        stopwords[count] = strdup(buf);
        count++;
        if (count >= MAX_STOPWORDS) break;
    }
    fclose(f);
    *stopwords_out = stopwords;
    return count;
}

// Giải phóng mảng stopwords
void free_stopwords(char** stopwords, int count) {
    for (int i = 0; i < count; ++i) free(stopwords[i]);
    free(stopwords);
}


int main() {
    // Đọc stopwords
    char** stopwords = NULL;
    int stopword_count = load_stopwords("../stopwords/vietnamese-stopwords.txt", &stopwords);
    if (stopword_count == 0) {
        printf("Không thể mở file stopwords/vietnamese-stopwords.txt\n");
    } else {
        printf("Đã load %d stopwords.\n", stopword_count);
    }

    // Đọc toàn bộ folder (ví dụ: data/kenh14_vn)
    const char* folder_path = "../data/kenh14_vn";
    DocumentCollection* collection = read_folder(folder_path);
    if (!collection) {
        printf("Không đọc được folder: %s\n", folder_path);
        free_stopwords(stopwords, stopword_count);
        return 1;
    }

    int num_documents = collection->count;
    int num_bands = DEFAULT_NUM_HASHES / DEFAULT_BAND_SIZE;
    double threshold = DEFAULT_SIMILARITY_THRESHOLD;

    Document* documents = (Document*)malloc(num_documents * sizeof(Document));
    uint32_t* hash_seeds = generate_hash_seeds(DEFAULT_NUM_HASHES);

    // 2. Tiền xử lý, tạo shingles, signature, document object
    for (int i = 0; i < num_documents; i++) {
        char* clean_text = preprocess_document_pipeline(collection->files[i].content, stopwords, stopword_count);
        ShingleSet* shingles = create_character_shingles(clean_text, DEFAULT_SHINGLE_SIZE);
        MinHashSignature* signature = create_minhash_signature(shingles, hash_seeds, DEFAULT_NUM_HASHES);
        
        // Gọi hàm đã tạo
        documents[i] = create_document_object(collection->files[i].doc_id, // Sử dụng doc_id từ DocumentFile
                                              collection->files[i].content,
                                              shingles, signature);
        
        // clean_text được sử dụng để tạo shingles, sau đó được giải phóng
        // vì struct Document hiện tại không lưu trữ preprocessed_content
        if (clean_text) {
            free(clean_text);
        }
    }

    // 5. Xây dựng bảng LSH Table
    LSHTable* lsh_table = create_lsh_table(1000, DEFAULT_BAND_SIZE, num_bands);

    // 6. Thêm các document vào LSH Table
    for (int i = 0; i < num_documents; i++) {
        add_document_to_lsh(lsh_table, &documents[i]);
    }

    // 7. Phát hiện ứng viên tương tự (nếu cần, ví dụ: lấy các cặp ứng viên)
    // CandidatePairs* candidates = get_candidate_pairs(lsh_table);

    // 8. Phân cụm tài liệu dựa trên LSH và ngưỡng độ tương đồng
    int num_clusters = 0;
    Cluster* clusters = cluster_documents_lsh(documents, num_documents, threshold, &num_clusters);

    // 9. Hiển thị kết quả phân cụm
    print_clusters(clusters, num_clusters, documents);

    // Giải phóng bộ nhớ
    free_clusters(clusters, num_clusters);
    free_lsh_table(lsh_table);
    free(hash_seeds);
    // Giải phóng từng đối tượng Document bên trong mảng trước
    for (int i = 0; i < num_documents; i++) {
        free_document_object(&documents[i]); // Truyền địa chỉ của Document struct
    }
    free(documents); // Sau đó giải phóng chính mảng documents
    free(documents);
    free_document_collection(collection);
    free_stopwords(stopwords, stopword_count);

    return 0;
}