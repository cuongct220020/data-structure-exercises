#include <iostream>
#include <stdlib.h>
#include <string.h> // For strcpy
#include <unordered_map> // For std::unordered_map

#define MAX_LEN 256

// Định nghĩa Node của danh sách liên kết đôi
typedef struct Node {
    int key;
    char value[MAX_LEN];
    struct Node* prev;
    struct Node* next;
    int frequency; // Trường này hữu ích cho MFU/LFU, nhưng không trực tiếp dùng cho LRU/MRU trong cài đặt cơ bản này
} Node;

// Định nghĩa cấu trúc LRU Cache
typedef struct LRUCache {
    int capacity;
    int size; // Current number of items in cache
    std::unordered_map<int, Node*> cacheMap; // Hash map for O(1) lookup
    Node *head; // Head of the doubly linked list (MRU)
    Node *tail; // Tail of the doubly linked list (LRU)
} LRUCache;

// ---

enum CacheStrategy {
    LRU, // Least Recently Used
    MRU  // Most Recently Used (for demonstration with current structure)
    // MFU // Most Frequently Used (requires more complex structure)
};

// --- Helper Functions for Doubly Linked List ---

// Function to add a node to the head (MRU position)
void addNodeToHead(LRUCache* cache, Node* node) {
    node->next = cache->head;
    node->prev = nullptr;
    if (cache->head != nullptr) {
        cache->head->prev = node;
    }
    cache->head = node;
    if (cache->tail == nullptr) { // If list was empty
        cache->tail = node;
    }
}

// Function to remove a node from the list
void removeNode(LRUCache* cache, Node* node) {
    if (node->prev != nullptr) {
        node->prev->next = node->next;
    } else { // Node is head
        cache->head = node->next;
    }

    if (node->next != nullptr) {
        node->next->prev = node->prev;
    } else { // Node is tail
        cache->tail = node->prev;
    }
    // Clean up pointers of the removed node
    node->prev = nullptr;
    node->next = nullptr;
}

// Function to move a node to the head (making it MRU)
void moveNodeToHead(LRUCache* cache, Node* node) {
    removeNode(cache, node);
    addNodeToHead(cache, node);
}

// --- LRU Cache Functions ---

// Hàm khởi tạo LRU Cache
LRUCache* initCache(int capacity) {
    if (capacity <= 0) {
        std::cerr << "Cache capacity must be positive." << std::endl;
        return nullptr;
    }
    LRUCache* cache = (LRUCache*)malloc(sizeof(LRUCache));
    if (cache == nullptr) {
        std::cerr << "Memory allocation failed for LRUCache." << std::endl;
        return nullptr;
    }
    cache->capacity = capacity;
    cache->size = 0;
    cache->head = nullptr;
    cache->tail = nullptr;
    // cacheMap is automatically initialized by std::unordered_map constructor
    return cache;
}

// Hàm tìm kiếm 1 khoá và nếu khoá đó tồn tại, phần tử tương ứng sẽ được di chuyển về đầu (MRU)
// Trả về giá trị của phần tử nếu tìm thấy, ngược lại trả về nullptr
const char* getCache(LRUCache* cache, int key, CacheStrategy strategy_type) {
    if (cache == nullptr) {
        return nullptr;
    }

    auto it = cache->cacheMap.find(key);
    if (it == cache->cacheMap.end()) {
        // Key not found in cache
        return nullptr;
    }

    Node* node = it->second;

    // For LRU/MRU, the node that was just accessed becomes the most recently used.
    // So, it's moved to the head of the list.
    moveNodeToHead(cache, node);

    // If using MFU, you'd increment node->frequency here.
    // node->frequency++;

    return node->value;
}

