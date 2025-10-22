#include "document_reader.h"
#include "data_structures.h" // Đảm bảo data_structures.h là file C

// Đọc nội dung một file text
char* read_file(const char* filepath) {
    if (filepath == NULL) return NULL;

    FILE* file = fopen(filepath, "r");
    if (!file) {
        // printf("Lỗi: Không thể mở file %s\n", filepath); // Có thể bỏ comment để debug
        return NULL;
    }

    // Tìm kích thước file
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Cấp phát bộ nhớ và đọc
    char* content = (char*)malloc(file_size + 1); // +1 cho ký tự null
    if (!content) {
        // printf("Lỗi: Không đủ bộ nhớ cho file %s\n", filepath);
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(content, 1, file_size, file);
    content[bytes_read] = '\0'; // Kết thúc chuỗi

    fclose(file);
    return content;
}

// Kiểm tra file có phải là .txt không
int is_txt_file(const char* filename) {
    if (filename == NULL) return 0;
    int len = strlen(filename);
    if (len < 4) return 0;

    // Kiểm tra đuôi .txt (không phân biệt hoa thường)
    const char* ext = filename + len - 4;
    return (strcasecmp(ext, ".txt") == 0); // strcasecmp là POSIX, thường có sẵn
}

// ========== HÀM QUẢN LÝ DOCUMENT COLLECTION ==========

// Tạo DocumentCollection mới
DocumentCollection* create_document_collection(const char* folder_path) {
    if (folder_path == NULL) return NULL;

    DocumentCollection* collection = (DocumentCollection*)malloc(sizeof(DocumentCollection));
    if (!collection) return NULL;

    collection->capacity = 100; // Bắt đầu với 100 files
    collection->count = 0;
    collection->files = (DocumentFile*)malloc(collection->capacity * sizeof(DocumentFile));
    if (!collection->files) {
        free(collection);
        return NULL;
    }
    
    collection->folder_path = (char*)malloc(strlen(folder_path) + 1);
    if (!collection->folder_path) {
        free(collection->files);
        free(collection);
        return NULL;
    }
    strcpy(collection->folder_path, folder_path);

    return collection;
}

// Mở rộng capacity nếu cần
void expand_document_collection(DocumentCollection* collection) {
    if (collection == NULL) return;

    if (collection->count >= collection->capacity) {
        collection->capacity *= 2;
        DocumentFile* new_files = (DocumentFile*)realloc(collection->files, 
                                                         collection->capacity * sizeof(DocumentFile));
        if (!new_files) {
            // Xử lý lỗi realloc: có thể in thông báo hoặc thoát chương trình
            fprintf(stderr, "Lỗi: Không thể mở rộng bộ nhớ cho DocumentCollection.\n");
            // Trong ứng dụng thực tế có thể muốn xử lý lỗi một cách duyên dáng hơn
            exit(EXIT_FAILURE); 
        }
        collection->files = new_files;
    }
}

// Hàm hỗ trợ: Thêm một file tài liệu vào DocumentCollection
bool add_document_to_collection(DocumentCollection* collection, const char* filename, const char* filepath) {
    if (collection == NULL || filename == NULL || filepath == NULL) return false;

    expand_document_collection(collection); // Mở rộng nếu cần

    DocumentFile* doc = &collection->files[collection->count];

    doc->filename = strdup(filename); // strdup cấp phát và sao chép
    if (!doc->filename) return false;

    doc->filepath = strdup(filepath);
    if (!doc->filepath) {
        free(doc->filename);
        return false;
    }

    doc->content = read_file(filepath); // Đọc nội dung file
    if (!doc->content) {
        free(doc->filename);
        free(doc->filepath);
        return false;
    }
    doc->file_size = strlen(doc->content); // Lấy kích thước nội dung đã đọc
    doc->doc_id = collection->count;       // Gán ID đơn giản

    collection->count++;
    return true;
}

// ========== HÀM ĐỌC FOLDER ==========

// Đọc tất cả file .txt trong một folder
DocumentCollection* read_folder(const char* folder_path) {
    if (folder_path == NULL) return NULL;

    DIR* dir = opendir(folder_path);
    if (!dir) {
        printf("Lỗi: Không thể mở folder %s\n", folder_path);
        return NULL;
    }

    DocumentCollection* collection = create_document_collection(folder_path);
    if (!collection) {
        closedir(dir);
        return NULL;
    }

    struct dirent* entry;
    char filepath[1024]; // Buffer để xây dựng đường dẫn đầy đủ
    int successful_reads = 0;

    printf("Đang đọc folder: %s\n", folder_path);

    while ((entry = readdir(dir)) != NULL) {
        // Bỏ qua "." và ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Tạo đường dẫn đầy đủ
        // Kiểm tra xem đường dẫn có bị tràn buffer không
        if (snprintf(filepath, sizeof(filepath), "%s/%s", folder_path, entry->d_name) >= sizeof(filepath)) {
            fprintf(stderr, "Cảnh báo: Đường dẫn quá dài, bỏ qua file: %s/%s\n", folder_path, entry->d_name);
            continue;
        }

        // Kiểm tra xem có phải là file thông thường và là .txt không
        struct stat file_stat;
        if (stat(filepath, &file_stat) == 0 && S_ISREG(file_stat.st_mode)) {
            if (is_txt_file(entry->d_name)) { // Chỉ xử lý file .txt
                if (add_document_to_collection(collection, entry->d_name, filepath)) {
                    successful_reads++;
                    if (successful_reads % 10 == 0) {
                        printf("Đã đọc %d files...\n", successful_reads);
                    }
                } else {
                    printf("Cảnh báo: Không thể thêm file %s vào collection (có thể do lỗi đọc nội dung hoặc cấp phát bộ nhớ).\n", filepath);
                }
            }
        }
    }

    closedir(dir);

    printf("Hoàn thành! Đã đọc %d files từ folder %s\n",
           collection->count, folder_path);

    return collection;
}

// Đọc nhiều folder cùng lúc
DocumentCollection* read_multiple_folders(char** folder_paths, int num_folders) {
    if (folder_paths == NULL || num_folders == 0) return NULL;

    // Bắt đầu với folder đầu tiên
    DocumentCollection* main_collection = read_folder(folder_paths[0]);
    if (!main_collection) return NULL; // Nếu folder đầu tiên không đọc được, thoát

    // Thêm các folder còn lại
    for (int i = 1; i < num_folders; i++) {
        DocumentCollection* temp_collection = read_folder(folder_paths[i]);
        if (temp_collection) {
            // Merge vào collection chính
            for (int j = 0; j < temp_collection->count; j++) {
                expand_document_collection(main_collection); // Mở rộng collection chính

                DocumentFile* src = &temp_collection->files[j];
                DocumentFile* dst = &main_collection->files[main_collection->count];

                // Chuyển quyền sở hữu bộ nhớ
                dst->filename = src->filename;
                dst->filepath = src->filepath;
                dst->content = src->content;
                dst->file_size = src->file_size;
                dst->doc_id = main_collection->count;

                // Đặt NULL cho con trỏ nguồn để free_document_collection của temp_collection
                // không cố gắng giải phóng bộ nhớ đã được chuyển.
                src->filename = NULL;
                src->filepath = NULL;
                src->content = NULL;

                main_collection->count++;
            }
            free_document_collection(temp_collection); // Giải phóng cấu trúc temp_collection (không giải phóng nội dung đã chuyển)
        }
    }
    return main_collection;
}

// Giải phóng toàn bộ bộ nhớ được cấp phát cho DocumentCollection
void free_document_collection(DocumentCollection* collection) {
    if (!collection) return;

    if (collection->files) {
        for (int i = 0; i < collection->count; i++) {
            if (collection->files[i].filename) free(collection->files[i].filename);
            if (collection->files[i].filepath) free(collection->files[i].filepath);
            if (collection->files[i].content) free(collection->files[i].content);
        }
        free(collection->files);
    }
    
    if (collection->folder_path) {
        free(collection->folder_path);
    }
    free(collection);
}