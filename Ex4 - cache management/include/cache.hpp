#ifndef CACHE_HPP
#define CACHE_HPP

#include <unordered_map>
#include <vector>
#include <queue>
#include <memory>
#include <string>
#include <chrono>
#include <atomic>

constexpr size_t MAX_LEN = 256;

enum class CacheStrategy {
    LRU, // Least Recently Used
    MRU, // Most Recently Used
    MFU  // Most Frequently Used
};

// Forward declaration
class Cache;

// Node of doubly linked list
class Node {
public:
    int key;
    std::string value;
    std::shared_ptr<Node> prev;
    std::shared_ptr<Node> next;
    int frequency;
    std::chrono::steady_clock::time_point timestamp;

    Node(int k, const std::string& val) 
        : key(k), value(val), prev(nullptr), next(nullptr), 
          frequency(1), timestamp(std::chrono::steady_clock::now()) {}
    
    ~Node() = default;
};

// Main Cache class
class Cache {
private:
    int capacity_;
    int size_;
    std::unordered_map<int, std::shared_ptr<Node>> cacheMap_;
    std::shared_ptr<Node> head_;
    std::shared_ptr<Node> tail_;
    static std::atomic<long long> globalTimestamp_;

    // Helper functions for doubly linked list
    void addNodeToHead(std::shared_ptr<Node> node);
    void removeNode(std::shared_ptr<Node> node);
    void moveNodeToHead(std::shared_ptr<Node> node);
    std::shared_ptr<Node> getMFUVictim();

public:
    explicit Cache(int capacity);
    ~Cache() = default;
    
    // Disable copy constructor and assignment operator
    Cache(const Cache&) = delete;
    Cache& operator=(const Cache&) = delete;
    
    // Allow move constructor and assignment operator
    Cache(Cache&&) = default;
    Cache& operator=(Cache&&) = default;

    // Main cache operations
    std::string get(int key);
    void put(int key, const std::string& value, CacheStrategy strategy);
    void clear();
    
    // Utility functions
    int getSize() const { return size_; }
    int getCapacity() const { return capacity_; }
    bool isEmpty() const { return size_ == 0; }
    bool isFull() const { return size_ == capacity_; }
    
    // Debug function
    void printCache() const;
};

// // Legacy C-style interface for backward compatibility
// extern "C" {
//     struct LRUCache;
//     LRUCache* initCache(int capacity);
//     const char* getCache(LRUCache* cache, int key);
//     void putCache(LRUCache* cache, int key, const char* value, CacheStrategy strategy_type);
//     void cleanCache(LRUCache* cache);
// }

#endif // CACHE_HPP