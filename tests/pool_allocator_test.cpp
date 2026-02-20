#include "pool_allocator.h"
#include "log_entry.h"

#include <gtest/gtest.h>
#include <vector>

using namespace LogAnalyzer;

TEST(PoolAllocatorTest, BasicAllocateDeallocate) {
    PoolAllocator<int> alloc(16);
    int* p = alloc.allocate(1);
    ASSERT_NE(p, nullptr);
    *p = 42;
    EXPECT_EQ(*p, 42);
    alloc.deallocate(p, 1); // no-op, should not crash
}

TEST(PoolAllocatorTest, MultipleAllocations) {
    PoolAllocator<int> alloc(64);
    int* a = alloc.allocate(1);
    int* b = alloc.allocate(1);
    int* c = alloc.allocate(1);
    EXPECT_NE(a, b);
    EXPECT_NE(b, c);
    *a = 1;
    *b = 2;
    *c = 3;
    EXPECT_EQ(*a, 1);
    EXPECT_EQ(*b, 2);
    EXPECT_EQ(*c, 3);
}

TEST(PoolAllocatorTest, PoolExhaustion) {
    PoolAllocator<int> alloc(2); // room for 2 ints
    (void)alloc.allocate(2);
    EXPECT_THROW((void)alloc.allocate(1), std::bad_alloc);
}

TEST(PoolAllocatorTest, VectorWithLogEntry) {
    PoolAllocator<LogEntry> alloc(128);
    std::vector<LogEntry, PoolAllocator<LogEntry>> entries(alloc);

    entries.push_back(LogEntry{
        std::chrono::system_clock::now(),
        LogLevel::Info,
        "test message"
    });
    entries.push_back(LogEntry{
        std::chrono::system_clock::now(),
        LogLevel::Error,
        "error message",
        "network"
    });

    EXPECT_EQ(entries.size(), 2u);
    EXPECT_EQ(entries[0].level, LogLevel::Info);
    EXPECT_EQ(entries[1].module.value(), "network");
}

TEST(PoolAllocatorTest, EqualitySemantics) {
    PoolAllocator<int> a(32);
    PoolAllocator<int> b(a); // copy shares pool
    PoolAllocator<int> c(32); // separate pool

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
}

TEST(PoolAllocatorTest, RebindSharesPool) {
    PoolAllocator<int> intAlloc(64);
    PoolAllocator<double> doubleAlloc(intAlloc); // converting constructor

    EXPECT_TRUE(intAlloc == doubleAlloc);
}