// Hàm thêm/cập nhật một phần tử vào cache
void putCache(LRUCache* cache, int key, const char* value, CacheStrategy strategy_type) {
    if (cache == nullptr) {
        return;
    }

    auto it = cache->cacheMap.find(key);

    if (it != cache->cacheMap.end()) {
        // Key already exists (Cache Hit)
        Node* node = it->second;
        // Update the value
        strncpy(node->value, value, MAX_LEN - 1);
        node->value[MAX_LEN - 1] = '\0'; // Ensure null termination
        
        // Move the node to the head (MRU position)
        moveNodeToHead(cache, node);

        // If using MFU, you'd increment node->frequency here.
        // node->frequency++;

    } else {
        // Key does not exist (Cache Miss)
        Node* newNode = (Node*)malloc(sizeof(Node));
        if (newNode == nullptr) {
            std::cerr << "Memory allocation failed for new Node." << std::endl;
            return;
        }
        newNode->key = key;
        strncpy(newNode->value, value, MAX_LEN - 1);
        newNode->value[MAX_LEN - 1] = '\0'; // Ensure null termination
        newNode->frequency = 1; // Initialize frequency for new node

        // If cache is full, evict based on strategy
        if (cache->size == cache->capacity) {
            Node* nodeToEvict = nullptr;
            
            switch (strategy_type) {
                case LRU:
                    // Evict the Least Recently Used (tail of the list)
                    nodeToEvict = cache->tail;
                    break;
                case MRU:
                    // Evict the Most Recently Used (head of the list).
                    // This is unusual for a cache, but possible to implement.
                    nodeToEvict = cache->head;
                    break;
                // case MFU:
                //     // For MFU, you would need to iterate to find the highest frequency,
                //     // or use a more complex data structure (e.g., min-heap of frequencies)
                //     // to find the MFU element efficiently.
                //     // For simplicity in this structure, finding MFU would be O(N).
                //     // Example (O(N) for MFU):
                //     // Node* current = cache->head;
                //     // int max_freq = -1;
                //     // while (current != nullptr) {
                //     //     if (current->frequency > max_freq) {
                //     //         max_freq = current->frequency;
                //     //         nodeToEvict = current;
                //     //     }
                //     //     current = current->next;
                //     // }
                //     // break;
                default:
                    std::cerr << "Unsupported cache strategy." << std::endl;
                    free(newNode); // Don't leak memory
                    return;
            }

            if (nodeToEvict != nullptr) {
                // Remove from hash map
                cache->cacheMap.erase(nodeToEvict->key);
                // Remove from doubly linked list
                removeNode(cache, nodeToEvict);
                // Free memory of the evicted node
                free(nodeToEvict);
                cache->size--;
            }
        }

        // Add the new node to the head (MRU position)
        addNodeToHead(cache, newNode);
        // Add to hash map
        cache->cacheMap[key] = newNode;
        cache->size++;
    }
}

// --- Example Usage (main function for testing) ---
int main() {
    // Test LRU Cache
    std::cout << "--- Testing LRU Cache ---" << std::endl;
    LRUCache* lru_cache = initCache(3); // Capacity of 3

    putCache(lru_cache, 1, "value1", LRU); // Cache: (1)
    putCache(lru_cache, 2, "value2", LRU); // Cache: (2, 1)
    putCache(lru_cache, 3, "value3", LRU); // Cache: (3, 2, 1)

    std::cout << "Get 2: " << (getCache(lru_cache, 2, LRU) ? getCache(lru_cache, 2, LRU) : "Not found") << std::endl; // Cache: (2, 3, 1)
    
    putCache(lru_cache, 4, "value4", LRU); // Cache is full, 1 (LRU) is evicted. Cache: (4, 2, 3)
    std::cout << "Get 1: " << (getCache(lru_cache, 1, LRU) ? getCache(lru_cache, 1, LRU) : "Not found") << std::endl; // Not found

    // Test MRU Cache (conceptual, as it's often not a good cache strategy)
    std::cout << "\n--- Testing MRU Cache ---" << std::endl;
    LRUCache* mru_cache = initCache(3);

    putCache(mru_cache, 10, "value10", MRU); // Cache: (10)
    putCache(mru_cache, 11, "value11", MRU); // Cache: (11, 10)
    putCache(mru_cache, 12, "value12", MRU); // Cache: (12, 11, 10)

    std::cout << "Get 11: " << (getCache(mru_cache, 11, MRU) ? getCache(mru_cache, 11, MRU) : "Not found") << std::endl; // Cache: (11, 12, 10)

    putCache(mru_cache, 13, "value13", MRU); // Cache is full, 11 (MRU) is evicted. Cache: (13, 12, 10)
    std::cout << "Get 11: " << (getCache(mru_cache, 11, MRU) ? getCache(mru_cache, 11, MRU) : "Not found") << std::endl; // Not found

    // Remember to free allocated memory
    // (This part would require iterating through cacheMap and freeing Node* and then the cache itself)
    // For simplicity, not included in main example, but crucial for real applications.
    return 0;
}