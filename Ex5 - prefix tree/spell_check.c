#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define ALPHABET_SIZE 26
#define MAX_WORD_LENGTH 100
#define MAX_SUGGESTIONS 20
#define MAX_LINE_LENGTH 1000
#define MAX_EDIT_DISTANCE 2 // Ngưỡng khoảng cách chỉnh sửa chấp nhận được

// Link tải file words.txt: https://github.com/dwyl/english-words/blob/master/words.txt

// Prefix Tree (Trie) node structure
typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    bool isEndOfWord;
} TrieNode;

// Function to create a new Trie node
TrieNode* createNode() {
    TrieNode* node = (TrieNode*)malloc(sizeof(TrieNode));
    if (node == NULL) {
        perror("Dynamic Allocation Error for TrieNode"); // Use perror for system errors
        return NULL;
    }
    node->isEndOfWord = false;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        node->children[i] = NULL;
    }
    return node;
}

// Convert character to index (a-z to 0-25)
int charToIndex(char c) {
    if (c >= 'a' && c <= 'z') {
        return c - 'a';
    } else if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    }
    return -1; // Invalid character
}

// Convert string to lowercase
void toLowerCase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

// Insert a word into the Trie
void insert(TrieNode* root, const char* word) {
    TrieNode* current = root;
    int length = strlen(word);
    
    for (int i = 0; i < length; i++) {
        int index = charToIndex(word[i]);
        if (index == -1) return; // Skip words with invalid characters
        
        if (!current->children[index]) {
            current->children[index] = createNode();
        }
        current = current->children[index];
    }
    
    current->isEndOfWord = true;
}

// Search for a word in the Trie
bool search(TrieNode* root, const char* word) {
    TrieNode* current = root;
    int length = strlen(word);
    
    for (int i = 0; i < length; i++) {
        int index = charToIndex(word[i]);
        if (index == -1 || !current->children[index]) {
            return false;
        }
        current = current->children[index];
    }
    
    return current && current->isEndOfWord;
}

// Hàm hỗ trợ tìm giá trị nhỏ nhất (min)
int min(int a, int b) {
    return (a < b) ? a : b;
}

