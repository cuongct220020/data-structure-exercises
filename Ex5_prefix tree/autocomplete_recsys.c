#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//Link tải file words_alpha.txt: https://github.com/dwyl/english-words/blob/master/words_alpha.txt

#define ALPHABET_SIZE 26
#define MAX_WORD_LENGTH 100
#define MAX_SUGGESTIONS 20

// Prefix Tree (Trie) node structure
typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    bool isEndOfWord;
    int frequency; // For prioritizing suggestions
} TrieNode;

// Function to create a new Trie node
TrieNode* createNode() {
    TrieNode* node = (TrieNode*)malloc(sizeof(TrieNode));
    if (node == NULL) {
        printf("Dynamic Allocation Error.\n");
        return NULL;
    }

    node->isEndOfWord = false;
    node->frequency = 0;
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

// Insert a word into the Trie
void insert(TrieNode* root, const char* word) {
    TrieNode* current = root;
    int length = strlen(word);
    
    for (int i = 0; i < length; i++) {
        int index = charToIndex(word[i]);
        if (index == -1) continue; // Skip invalid characters
        
        if (!current->children[index]) {
            current->children[index] = createNode();
        }
        current = current->children[index];
    }
    
    current->isEndOfWord = true;
    current->frequency++; // Increase frequency for better suggestions
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

// Find the node that represents the prefix
TrieNode* findPrefixNode(TrieNode* root, const char* prefix) {
    TrieNode* current = root;
    int length = strlen(prefix);
    
    for (int i = 0; i < length; i++) {
        int index = charToIndex(prefix[i]);
        if (index == -1 || !current->children[index]) {
            return NULL;
        }
        current = current->children[index];
    }
    
    return current;
}

// Recursive function to collect all words with given prefix
void collectWords(TrieNode* node, const char* prefix, char* word, int depth, 
                  char suggestions[][MAX_WORD_LENGTH], int* count) {
    if (*count >= MAX_SUGGESTIONS) return;
    
    if (node->isEndOfWord) {
        word[depth] = '\0';
        sprintf(suggestions[*count], "%s%s", prefix, word);
        (*count)++;
    }
    
    for (int i = 0; i < ALPHABET_SIZE && *count < MAX_SUGGESTIONS; i++) {
        if (node->children[i]) {
            word[depth] = 'a' + i;
            collectWords(node->children[i], prefix, word, depth + 1, suggestions, count);
        }
    }
}

// Get autocomplete suggestions for a prefix
int getAutocompleteSuggestions(TrieNode* root, const char* prefix, 
                               char suggestions[][MAX_WORD_LENGTH]) {
    TrieNode* prefixNode = findPrefixNode(root, prefix);
    int count = 0;
    
    if (!prefixNode) {
        return 0; // No words with this prefix
    }
    
    char word[MAX_WORD_LENGTH];
    collectWords(prefixNode, prefix, word, 0, suggestions, &count);
    
    return count;
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

// Hiển thị gợi ý
void displaySuggestions(const char* prefix, char suggestions[][MAX_WORD_LENGTH], int count) {
    if (count == 0) {
        printf("Không tìm thấy từ nào với tiền tố '%s'\n", prefix);
        return;
    }
    
    printf("Gợi ý cho tiền tố '%s':\n", prefix);
    for (int i = 0; i < count; i++) {
        printf("  %d. %s\n", i + 1, suggestions[i]);
    }
    printf("\n");
}

// --- Hàm chính cần bạn hoàn thiện ---
void loadWordsFromFile(TrieNode* root, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Không thể mở file words.txt");
        return;
    }

    char buffer[256]; // Buffer tạm thời để đọc từng dòng từ file
    int wordCount = 0;

    printf("Đang tải từ từ file '%s' vào hệ thống...\n", filename);

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        // Loại bỏ ký tự xuống dòng ('\n') nếu có
        buffer[strcspn(buffer, "\n")] = 0;

        // Chỉ chèn từ nếu nó không rỗng sau khi loại bỏ ký tự xuống dòng
        if (strlen(buffer) > 0) {
            // Chèn từ vào cây Trie
            insert(root, buffer);
            wordCount++;
        }
    }

    fclose(file); // Đóng file
    printf("Hoàn thành việc tải %d từ từ file '%s'!\n\n", wordCount, filename);
}

int main() {
    TrieNode* root = createNode();
    if (root == NULL) return 0;
    char prefix[MAX_WORD_LENGTH];
    char suggestions[MAX_SUGGESTIONS][MAX_WORD_LENGTH];
    
    printf("=== HỆ THỐNG GỢI Ý HOÀN THIỆN TỪ (AUTOCOMPLETE) ===\n");
    printf("Sử dụng cấu trúc dữ liệu Prefix Tree (Trie)\n\n");
    
    // Load sample words
    loadWordsFromFile(root, "data/autocomplete_recsys/words_alpha.txt");
    
    printf("Hướng dẫn sử dụng:\n");
    printf("- Nhập tiền tố để tìm gợi ý (ít nhất 1 ký tự)\n");
    printf("- Nhập 'quit' để thoát chương trình\n");
    printf("- Nhập 'add <word>' để thêm từ mới\n\n");
    
    while (1) {
        printf("Nhập tiền tố: ");
        if (fgets(prefix, sizeof(prefix), stdin) == NULL) {
            break;
        }
        
        // Remove newline character
        prefix[strcspn(prefix, "\n")] = 0;
        
        // Check for quit command
        if (strcmp(prefix, "quit") == 0) {
            printf("Tạm biệt!\n");
            break;
        }
        
        // Check for add command
        if (strncmp(prefix, "add ", 4) == 0) {
            char* word = prefix + 4;
            if (strlen(word) > 0) {
                insert(root, word);
                printf("Đã thêm từ '%s' vào từ điển.\n\n", word);
            } else {
                printf("Vui lòng nhập từ cần thêm sau 'add '.\n\n");
            }
            continue;
        }
        
        // Skip empty input
        if (strlen(prefix) == 0) {
            continue;
        }
        
        // Get and display suggestions
        int count = getAutocompleteSuggestions(root, prefix, suggestions);
        displaySuggestions(prefix, suggestions, count);
        
        // Show search result
        if (search(root, prefix)) {
            printf("'%s' là một từ hoàn chỉnh trong từ điển.\n\n", prefix);
        }
    }
    
    // Cleanup
    freeTrie(root);
    return 0;
}