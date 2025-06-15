#include <iostream>
#include <stdlib.h>

#define MAX_LEN 256

typedef struct Node {
    int key;
    char value[MAX_LEN];
    struct Node* prev;
    struct Node* next;
} Node;

typedef struct LRUCache {
    int capacity;
    int size;
    std::unordered_map<int, Node*> cacheMap;
    Node *head;
    Node *tail;
} LRUCache;

// Hàm khởi tạo LRU Cache
LRUCache* initCache(int capacity);

// Hàm tìm kiếm 1 khoá và khoá đó tồn tại, phần tử tương ứng sẽ được di chuyển về đầu
void getCache(int key);

void putCache(int key);

