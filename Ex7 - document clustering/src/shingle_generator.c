#include "shingle_generator.h" // Bao gồm header file của chính nó

// Tạo ShingleSet mới với dung lượng ban đầu
ShingleSet* create_shingle_set(int initial_capacity) {
    ShingleSet* set = (ShingleSet*)malloc(sizeof(ShingleSet));
    if (set == NULL) return NULL;

    set->items = (char**)malloc(initial_capacity * sizeof(char*));
    if (set->items == NULL) {
        free(set);
        return NULL;
    }

    set->count = 0;
    set->capacity = initial_capacity;

    return set;
}

// Kiểm tra shingle có tồn tại trong tập hợp không
bool contains_shingle(const ShingleSet* set, const char* shingle) {
    if (set == NULL || shingle == NULL) return false;

    for (int i = 0; i < set->count; i++) {
        if (set->items[i] != NULL && strcmp(set->items[i], shingle) == 0) {
            return true;
        }
    }
    return false;
}

// Thêm shingle vào tập hợp. Chỉ thêm nếu shingle chưa tồn tại.
int add_shingle(ShingleSet* set, const char* shingle) {
    if (set == NULL || shingle == NULL) return 0;

    // Kiểm tra và mở rộng dung lượng nếu cần
    if (set->count >= set->capacity) {
        int new_capacity = set->capacity == 0 ? 1 : set->capacity * 2; // Đảm bảo capacity tăng nếu ban đầu là 0
        char** new_items = (char**)realloc(set->items, new_capacity * sizeof(char*));
        if (new_items == NULL) {
            return 0; // Không đủ bộ nhớ để mở rộng
        }
        set->items = new_items;
        set->capacity = new_capacity;
    }

    // Cấp phát bộ nhớ cho shingle và sao chép nội dung
    set->items[set->count] = (char*)malloc((strlen(shingle) + 1) * sizeof(char));
    if (set->items[set->count] == NULL) {
        return 0; // Không đủ bộ nhớ cho shingle
    }
    strcpy(set->items[set->count], shingle);
    set->count++;

    return 1; // Thêm thành công
}

// Tạo character shingles (k-shingles) từ một chuỗi văn bản
ShingleSet* create_character_shingles(const char* text, int k) {
    if (text == NULL || k <= 0) return NULL;

    int text_len = strlen(text);
    if (text_len < k) {
        // Nếu văn bản quá ngắn, trả về một ShingleSet rỗng hoặc NULL tùy ý định
        // Tạo một set rỗng để tránh trả về NULL làm lỗi logic tiếp theo
        return create_shingle_set(1); // Cấp phát nhỏ nhất có thể
    }

    // Ước tính dung lượng ban đầu: text_len - k + 1 shingles
    ShingleSet* set = create_shingle_set(text_len - k + 1);
    if (set == NULL) return NULL;

    for (int i = 0; i <= text_len - k; i++) {
        char* shingle = (char*)malloc((k + 1) * sizeof(char));
        if (shingle == NULL) {
            // Nếu không cấp phát được shingle, bỏ qua và cố gắng tiếp tục
            // hoặc bạn có thể chọn free_shingle_set(set) và return NULL;
            continue;
        }
        strncpy(shingle, text + i, k);
        shingle[k] = '\0'; // Đảm bảo kết thúc chuỗi

        // Chỉ thêm nếu shingle chưa tồn tại trong tập hợp
        // Sau khi kiểm tra, shingle_to_add sẽ được cấp phát lại trong add_shingle
        // nên cần giải phóng `shingle` ở đây.
        if (!contains_shingle(set, shingle)) {
            if (!add_shingle(set, shingle)) {
                // Xử lý lỗi nếu không thể thêm shingle (ví dụ: hết bộ nhớ)
                free(shingle); // Giải phóng shingle tạm thời
                free_shingle_set(set); // Giải phóng toàn bộ set đã tạo
                return NULL;
            }
        }
        free(shingle); // Giải phóng shingle tạm thời đã tạo ở đây
    }

    return set;
}

