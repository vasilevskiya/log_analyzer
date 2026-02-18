#include <gtest/gtest.h>
#include "log_cache.h"

#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

using namespace LogAnalyzer;

class LogCacheTest : public ::testing::Test {
protected:
    std::filesystem::path tmpFile_;

    void SetUp() override {
        tmpFile_ = std::filesystem::temp_directory_path() / "log_cache_test.log";
        writeLogFile("[2024-01-15 10:30:45] INFO [Network] Connected\n"
                     "[2024-01-15 10:31:00] ERROR Disk full\n");
    }

    void TearDown() override {
        std::filesystem::remove(tmpFile_);
    }

    void writeLogFile(const std::string& content) {
        std::ofstream out(tmpFile_);
        out << content;
    }
};

TEST_F(LogCacheTest, CacheMissTriggersParsingAndReturnsEntries) {
    LogCache cache;
    EXPECT_EQ(cache.size(), 0u);

    auto entries = cache.getEntries(tmpFile_);

    EXPECT_EQ(cache.size(), 1u);
    ASSERT_EQ(entries.size(), 2u);
    EXPECT_EQ(entries[0].level, LogLevel::Info);
    EXPECT_EQ(entries[0].message, "Connected");
    EXPECT_EQ(entries[1].level, LogLevel::Error);
    EXPECT_EQ(entries[1].message, "Disk full");
}

TEST_F(LogCacheTest, SecondCallReturnsCachedData) {
    LogCache cache;

    auto entries1 = cache.getEntries(tmpFile_);
    auto entries2 = cache.getEntries(tmpFile_);

    // Both spans should point to the same underlying data
    EXPECT_EQ(entries1.data(), entries2.data());
    EXPECT_EQ(entries1.size(), entries2.size());
}

TEST_F(LogCacheTest, EvictRemovesCachedEntry) {
    LogCache cache;

    cache.getEntries(tmpFile_);
    EXPECT_EQ(cache.size(), 1u);

    cache.evict(tmpFile_);
    EXPECT_EQ(cache.size(), 0u);

    // Next call should re-parse
    auto entries = cache.getEntries(tmpFile_);
    EXPECT_EQ(cache.size(), 1u);
    ASSERT_EQ(entries.size(), 2u);
}

TEST_F(LogCacheTest, ClearEmptiesCache) {
    LogCache cache;

    cache.getEntries(tmpFile_);
    EXPECT_EQ(cache.size(), 1u);

    cache.clear();
    EXPECT_EQ(cache.size(), 0u);
}

TEST_F(LogCacheTest, FileModificationInvalidatesCache) {
    LogCache cache;

    auto entries1 = cache.getEntries(tmpFile_);
    ASSERT_EQ(entries1.size(), 2u);
    auto* oldData = entries1.data();

    // Wait briefly so filesystem modification time changes
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Rewrite the file with different content
    writeLogFile("[2024-01-15 12:00:00] WARNING [Auth] Session expired\n");

    auto entries2 = cache.getEntries(tmpFile_);

    // Should have re-parsed — different data
    ASSERT_EQ(entries2.size(), 1u);
    EXPECT_EQ(entries2[0].level, LogLevel::Warning);
    EXPECT_EQ(entries2[0].message, "Session expired");
    EXPECT_NE(entries2.data(), oldData);
}