// Hàm đệ quy để tìm kiếm sửa lỗi trong Trie (Levenshtein trên Trie)
// misspelled_word: từ bị sai chính tả gốc (đã chuyển về chữ thường)
// misspelled_word_len: độ dài của từ bị sai chính tả
// current_node: nút Trie hiện tại đang duyệt
// current_trie_word_chars: mảng ký tự để xây dựng từ trong Trie (đang duyệt)
// current_trie_word_depth: độ sâu hiện tại trong Trie (cũng là độ dài tiền tố của current_trie_word_chars)
// dp_matrix: ma trận DP đầy đủ (misspelled_word_len + 1 hàng, mỗi hàng là độ sâu hiện tại trong Trie)
// suggestions: mảng để lưu các từ gợi ý
// count: con trỏ đến biến đếm số lượng gợi ý
void findClosestWords(const char* misspelled_word, int misspelled_word_len,
                      TrieNode* current_node, 
                      char* current_trie_word_chars, int current_trie_word_depth,
                      int** dp_matrix, // Ma trận DP 2D (cấp phát động)
                      char suggestions[][MAX_WORD_LENGTH], int* count) {

    // Nếu đã tìm đủ số lượng gợi ý tối đa, dừng lại
    if (*count >= MAX_SUGGESTIONS) {
        return;
    }

    // Lấy hàng DP hiện tại từ ma trận
    int* current_dp_row = dp_matrix[current_trie_word_depth];

    // Nếu nút hiện tại là một từ hoàn chỉnh và khoảng cách chỉnh sửa trong ngưỡng chấp nhận
    if (current_node->isEndOfWord) {
        if (current_dp_row[misspelled_word_len] <= MAX_EDIT_DISTANCE) {
            current_trie_word_chars[current_trie_word_depth] = '\0';
            strcpy(suggestions[*count], current_trie_word_chars);
            (*count)++;
            if (*count >= MAX_SUGGESTIONS) return; // Kiểm tra lại sau khi thêm từ
        }
    }

    // Duyệt qua các nút con
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        TrieNode* child_node = current_node->children[i];
        if (child_node) {
            // Lấy hàng DP cho nút con (độ sâu tiếp theo)
            int* next_dp_row = dp_matrix[current_trie_word_depth + 1];
            
            // Khởi tạo cột đầu tiên của hàng mới
            next_dp_row[0] = current_trie_word_depth + 1;

            // Tính toán khoảng cách chỉnh sửa cho hàng DP mới
            for (int j = 1; j <= misspelled_word_len; j++) {
                int cost = (misspelled_word[j-1] == ('a' + i)) ? 0 : 1;
                next_dp_row[j] = min(min(current_dp_row[j] + 1, // Xóa 
                                      next_dp_row[j-1] + 1), // Thêm 
                                      current_dp_row[j-1] + cost); // Thay thế hoặc khớp
            }

            // Kiểm tra "cắt tỉa" (pruning)
            // Tìm khoảng cách tối thiểu trong hàng DP mới nhất
            int min_dist_in_row = next_dp_row[0];
            for (int j = 1; j <= misspelled_word_len; j++) {
                if (next_dp_row[j] < min_dist_in_row) {
                    min_dist_in_row = next_dp_row[j];
                }
            }
            
            if (min_dist_in_row <= MAX_EDIT_DISTANCE) { // Chỉ duyệt tiếp nếu vẫn có tiềm năng tìm được từ gần đúng
                current_trie_word_chars[current_trie_word_depth] = 'a' + i;
                findClosestWords(misspelled_word, misspelled_word_len, child_node, 
                                current_trie_word_chars, current_trie_word_depth + 1, 
                                dp_matrix, 
                                suggestions, count);
            }
        }
    }
}


// Generate spelling suggestions for a misspelled word
int generateSuggestions(TrieNode* root, const char* misspelled_word, 
                       char suggestions[][MAX_WORD_LENGTH]) {
    
    int misspelled_word_len = strlen(misspelled_word);
    int count = 0;
    
    // Cấp phát động ma trận DP
    // Số hàng tối đa = MAX_WORD_LENGTH + 1 (độ sâu tối đa của Trie + 1 cho hàng gốc)
    // Số cột = misspelled_word_len + 1
    int** dp_matrix = (int**)malloc(sizeof(int*) * (MAX_WORD_LENGTH + 1));
    if (dp_matrix == NULL) {
        perror("Lỗi cấp phát bộ nhớ cho ma trận DP");
        return 0;
    }
    for (int i = 0; i <= MAX_WORD_LENGTH; i++) {
        dp_matrix[i] = (int*)malloc(sizeof(int) * (misspelled_word_len + 1));
        if (dp_matrix[i] == NULL) {
            perror("Lỗi cấp phát bộ nhớ cho hàng DP");
            // Giải phóng các hàng đã cấp phát trước đó nếu có lỗi
            for (int k = 0; k < i; k++) {
                free(dp_matrix[k]);
            }
            free(dp_matrix);
            return 0;
        }
    }

    // Khởi tạo hàng DP đầu tiên (cho gốc Trie, độ sâu 0)
    // dp_matrix[0][j] = j
    for (int j = 0; j <= misspelled_word_len; j++) {
        dp_matrix[0][j] = j;
    }

    char current_trie_word_chars[MAX_WORD_LENGTH]; // Để xây dựng từ Trie đang duyệt

    // Bắt đầu tìm kiếm đệ quy từ gốc Trie
    findClosestWords(misspelled_word, misspelled_word_len,
                     root, 
                     current_trie_word_chars, 0, // Bắt đầu ở độ sâu 0
                     dp_matrix, 
                     suggestions, &count);
    
    // Giải phóng bộ nhớ của ma trận DP
    for (int i = 0; i <= MAX_WORD_LENGTH; i++) {
        free(dp_matrix[i]);
    }
    free(dp_matrix);

    return count;
}


