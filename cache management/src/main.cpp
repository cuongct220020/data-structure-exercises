#include <iostream>
#include <memory>
#include <stdexcept>
#include "cache.hpp"

int main() {
    try {
        // Test LRU Cache
        std::cout << "--- Testing LRU Cache ---" << std::endl;
        auto lru_cache = std::make_unique<Cache>(3); // Capacity of 3

        lru_cache->put(1, "value1", CacheStrategy::LRU); // Cache: (1)
        lru_cache->put(2, "value2", CacheStrategy::LRU); // Cache: (2, 1)
        lru_cache->put(3, "value3", CacheStrategy::LRU); // Cache: (3, 2, 1)
        //lru_cache->printCache();

        std::string result = lru_cache->get(2);
        std::cout << "Get 2: " << (result.empty() ? "Not found" : result) << std::endl; // Cache: (2, 3, 1)
        //lru_cache->printCache();
        
        lru_cache->put(4, "value4", CacheStrategy::LRU); // Cache is full, 1 (LRU) is evicted. Cache: (4, 2, 3)
        //lru_cache->printCache();
        
        result = lru_cache->get(1);
        std::cout << "Get 1: " << (result.empty() ? "Not found" : result) << std::endl; // Not found
        lru_cache->printCache();

        // Test MRU Cache
        std::cout << "\n--- Testing MRU Cache ---" << std::endl;
        auto mru_cache = std::make_unique<Cache>(3);

        mru_cache->put(10, "value10", CacheStrategy::MRU); // Cache: (10)
        mru_cache->put(11, "value11", CacheStrategy::MRU); // Cache: (11, 10)
        mru_cache->put(12, "value12", CacheStrategy::MRU); // Cache: (12, 11, 10)

        result = mru_cache->get(11);
        std::cout << "Get 11: " << (result.empty() ? "Not found" : result) << std::endl; // Cache: (11, 12, 10)

        mru_cache->put(13, "value13", CacheStrategy::MRU); // Cache is full, 11 (MRU) is evicted. Cache: (13, 12, 10)
        result = mru_cache->get(11);
        std::cout << "Get 11: " << (result.empty() ? "Not found" : result) << std::endl; // Not found

        mru_cache->printCache();

        // Test MFU Cache
        std::cout << "\n--- Testing MFU Cache ---" << std::endl;
        auto mfu_cache = std::make_unique<Cache>(3);

        mfu_cache->put(20, "value20", CacheStrategy::MFU);
        mfu_cache->put(21, "value21", CacheStrategy::MFU);
        mfu_cache->put(22, "value22", CacheStrategy::MFU);

        // Access some items multiple times to increase frequency
        mfu_cache->get(20);
        mfu_cache->get(20);
        mfu_cache->get(21);
        mfu_cache->get(22);
        mfu_cache->get(22);
        mfu_cache->get(22);

        std::cout << "Before eviction:" << std::endl;
        mfu_cache->printCache();

        // This should evict the least frequently used item
        mfu_cache->put(23, "value23", CacheStrategy::MFU);

        std::cout << "After eviction:" << std::endl;
        mfu_cache->printCache();

        // Test error handling
        std::cout << "\n--- Testing Error Handling ---" << std::endl;
        
        try {
            auto invalid_cache = std::make_unique<Cache>(-1); // Should throw
        } catch (const std::invalid_argument& e) {
            std::cout << "Caught expected error: " << e.what() << std::endl;
        }

        try {
            std::string long_value(300, 'x'); // Exceeds MAX_LEN
            lru_cache->put(100, long_value, CacheStrategy::LRU);
        } catch (const std::invalid_argument& e) {
            std::cout << "Caught expected error: " << e.what() << std::endl;
        }

        std::cout << "\n--- All tests completed successfully ---" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}