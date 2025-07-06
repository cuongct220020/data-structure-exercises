#include <iostream>
#include <memory>
#include <string>
#include <chrono>
#include <atomic>
#include <stdexcept>
#include "cache.hpp"

// Static member initialization
std::atomic<long long> Cache::globalTimestamp_{0};

// Helper Functions for Doubly Linked List

void Cache::addNodeToHead(std::shared_ptr<Node> node) {
    if (!node) return;
    
    node->next = head_;
    node->prev = nullptr;
    
    if (head_) {
        head_->prev = node;
    }
    
    head_ = node;
    
    if (!tail_) {
        tail_ = node;
    }
}

void Cache::removeNode(std::shared_ptr<Node> node) {
    if (!node) return;
    
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        // Node is head
        head_ = node->next;
    }
    
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        // Node is tail
        tail_ = node->prev;
    }
    
    // Clear pointers of the removed node
    node->prev = nullptr;
    node->next = nullptr;
}

void Cache::moveNodeToHead(std::shared_ptr<Node> node) {
    if (!node) return;
    
    removeNode(node);
    addNodeToHead(node);
}

// Cache Constructor
Cache::Cache(int capacity) : capacity_(capacity), size_(0), head_(nullptr), tail_(nullptr) {
    if (capacity <= 0) {
        throw std::invalid_argument("Cache capacity must be positive");
    }
}

// Get operation
std::string Cache::get(int key) {
    auto it = cacheMap_.find(key);
    if (it == cacheMap_.end()) {
        return ""; // Key not found
    }
    
    auto node = it->second;
    if (!node) {
        cacheMap_.erase(it);
        return ""; // Invalid node reference
    }
    
    // Update frequency and timestamp
    node->frequency++;
    node->timestamp = std::chrono::steady_clock::now();
    
    // Move node to MRU position
    moveNodeToHead(node);
    
    // Update priority queue
    pq_.push(node);
    
    return node->value;
}

// Get MFU victim for eviction
std::shared_ptr<Node> Cache::getMFUVictim() {
    while (!pq_.empty()) {
        auto top = pq_.top();
        pq_.pop();
        
        // Check if this node is still valid in the cache
        auto it = cacheMap_.find(top->key);
        if (it != cacheMap_.end() && it->second == top) {
            return top;
        }
        // Node is no longer valid, continue to next
    }
    return nullptr;
}

// Put operation
void Cache::put(int key, const std::string& value, CacheStrategy strategy) {
    if (value.length() > MAX_LEN) {
        throw std::invalid_argument("Value length exceeds maximum allowed size");
    }
    
    // Check if key already exists
    auto it = cacheMap_.find(key);
    
    if (it != cacheMap_.end()) {
        // Key exists - update value and move to MRU
        auto node = it->second;
        if (!node) {
            cacheMap_.erase(it);
            return;
        }
        
        node->value = value;
        node->frequency++;
        node->timestamp = std::chrono::steady_clock::now();
        
        moveNodeToHead(node);
        pq_.push(node);
    } else {
        // Key doesn't exist - need to add new node
        if (size_ == capacity_) {
            // Cache is full, need to evict
            std::shared_ptr<Node> nodeToEvict = nullptr;
            
            switch (strategy) {
                case CacheStrategy::LRU:
                    nodeToEvict = tail_;
                    break;
                case CacheStrategy::MRU:
                    nodeToEvict = head_;
                    break;
                case CacheStrategy::MFU:
                    nodeToEvict = getMFUVictim();
                    break;
                default:
                    throw std::invalid_argument("Unsupported cache strategy");
            }
            
            if (nodeToEvict) {
                cacheMap_.erase(nodeToEvict->key);
                removeNode(nodeToEvict);
                size_--;
            }
        }
        
        // Create new node
        auto newNode = std::make_shared<Node>(key, value);
        newNode->timestamp = std::chrono::steady_clock::now();
        
        addNodeToHead(newNode);
        cacheMap_[key] = newNode;
        pq_.push(newNode);
        size_++;
    }
}

// Clear cache
void Cache::clear() {
    cacheMap_.clear();
    while (!pq_.empty()) {
        pq_.pop();
    }
    head_ = nullptr;
    tail_ = nullptr;
    size_ = 0;
}

// Print cache state for debugging
void Cache::printCache() const {
    std::cout << "Cache State (size=" << size_ << "/" << capacity_ << "): ";
    auto curr = head_;
    while (curr) {
        std::cout << "(key=" << curr->key 
                  << ", value=" << curr->value 
                  << ", freq=" << curr->frequency << ") ";
        curr = curr->next;
    }
    std::cout << std::endl;
}

// // Legacy C-style interface implementation
// extern "C" {
//     struct LRUCache {
//         std::unique_ptr<Cache> cache;
//         LRUCache(std::unique_ptr<Cache> c) : cache(std::move(c)) {}
//     };
    
//     LRUCache* initCache(int capacity) {
//         try {
//             auto cache = std::make_unique<Cache>(capacity);
//             return new LRUCache(std::move(cache));
//         } catch (const std::exception& e) {
//             std::cerr << "Failed to initialize cache: " << e.what() << std::endl;
//             return nullptr;
//         }
//     }
    
//     const char* getCache(LRUCache* cache, int key) {
//         if (!cache || !cache->cache) {
//             return nullptr;
//         }
        
//         try {
//             std::string result = cache->cache->get(key);
//             if (result.empty()) {
//                 return nullptr;
//             }
            
//             // Note: This is a temporary solution. In a real application,
//             // you'd want to manage the returned string properly
//             static std::string temp;
//             temp = result;
//             return temp.c_str();
//         } catch (const std::exception& e) {
//             std::cerr << "Error in getCache: " << e.what() << std::endl;
//             return nullptr;
//         }
//     }
    
//     void putCache(LRUCache* cache, int key, const char* value, CacheStrategy strategy_type) {
//         if (!cache || !cache->cache || !value) {
//             return;
//         }
        
//         try {
//             cache->cache->put(key, std::string(value), strategy_type);
//         } catch (const std::exception& e) {
//             std::cerr << "Error in putCache: " << e.what() << std::endl;
//         }
//     }
    
//     void cleanCache(LRUCache* cache) {
//         if (cache) {
//             delete cache;
//         }
//     }
// }