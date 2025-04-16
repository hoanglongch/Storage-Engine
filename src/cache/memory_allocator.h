#ifndef MEMORY_ALLOCATOR_H
#define MEMORY_ALLOCATOR_H

#include <cstddef>

namespace MemoryAllocator {
    // Initialize the custom memory allocator with thread-local memory pools.
    bool initialize();

    // Allocate memory of given size.
    void* allocate(std::size_t size);

    // Deallocate memory block.
    void deallocate(void* ptr, std::size_t size);
}

#endif // MEMORY_ALLOCATOR_H
