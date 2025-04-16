#include <gtest/gtest.h>
#include "memory_allocator.h"

TEST(MemoryAllocatorTest, AllocateSmallSize) {
    // Initialize the allocator.
    EXPECT_TRUE(MemoryAllocator::initialize());
    void* ptr = MemoryAllocator::allocate(100);
    ASSERT_NE(ptr, nullptr);
    MemoryAllocator::deallocate(ptr, 100);
}

TEST(MemoryAllocatorTest, AllocateLargeSize) {
    EXPECT_TRUE(MemoryAllocator::initialize());
    void* ptr = MemoryAllocator::allocate(1024);
    ASSERT_NE(ptr, nullptr);
    MemoryAllocator::deallocate(ptr, 1024);
}
