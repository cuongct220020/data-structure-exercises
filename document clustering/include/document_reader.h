#ifndef DOCUMENT_READER_H
#define DOCUMENT_READER_H

#include <stdio.h>   // Cho FILE, NULL, size_t, printf
#include <stdlib.h>  // Cho malloc, free, realloc
#include <string.h>  // Cho strlen, strcpy, strcmp, strcasecmp (nếu dùng)
#include <dirent.h>  // Cho DIR, dirent, opendir, readdir, closedir
#include <sys/stat.h> // Cho stat, S_ISREG
#include <stdbool.h> // Cho kiểu dữ liệu bool (C99 trở lên)

// Bao gồm các cấu trúc dữ liệu C của bạn (Đảm bảo data_structures.h dùng char* thuần túy)
#include "data_structures.h"

// Đọc nội dung một file text.
// Tham số filepath: Đường dẫn đầy đủ đến file.
// Trả về: Con trỏ tới nội dung file (cần được free bởi người gọi), hoặc NULL nếu lỗi.
char* read_file(const char* filepath);

// Hàm hỗ trợ: Kiểm tra file có phải là .txt không (không phân biệt hoa thường).
// Tham số filename: Tên file.
// Trả về: 1 nếu là .txt, 0 nếu không.
int is_txt_file(const char* filename);

// Tạo DocumentCollection mới với dung lượng ban đầu.
// Tham số folder_path: Đường dẫn của folder mà collection này đại diện.
// Trả về: Con trỏ tới DocumentCollection đã cấp phát, hoặc NULL nếu lỗi.
DocumentCollection* create_document_collection(const char* folder_path);

// Hàm hỗ trợ: Mở rộng dung lượng của DocumentCollection nếu cần.
// Tham số collection: Con trỏ tới DocumentCollection.
void expand_document_collection(DocumentCollection* collection);

// Hàm hỗ trợ: Thêm một file tài liệu vào DocumentCollection.
// Hàm này sẽ đọc nội dung file và cấp phát bộ nhớ cho tên, đường dẫn, nội dung.
// Trả về: true (1) nếu thành công, false (0) nếu thất bại.
bool add_document_to_collection(DocumentCollection* collection, const char* filename, const char* filepath);

// Đọc tất cả file .txt trong một folder.
// Tham số folder_path: Đường dẫn đến folder.
// Trả về: Con trỏ tới DocumentCollection chứa các file đã đọc, hoặc NULL nếu lỗi.
// Người gọi có trách nhiệm giải phóng bộ nhớ bằng free_document_collection.
DocumentCollection* read_folder(const char* folder_path);

// Đọc nhiều folder cùng lúc và hợp nhất các tài liệu vào một DocumentCollection duy nhất.
// Tham số folder_paths: Mảng các chuỗi đường dẫn folder.
// Tham số num_folders: Số lượng folder trong mảng.
// Trả về: Con trỏ tới DocumentCollection chứa tất cả các file đã đọc, hoặc NULL nếu lỗi.
// Người gọi có trách nhiệm giải phóng bộ nhớ bằng free_document_collection.
DocumentCollection* read_multiple_folders(char** folder_paths, int num_folders);

// Giải phóng toàn bộ bộ nhớ được cấp phát cho DocumentCollection.
// Tham số collection: Con trỏ tới DocumentCollection cần giải phóng.
void free_document_collection(DocumentCollection* collection);

#endif // DOCUMENT_READER_H