// Tách từ từ text, chuyển về chữ thường và loại bỏ dấu câu cơ bản
char** tokenize_text(const char* text, int* num_tokens) {
    if (text == NULL || num_tokens == NULL) {
        if (num_tokens != NULL) *num_tokens = 0;
        return NULL;
    }

    int len = strlen(text);
    char* text_copy = (char*)malloc((len + 1) * sizeof(char));
    if (text_copy == NULL) {
        *num_tokens = 0;
        return NULL;
    }
    strcpy(text_copy, text);

    // Bước 1: Đếm số từ để cấp phát mảng
    int count = 0;
    char* temp_copy_for_counting = (char*)malloc((len + 1) * sizeof(char));
    if (temp_copy_for_counting == NULL) {
        free(text_copy);
        *num_tokens = 0;
        return NULL;
    }
    strcpy(temp_copy_for_counting, text);

    char* token_counting = strtok(temp_copy_for_counting, " \t\n\r\f\v.,!?;:()"); // Thêm dấu ngoặc
    while (token_counting != NULL) {
        count++;
        token_counting = strtok(NULL, " \t\n\r\f\v.,!?;:()");
    }
    free(temp_copy_for_counting); // Giải phóng bản sao tạm thời dùng để đếm

    if (count == 0) {
        free(text_copy);
        *num_tokens = 0;
        return NULL;
    }

    // Bước 2: Cấp phát mảng con trỏ và sao chép/tokenize từ
    char** tokens = (char**)malloc(count * sizeof(char*));
    if (tokens == NULL) {
        free(text_copy);
        *num_tokens = 0;
        return NULL;
    }

    int i = 0;
    char* token = strtok(text_copy, " \t\n\r\f\v.,!?;:()"); // Sử dụng text_copy ban đầu cho strtok
    while (token != NULL && i < count) {
        // Loại bỏ ký tự không phải chữ/số và chuyển về chữ thường
        char* cleaned_token = (char*)malloc((strlen(token) + 1) * sizeof(char));
        if (cleaned_token == NULL) {
            // Xử lý lỗi: giải phóng các tokens đã cấp phát trước đó
            for (int j = 0; j < i; j++) free(tokens[j]);
            free(tokens);
            free(text_copy);
            *num_tokens = 0;
            return NULL;
        }

        int k = 0;
        for (int j = 0; token[j] != '\0'; j++) {
            if (isalnum((unsigned char)token[j])) { // isalnum kiểm tra chữ cái hoặc số
                cleaned_token[k++] = (char)tolower((unsigned char)token[j]);
            }
        }
        cleaned_token[k] = '\0'; // Kết thúc chuỗi

        if (strlen(cleaned_token) > 0) { // Chỉ thêm từ không rỗng sau khi làm sạch
            tokens[i] = cleaned_token;
            i++;
        } else {
            free(cleaned_token); // Giải phóng nếu từ rỗng
        }
        token = strtok(NULL, " \t\n\r\f\v.,!?;:()");
    }

    free(text_copy);
    *num_tokens = i; // Số lượng token thực tế sau khi làm sạch
    // Có thể realloc tokens nếu count ban đầu lớn hơn số token thực tế sau khi làm sạch
    // Tuy nhiên, việc này phức tạp hơn và có thể bỏ qua nếu chấp nhận một số vùng trống
    return tokens;
}

// Tạo word n-grams từ một chuỗi văn bản
ShingleSet* create_word_ngrams(const char* text, int n) {
    if (text == NULL || n <= 0) return NULL;

    int num_tokens;
    char** tokens = tokenize_text(text, &num_tokens); // Tokenize văn bản
    if (tokens == NULL || num_tokens == 0) {
        // Nếu không có token hoặc lỗi khi tokenize
        return create_shingle_set(1); // Trả về ShingleSet rỗng
    }

    // Nếu số lượng token ít hơn n, không thể tạo n-grams
    if (num_tokens < n) {
        // Giải phóng tokens đã cấp phát bởi tokenize_text
        for (int i = 0; i < num_tokens; i++) {
            free(tokens[i]);
        }
        free(tokens);
        return create_shingle_set(1); // Trả về ShingleSet rỗng
    }

    // Ước tính dung lượng ban đầu: num_tokens - n + 1 n-grams
    ShingleSet* set = create_shingle_set(num_tokens - n + 1);
    if (set == NULL) {
        // Giải phóng tokens nếu không cấp phát được ShingleSet
        for (int i = 0; i < num_tokens; i++) {
            free(tokens[i]);
        }
        free(tokens);
        return NULL;
    }

    for (int i = 0; i <= num_tokens - n; i++) {
        // Tính tổng độ dài của n-gram và số lượng khoảng trắng
        int total_len = 0;
        for (int j = 0; j < n; j++) {
            total_len += strlen(tokens[i + j]);
        }
        total_len += (n - 1); // Cộng thêm khoảng trắng giữa các từ

        char* ngram = (char*)malloc((total_len + 1) * sizeof(char));
        if (ngram == NULL) {
            // Xử lý lỗi: giải phóng tất cả và thoát
            free_shingle_set(set);
            for (int k = 0; k < num_tokens; k++) {
                free(tokens[k]);
            }
            free(tokens);
            return NULL;
        }

        // Ghép các từ lại thành n-gram
        strcpy(ngram, tokens[i]);
        for (int j = 1; j < n; j++) {
            strcat(ngram, " ");
            strcat(ngram, tokens[i + j]);
        }

        // Chỉ thêm nếu n-gram chưa tồn tại trong tập hợp
        if (!contains_shingle(set, ngram)) {
            if (!add_shingle(set, ngram)) {
                // Xử lý lỗi nếu không thể thêm n-gram
                free(ngram); // Giải phóng n-gram tạm thời
                free_shingle_set(set);
                for (int k = 0; k < num_tokens; k++) {
                    free(tokens[k]);
                }
                free(tokens);
                return NULL;
            }
        }
        free(ngram); // Giải phóng n-gram tạm thời đã tạo ở đây
    }

    // Giải phóng tokens đã cấp phát bởi tokenize_text
    for (int i = 0; i < num_tokens; i++) {
        free(tokens[i]);
    }
    free(tokens);

    return set;
}

// Giải phóng toàn bộ bộ nhớ của ShingleSet
void free_shingle_set(ShingleSet* set) {
    if (set == NULL) return;

    if (set->items != NULL) {
        for (int i = 0; i < set->count; i++) {
            if (set->items[i] != NULL) {
                free(set->items[i]); // Giải phóng từng chuỗi shingle
            }
        }
        free(set->items); // Giải phóng mảng con trỏ
    }
    free(set); // Giải phóng cấu trúc ShingleSet
}