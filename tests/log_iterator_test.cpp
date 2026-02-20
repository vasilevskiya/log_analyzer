#include <gtest/gtest.h>
#include "log_iterator.h"

#include <algorithm>
#include <fstream>
#include <filesystem>
#include <ranges>
#include <vector>

using namespace LogAnalyzer;

// --- Test fixture ---

class BracketLogIteratorTest : public ::testing::Test {
protected:
    std::filesystem::path tmpFile_;

    void SetUp() override {
        tmpFile_ = std::filesystem::temp_directory_path() / "log_iterator_bracket_test.log";
    }

    void TearDown() override {
        std::filesystem::remove(tmpFile_);
    }
};

class CsvLogIteratorTest : public ::testing::Test {
protected:
    std::filesystem::path tmpFile_;

    void SetUp() override {
        tmpFile_ = std::filesystem::temp_directory_path() / "log_iterator_csv_test.csv";
    }

    void TearDown() override {
        std::filesystem::remove(tmpFile_);
    }
};

// --- BracketFormat tests ---

TEST_F(BracketLogIteratorTest, IterateOverAllEntries) {
    {
        std::ofstream out(tmpFile_);
        out << "[2024-01-15 10:30:45] INFO [Network] Connected\n";
        out << "[2024-01-15 10:31:00] ERROR [Disk] Write failed\n";
        out << "[2024-01-15 10:32:00] WARNING Security alert\n";
    }

    std::vector<LogEntry> entries;
    for (const auto& entry : BracketLogFile(tmpFile_)) {
        entries.push_back(entry);
    }

    ASSERT_EQ(entries.size(), 3u);
    EXPECT_EQ(entries[0].level, LogLevel::Info);
    EXPECT_EQ(entries[0].message, "Connected");
    EXPECT_EQ(entries[1].level, LogLevel::Error);
    EXPECT_EQ(entries[1].message, "Write failed");
    EXPECT_EQ(entries[2].level, LogLevel::Warning);
    EXPECT_EQ(entries[2].message, "Security alert");
}

TEST_F(BracketLogIteratorTest, EmptyFileProducesEmptyRange) {
    {
        std::ofstream out(tmpFile_);
        // write nothing
    }

    BracketLogFile logFile(tmpFile_);
    EXPECT_EQ(logFile.begin(), logFile.end());
}

TEST_F(BracketLogIteratorTest, UnparseableLinesAreSkipped) {
    {
        std::ofstream out(tmpFile_);
        out << "this is garbage\n";
        out << "[2024-01-15 10:30:45] INFO [Net] OK\n";
        out << "more garbage\n";
        out << "[2024-01-15 10:31:00] ERROR Fail\n";
        out << "trailing junk\n";
    }

    std::vector<LogEntry> entries;
    for (const auto& entry : BracketLogFile(tmpFile_)) {
        entries.push_back(entry);
    }

    ASSERT_EQ(entries.size(), 2u);
    EXPECT_EQ(entries[0].message, "OK");
    EXPECT_EQ(entries[1].message, "Fail");
}

TEST_F(BracketLogIteratorTest, LazyBehavior) {
    {
        std::ofstream out(tmpFile_);
        out << "[2024-01-15 10:30:45] INFO [Net] First\n";
        out << "[2024-01-15 10:31:00] ERROR Second\n";
    }

    LogIterator<BracketFormat> it(tmpFile_);
    LogIterator<BracketFormat> end;

    // First entry is available immediately after construction
    ASSERT_NE(it, end);
    EXPECT_EQ(it->message, "First");

    // Advance to second entry
    ++it;
    ASSERT_NE(it, end);
    EXPECT_EQ(it->message, "Second");

    // Advance past end
    ++it;
    EXPECT_EQ(it, end);
}

TEST_F(BracketLogIteratorTest, WorksWithRangesForEach) {
    {
        std::ofstream out(tmpFile_);
        out << "[2024-01-15 10:30:45] INFO [Net] Alpha\n";
        out << "[2024-01-15 10:31:00] ERROR Beta\n";
    }

    int count = 0;
    std::ranges::for_each(BracketLogFile(tmpFile_),
                          [&count](const LogEntry&) { ++count; });
    EXPECT_EQ(count, 2);
}

// --- CsvFormat tests ---

TEST_F(CsvLogIteratorTest, IterateOverCsvEntries) {
    {
        std::ofstream out(tmpFile_);
        out << "# comment\n";
        out << "2024-01-15 10:30:45,INFO,Network,Connected\n";
        out << "2024-01-15 10:31:00,ERROR,,Disk full\n";
    }

    std::vector<LogEntry> entries;
    for (const auto& entry : CsvLogFile(tmpFile_)) {
        entries.push_back(entry);
    }

    ASSERT_EQ(entries.size(), 2u);
    EXPECT_EQ(entries[0].level, LogLevel::Info);
    EXPECT_EQ(entries[0].message, "Connected");
    ASSERT_TRUE(entries[0].module.has_value());
    EXPECT_EQ(*entries[0].module, "Network");

    EXPECT_EQ(entries[1].level, LogLevel::Error);
    EXPECT_EQ(entries[1].message, "Disk full");
    EXPECT_FALSE(entries[1].module.has_value());
}

TEST_F(CsvLogIteratorTest, EmptyFileProducesEmptyRange) {
    {
        std::ofstream out(tmpFile_);
    }

    CsvLogFile logFile(tmpFile_);
    EXPECT_EQ(logFile.begin(), logFile.end());
}