// Load words from file into Trie
int loadDictionaryFromFile(TrieNode* root, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Lỗi: Không thể mở file '%s'\n", filename);
        printf("Hãy đảm bảo file words.txt có trong thư mục hiện tại hoặc cung cấp đúng đường dẫn.\n");
        printf("Bạn có thể tải file từ: https://raw.githubusercontent.com/dwyl/english-words/refs/heads/master/words.txt\n\n");
        return 0;
    }
    
    char word[MAX_WORD_LENGTH];
    int word_count = 0;
    
    printf("Đang tải từ điển từ file '%s'...\n", filename);
    
    while (fgets(word, sizeof(word), file)) {
        // Remove newline character
        word[strcspn(word, "\n\r")] = 0;
        
        // Skip empty lines
        if (strlen(word) == 0) continue;
        
        // Convert to lowercase and insert
        toLowerCase(word);
        insert(root, word);
        word_count++;
        
        // Show progress every 10000 words
        if (word_count % 10000 == 0) {
            printf("Đã tải %d từ...\n", word_count);
        }
    }
    
    fclose(file);
    printf("Hoàn thành! Đã tải %d từ vào từ điển.\n\n", word_count);
    return word_count;
}

// Extract words from a text string
int extractWords(const char* text, char words[][MAX_WORD_LENGTH]) {
    int word_count = 0;
    int i = 0, j = 0;
    char current_word[MAX_WORD_LENGTH];
    
    while (text[i] != '\0' && word_count < 100) {
        if (isalpha(text[i])) {
            current_word[j++] = tolower(text[i]);
        } else {
            if (j > 0) {
                current_word[j] = '\0';
                strcpy(words[word_count], current_word);
                word_count++;
                j = 0;
            }
        }
        i++;
    }
    
    // Add the last word if exists
    if (j > 0) {
        current_word[j] = '\0';
        strcpy(words[word_count], current_word);
        word_count++;
    }
    
    return word_count;
}

// Check spelling of text and display results
void checkSpelling(TrieNode* root, const char* text) {
    printf("--- KẾT QUẢ KIỂM TRA CHÍNH TẢ ---\n");
    printf("Văn bản gốc: %s\n\n", text);

    bool has_errors = false;
    int i = 0, j = 0;
    char current_word[MAX_WORD_LENGTH];
    int text_len = strlen(text);

    while (i <= text_len) { // include null terminator
        if (isalpha(text[i])) {
            if (j < MAX_WORD_LENGTH - 1) {
                current_word[j++] = tolower(text[i]);
            }
            // else: ignore extra chars in too-long word
        } else {
            if (j > 0) {
                current_word[j] = '\0';
                // Process the word immediately
                if (!search(root, current_word)) {
                    has_errors = true;
                    printf("Từ SAI: '%s'\n", current_word);

                    char suggestions[MAX_SUGGESTIONS][MAX_WORD_LENGTH];
                    int suggestion_count = generateSuggestions(root, current_word, suggestions);

                    if (suggestion_count > 0) {
                        printf("--> Gợi ý sửa: ");
                        for (int k = 0; k < suggestion_count; k++) {
                            printf("'%s'", suggestions[k]);
                            if (k < suggestion_count - 1) printf(", ");
                        }
                        printf("\n");
                    } else {
                        printf("--> Không tìm thấy gợi ý phù hợp.\n");
                    }
                    printf("\n");
                } else {
                    printf("Từ ĐÚNG: '%s'\n", current_word);
                }
                j = 0;
            }
        }
        i++;
    }

    if (!has_errors) {
        printf("Tất cả các từ đều được viết đúng chính tả!\n");
    }

    printf("\n------------------------\n\n");
}

// Free the Trie memory
void freeTrie(TrieNode* root) {
    if (!root) return;
    
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (root->children[i]) {
            freeTrie(root->children[i]);
        }
    }
    
    free(root);
}

