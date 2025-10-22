#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <stdint.h> // Thay thế <cstdint> bằng <stdint.h> cho C

// Các tham số có thể điều chỉnh
#define DEFAULT_SHINGLE_SIZE 5  // Kích thước shingle mặc định
#define DEFAULT_NUM_HASHES 128  // Số lượng hàm hash MinHash
#define DEFAULT_BAND_SIZE 8     // Kích thước mỗi band trong LSH
#define DEFAULT_SIMILARITY_THRESHOLD 0.5  // Ngưỡng tương đồng
#define MAX_DOCUMENTS 10000           // Số lượng văn bản tối đa
#define MAX_SHINGLES 100000          // Số lượng shingles tối đa

// Cấu trúc lưu shingle/n-gram
// Quản lý thủ công: items (mảng con trỏ chuỗi), count (số lượng hiện tại), capacity (dung lượng tối đa)
typedef struct {
    char** items;          // Mảng các con trỏ tới chuỗi shingle (malloc/free từng chuỗi và mảng này)
    int count;            // Số lượng shingle hiện có
    int capacity;         // Dung lượng tối đa của mảng items
} ShingleSet;

// Cấu trúc lưu signature MinHash
// Quản lý thủ công: values (mảng các giá trị hash), num_hashes (số lượng giá trị)
typedef struct {
    uint32_t* values;     // Mảng các giá trị hash (malloc/free mảng này)
    int num_hashes;       // Số lượng hàm hash (kích thước của mảng values)
} MinHashSignature;

// Cấu trúc một văn bản
// Quản lý thủ công: content (nội dung gốc), shingles (tập shingles), signature (chữ ký MinHash)
typedef struct {
    int id;               // ID văn bản
    char* content;        // Nội dung gốc (malloc/free chuỗi này)
    ShingleSet* shingles; // Con trỏ tới ShingleSet (malloc/free cấu trúc này)
    MinHashSignature* signature; // Con trỏ tới MinHashSignature (malloc/free cấu trúc này)
} Document;

// Cấu trúc LSH bucket
// Quản lý thủ công: doc_ids (danh sách ID văn bản), count (số lượng ID), capacity (dung lượng tối đa)
typedef struct {
    int* doc_ids;         // Danh sách ID văn bản trong bucket (malloc/free mảng này)
    int count;            // Số lượng văn bản hiện có trong bucket
    int capacity;         // Dung lượng tối đa của mảng doc_ids
} LSHBucket;

// Cấu trúc bảng hash LSH
// Quản lý thủ công: buckets (mảng các LSHBucket)
typedef struct {
    LSHBucket* buckets;   // Mảng các bucket (malloc/free mảng các LSHBucket, mỗi LSHBucket cũng cần quản lý riêng)
    int num_buckets;      // Số lượng bucket trong bảng
    int band_size;        // Kích thước mỗi band (số lượng giá trị hash trong một band)
    int num_bands;        // Số lượng band (số lượng lần chia signature)
} LSHTable;

// Cấu trúc lưu một cụm
// Quản lý thủ công: doc_ids (danh sách ID văn bản), count (số lượng ID)
typedef struct {
    int* doc_ids;         // Danh sách ID văn bản trong cụm (malloc/free mảng này)
    int count;            // Số lượng văn bản hiện có trong cụm
    double avg_similarity; // Độ tương đồng trung bình trong cụm
} Cluster;

// Cấu trúc lưu thông tin một file
// Quản lý thủ công: filename, filepath, content
typedef struct {
    char* filename;       // Tên file (không bao gồm path) (malloc/free chuỗi này)
    char* filepath;       // Đường dẫn đầy đủ (malloc/free chuỗi này)
    char* content;        // Nội dung file (malloc/free chuỗi này)
    long file_size;       // Kích thước file
    int doc_id;           // ID duy nhất cho document
} DocumentFile;

// Cấu trúc lưu danh sách tất cả file trong folder
// Quản lý thủ công: files (mảng các DocumentFile), folder_path
typedef struct {
    DocumentFile* files;  // Mảng các file (malloc/free mảng các DocumentFile, mỗi DocumentFile cũng cần quản lý riêng)
    int count;            // Số lượng file hiện có
    int capacity;         // Dung lượng tối đa của mảng files
    char* folder_path;    // Đường dẫn folder gốc (malloc/free chuỗi này)
} DocumentCollection;

#endif // DATA_STRUCTURES_HPP