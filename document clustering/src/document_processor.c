#include "document_processor.h" // Bao gồm header file của chính nó
#include "shingle_generator.h"  // Cần include để sử dụng hàm tokenize_text

// Hàm hỗ trợ: Tìm kiếm một từ trong mảng stopwords
// Trả về true nếu từ là stopword, ngược lại false
static bool is_stopword(const char* word, const char** stopwords, int num_stopwords) {
    if (word == NULL || stopwords == NULL) return false;
    for (int i = 0; i < num_stopwords; i++) {
        if (stopwords[i] != NULL && strcmp(word, stopwords[i]) == 0) {
            return true;
        }
    }
    return false;
}

// Chuyển văn bản thành chữ thường
void to_lowercase(char* str) {
    if (str == NULL) return;
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = (char)tolower((unsigned char)str[i]);
    }
}

// Loại bỏ các ký tự không phải chữ cái hoặc số và thay thế bằng khoảng trắng
void remove_non_alphanumeric(char* str) {
    if (str == NULL) return;
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isalnum((unsigned char)str[i]) && !isspace((unsigned char)str[i])) {
            str[i] = ' ';
        }
    }
}

// Loại bỏ khoảng trắng thừa (đầu, cuối và nhiều khoảng trắng liên tiếp)
void trim_and_reduce_whitespace(char* str) {
    if (str == NULL) return;

    int len = strlen(str);
    if (len == 0) return;

    // 1. Loại bỏ khoảng trắng đầu
    int i = 0;
    while (i < len && isspace((unsigned char)str[i])) {
        i++;
    }
    if (i > 0) {
        memmove(str, str + i, len - i + 1); // Di chuyển phần còn lại của chuỗi
        len -= i;
    }

    if (len == 0) return; // Chuỗi rỗng sau khi trim đầu

    // 2. Loại bỏ khoảng trắng cuối
    i = len - 1;
    while (i >= 0 && isspace((unsigned char)str[i])) {
        i--;
    }
    str[i + 1] = '\0'; // Kết thúc chuỗi tại đây
    len = i + 1; // Cập nhật độ dài mới

    if (len == 0) return; // Chuỗi rỗng sau khi trim cuối

    // 3. Giảm nhiều khoảng trắng liên tiếp thành một khoảng trắng duy nhất
    int write_idx = 0;
    int read_idx = 0;
    bool in_space = false;

    while (str[read_idx] != '\0') {
        if (isspace((unsigned char)str[read_idx])) {
            if (!in_space) { // Nếu đây là khoảng trắng đầu tiên trong một chuỗi khoảng trắng
                str[write_idx++] = ' ';
                in_space = true;
            }
        } else { // Nếu là ký tự không phải khoảng trắng
            str[write_idx++] = str[read_idx];
            in_space = false;
        }
        read_idx++;
    }
    str[write_idx] = '\0'; // Kết thúc chuỗi
}

