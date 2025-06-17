#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define ALPHABET_SIZE 26
#define MAX_WORD_LENGTH 100
#define MAX_SUGGESTIONS 10
#define MAX_LINE_LENGTH 1000

// Prefix Tree (Trie) node structure
typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    bool isEndOfWord;
} TrieNode;

// Function to create a new Trie node
TrieNode* createNode() {
    TrieNode* node = (TrieNode*)malloc(sizeof(TrieNode));
    if (node) {
        node->isEndOfWord = false;
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            node->children[i] = NULL;
        }
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

// Calculate edit distance (Levenshtein distance) for suggestions
int min(int a, int b, int c) {
    int min_val = a;
    if (b < min_val) min_val = b;
    if (c < min_val) min_val = c;
    return min_val;
}

int editDistance(const char* str1, const char* str2) {
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    
    // Create a 2D array for dynamic programming
    int dp[len1 + 1][len2 + 1];
    
    // Initialize base cases
    for (int i = 0; i <= len1; i++) {
        dp[i][0] = i;
    }
    for (int j = 0; j <= len2; j++) {
        dp[0][j] = j;
    }
    
    // Fill the dp table
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            if (str1[i-1] == str2[j-1]) {
                dp[i][j] = dp[i-1][j-1];
            } else {
                dp[i][j] = 1 + min(dp[i-1][j], dp[i][j-1], dp[i-1][j-1]);
            }
        }
    }
    
    return dp[len1][len2];
}

// Collect words from trie for suggestion generation
void collectAllWords(TrieNode* node, char* current_word, int depth, 
                     char words[][MAX_WORD_LENGTH], int* count, int max_words) {
    if (*count >= max_words) return;
    
    if (node->isEndOfWord) {
        current_word[depth] = '\0';
        strcpy(words[*count], current_word);
        (*count)++;
    }
    
    for (int i = 0; i < ALPHABET_SIZE && *count < max_words; i++) {
        if (node->children[i]) {
            current_word[depth] = 'a' + i;
            collectAllWords(node->children[i], current_word, depth + 1, words, count, max_words);
        }
    }
}

// Generate spelling suggestions for a misspelled word
int generateSuggestions(TrieNode* root, const char* misspelled_word, 
                       char suggestions[][MAX_WORD_LENGTH]) {
    const int MAX_DICT_WORDS = 1000; // Limit for performance
    char dictionary_words[MAX_DICT_WORDS][MAX_WORD_LENGTH];
    int dict_count = 0;
    
    // Collect words from dictionary (limited for performance)
    char temp_word[MAX_WORD_LENGTH];
    collectAllWords(root, temp_word, 0, dictionary_words, &dict_count, MAX_DICT_WORDS);
    
    // Find words with small edit distance
    int suggestion_count = 0;
    int word_len = strlen(misspelled_word);
    
    for (int i = 0; i < dict_count && suggestion_count < MAX_SUGGESTIONS; i++) {
        int distance = editDistance(misspelled_word, dictionary_words[i]);
        int dict_word_len = strlen(dictionary_words[i]);
        
        // Accept words with edit distance <= 2 and similar length
        if (distance <= 2 && abs(word_len - dict_word_len) <= 2) {
            strcpy(suggestions[suggestion_count], dictionary_words[i]);
            suggestion_count++;
        }
    }
    
    return suggestion_count;
}

// Load words from file into Trie
int loadDictionaryFromFile(TrieNode* root, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Lỗi: Không thể mở file '%s'\n", filename);
        printf("Hãy đảm bảo file words.txt có trong thư mục hiện tại.\n");
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
    char words[100][MAX_WORD_LENGTH];
    int word_count = extractWords(text, words);
    
    printf("=== KẾT QUẢ KIỂM TRA CHÍNH TẢ ===\n");
    printf("Văn bản gốc: %s\n\n", text);
    
    bool has_errors = false;
    
    for (int i = 0; i < word_count; i++) {
        if (!search(root, words[i])) {
            has_errors = true;
            printf("❌ Từ SAI: '%s'\n", words[i]);
            
            char suggestions[MAX_SUGGESTIONS][MAX_WORD_LENGTH];
            int suggestion_count = generateSuggestions(root, words[i], suggestions);
            
            if (suggestion_count > 0) {
                printf("   Gợi ý sửa: ");
                for (int j = 0; j < suggestion_count; j++) {
                    printf("'%s'", suggestions[j]);
                    if (j < suggestion_count - 1) printf(", ");
                }
                printf("\n");
            } else {
                printf("   Không tìm thấy gợi ý phù hợp.\n");
            }
            printf("\n");
        } else {
            printf("✅ Từ ĐÚNG: '%s'\n", words[i]);
        }
    }
    
    if (!has_errors) {
        printf("🎉 Tất cả các từ đều được viết đúng chính tả!\n");
    }
    
    printf("\n" "========================\n\n");
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
        "which", "their", "time", "will", "about", "make"
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
    int loaded = loadDictionaryFromFile(root, "words.txt");
    
    // If file loading failed, use sample dictionary
    if (loaded == 0) {
        printf("Sử dụng từ điển mẫu thay thế...\n\n");
        loadSampleDictionary(root);
    }
    
    printf("Hướng dẫn sử dụng:\n");
    printf("- Nhập văn bản để kiểm tra chính tả\n");
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