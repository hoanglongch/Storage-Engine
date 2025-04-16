#include "memory_allocator.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <thread>

namespace MemoryAllocator {
    // Fixed block size for pool allocations.
    static const std::size_t BLOCK_SIZE = 256;

    // Thread-local free list for block allocations.
    thread_local std::vector<void*> freeList;

    bool initialize() {
        std::cout << "[MemoryAllocator] Custom memory allocator initialized with thread-local pools." << std::endl;
        return true;
    }

    void* allocate(std::size_t size) {
        // For simplicity, if the requested size exceeds BLOCK_SIZE, delegate to malloc.
        if (size > BLOCK_SIZE) {
            return std::malloc(size);
        }
        
        if (!freeList.empty()) {
            void* block = freeList.back();
            freeList.pop_back();
            return block;
        }
        // Allocate a new block if the free list is empty.
        return std::malloc(BLOCK_SIZE);
    }

    void deallocate(void* ptr, std::size_t size) {
        if (!ptr) return;
        if (size > BLOCK_SIZE) {
            std::free(ptr);
        } else {
            freeList.push_back(ptr);
        }
    }
} // namespace MemoryAllocator