// Load sample words if file is not available
void loadSampleDictionary(TrieNode* root) {
    const char* sample_words[] = {
        "hello", "world", "computer", "programming", "language",
        "english", "spell", "check", "correct", "word",
        "text", "file", "data", "structure", "algorithm",
        "tree", "node", "search", "insert", "delete",
        "the", "and", "for", "are", "but", "not", "you",
        "all", "can", "her", "was", "one", "our", "had",
        "have", "what", "were", "they", "said", "each",
        "which", "their", "time", "will", "about", "make",
        "opinion", "artificial", "intelligence", "big", "technology", 
        "people", "concerned", "implications", "believe", "potential", 
        "solve", "humanity", "greatest", "challenges", "computer", 
        "screen", "flickered", "tried", "type", "fingers", "awkward", 
        "difficult", "concentrate", "noise", "door", "travel", "wonderful", 
        "different", "cultures", "summer", "small", "village", "mountains", 
        "hospital", "incredible", "wish", "could", "have", "stayed", "longer", "vacation", "short"
    };
    
    int num_words = sizeof(sample_words) / sizeof(sample_words[0]);
    
    printf("Đang tải từ điển mẫu (%d từ)...\n", num_words);
    for (int i = 0; i < num_words; i++) {
        insert(root, sample_words[i]);
    }
    printf("Hoàn thành việc tải từ điển mẫu!\n\n");
}

int main() {
    TrieNode* root = createNode();
    char input_text[MAX_LINE_LENGTH];
    
    printf("=== HỆ THỐNG KIỂM TRA CHÍNH TẢ ===\n");
    printf("Sử dụng cấu trúc dữ liệu Prefix Tree (Trie)\n\n");
    
    // Try to load dictionary from file
    // Đảm bảo đường dẫn này đúng với vị trí file words.txt của bạn
    int loaded = loadDictionaryFromFile(root, "data/spell_check/words.txt"); 
    
    // If file loading failed, use sample dictionary
    if (loaded == 0) {
        printf("Sử dụng từ điển mẫu thay thế...\n\n");
        loadSampleDictionary(root);
    }
    
    printf("Hướng dẫn sử dụng:\n");
    printf("- Nhập văn bản để kiểm tra chính tả\n");
    printf("- Nhập 'file' để kiểm tra chính tả cho file 'spell_check_input.txt'\n");
    printf("- Nhập 'quit' để thoát chương trình\n");
    printf("- Hệ thống sẽ tìm các từ SAI và đưa ra gợi ý sửa\n\n");
    
    while (1) {
        printf("Nhập văn bản cần kiểm tra: ");
        if (fgets(input_text, sizeof(input_text), stdin) == NULL) {
            break;
        }
        
        // Remove newline character
        input_text[strcspn(input_text, "\n")] = 0;
        
        // Check for quit command
        if (strcmp(input_text, "quit") == 0) {
            printf("Tạm biệt!\n");
            break;
        }
        
        // Check for file command
        if (strcmp(input_text, "file") == 0) {
            FILE* f = fopen("spell_check_input.txt", "r");
            if (!f) {
                printf("Không thể mở file 'spell_check_input.txt'!\n\n");
                continue;
            }
            char file_buffer[10000]; // Adjust size as needed for your largest file
            size_t total_len = 0;
            file_buffer[0] = '\0';
            char line[1024];
            while (fgets(line, sizeof(line), f)) {
                size_t line_len = strlen(line);
                if (total_len + line_len < sizeof(file_buffer) - 1) {
                    strcpy(file_buffer + total_len, line);
                    total_len += line_len;
                } else {
                    // Truncate if file is too large
                    break;
                }
            }
            fclose(f);
            file_buffer[total_len] = '\0';
            checkSpelling(root, file_buffer);
            continue;
        }
        
        // Skip empty input
        if (strlen(input_text) == 0) {
            continue;
        }
        
        // Check spelling
        checkSpelling(root, input_text);
    }
    
    // Cleanup
    freeTrie(root);
    return 0;
}