// Tách từ (tokenization) và loại bỏ stop words, số nguyên và từ có 1 ký tự.
char* tokenize_and_filter(const char* text, const char** stopwords, int num_stopwords) {
    if (text == NULL) return NULL;

    int num_initial_tokens;
    // Gọi hàm tokenize_text từ shingle_generator để có các token đã được làm sạch cơ bản
    char** initial_tokens = tokenize_text(text, &num_initial_tokens);
    if (initial_tokens == NULL || num_initial_tokens == 0) {
        // Trả về chuỗi rỗng nếu không có token hoặc lỗi
        char* empty_str = (char*)malloc(1);
        if (empty_str) empty_str[0] = '\0';
        return empty_str;
    }

    // Buffer để xây dựng chuỗi kết quả
    size_t current_buffer_size = 256; // Kích thước ban đầu
    char* result_buffer = (char*)malloc(current_buffer_size);
    if (result_buffer == NULL) {
        // Giải phóng tokens nhận được từ tokenize_text trước khi thoát
        for (int i = 0; i < num_initial_tokens; i++) free(initial_tokens[i]);
        free(initial_tokens);
        return NULL;
    }
    result_buffer[0] = '\0'; // Khởi tạo chuỗi rỗng

    bool first_token_added = false;

    for (int i = 0; i < num_initial_tokens; i++) {
        char* token = initial_tokens[i];
        if (token == NULL || strlen(token) == 0) continue; // Bỏ qua token rỗng

        // Kiểm tra nếu là tất cả chữ số
        bool all_digits = true;
        for (int j = 0; token[j] != '\0'; j++) {
            if (!isdigit((unsigned char)token[j])) {
                all_digits = false;
                break;
            }
        }

        // Lọc stop word, số nguyên, và từ có 1 ký tự
        if (!all_digits && strlen(token) > 1 && !is_stopword(token, stopwords, num_stopwords)) {
            // Thêm khoảng trắng trước nếu không phải từ đầu tiên
            if (first_token_added) {
                // Kiểm tra và mở rộng buffer nếu cần cho khoảng trắng và từ mới
                size_t needed_len = strlen(result_buffer) + 1 + strlen(token) + 1;
                if (needed_len > current_buffer_size) {
                    current_buffer_size = needed_len * 2; // Gấp đôi kích thước buffer
                    char* temp_realloc = (char*)realloc(result_buffer, current_buffer_size);
                    if (temp_realloc == NULL) {
                        // Xử lý lỗi realloc: giải phóng tất cả và thoát
                        for (int k = 0; k < num_initial_tokens; k++) free(initial_tokens[k]);
                        free(initial_tokens);
                        free(result_buffer);
                        return NULL;
                    }
                    result_buffer = temp_realloc;
                }
                strcat(result_buffer, " ");
            }

            // Thêm từ vào buffer
            size_t needed_len = strlen(result_buffer) + strlen(token) + 1;
            if (needed_len > current_buffer_size) { // Kiểm tra lại sau khi thêm khoảng trắng
                 current_buffer_size = needed_len * 2;
                 char* temp_realloc = (char*)realloc(result_buffer, current_buffer_size);
                 if (temp_realloc == NULL) {
                    for (int k = 0; k < num_initial_tokens; k++) free(initial_tokens[k]);
                    free(initial_tokens);
                    free(result_buffer);
                    return NULL;
                 }
                 result_buffer = temp_realloc;
            }
            strcat(result_buffer, token);
            first_token_added = true;
        }
    }

    // Giải phóng tokens nhận được từ tokenize_text
    for (int i = 0; i < num_initial_tokens; i++) {
        free(initial_tokens[i]);
    }
    free(initial_tokens);

    // Tối ưu hóa kích thước buffer cuối cùng
    char* final_result = (char*)realloc(result_buffer, strlen(result_buffer) + 1);
    if (final_result == NULL) {
        // realloc có thể trả về NULL khi cố gắng thu nhỏ nếu có lỗi,
        // trong trường hợp này, trả về buffer cũ nếu nó vẫn hợp lệ.
        return result_buffer;
    }
    return final_result;
}

// Quy trình tiền xử lý tài liệu hoàn chỉnh
char* preprocess_document_pipeline(const char* content, const char** stopwords, int num_stopwords) {
    if (content == NULL) return NULL;

    // Bước 1: Tạo một bản sao có thể sửa đổi của nội dung
    char* mutable_content = strdup(content);
    if (mutable_content == NULL) return NULL;

    // Bước 2: Chuyển văn bản thành chữ thường
    to_lowercase(mutable_content);

    // Bước 3: Loại bỏ các ký tự không phải chữ và số
    remove_non_alphanumeric(mutable_content);

    // Bước 4: Loại bỏ khoảng trắng thừa
    trim_and_reduce_whitespace(mutable_content);

    // Bước 5: Tokenize và lọc stop words, số nguyên, từ 1 ký tự
    char* filtered_content = tokenize_and_filter(mutable_content, stopwords, num_stopwords);

    free(mutable_content); // Giải phóng bản sao tạm thời

    return filtered_content; // Người gọi có trách nhiệm giải phóng chuỗi này
}