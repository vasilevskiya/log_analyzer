#include <gtest/gtest.h>
#include "async_parser.h"

#include <fstream>
#include <filesystem>

using namespace LogAnalyzer;

// --- AsyncLogParser (BracketFormat) tests ---

class AsyncParserTest : public ::testing::Test {
protected:
    std::filesystem::path tmpFile_;

    void SetUp() override {
        tmpFile_ = std::filesystem::temp_directory_path() / "async_parser_test.log";
        std::ofstream out(tmpFile_);
        out << "[2024-01-15 10:30:45] INFO [Network] Connection established\n";
        out << "[2024-01-15 10:31:00] ERROR [Disk] Write failure\n";
    }

    void TearDown() override {
        std::filesystem::remove(tmpFile_);
    }
};

TEST_F(AsyncParserTest, ParseAsync) {
    AsyncLogParser parser;
    auto future = parser.parseAsync(tmpFile_);
    auto entries = future.get();

    ASSERT_EQ(entries.size(), 2u);
    EXPECT_EQ(entries[0].level, LogLevel::Info);
    EXPECT_EQ(entries[0].message, "Connection established");
    EXPECT_EQ(entries[1].level, LogLevel::Error);
    EXPECT_EQ(entries[1].message, "Write failure");
}

TEST_F(AsyncParserTest, ParseAsyncNonExistent) {
    AsyncLogParser parser;
    auto future = parser.parseAsync("/tmp/does_not_exist_12345.log");
    EXPECT_THROW(future.get(), std::runtime_error);
}

// --- ParseMultiple tests ---

class AsyncParserMultipleTest : public ::testing::Test {
protected:
    std::vector<std::filesystem::path> tmpFiles_;

    void SetUp() override {
        for (int i = 0; i < 3; ++i) {
            auto path = std::filesystem::temp_directory_path() /
                        ("async_multi_test_" + std::to_string(i) + ".log");
            tmpFiles_.push_back(path);
            std::ofstream out(path);
            out << "[2024-01-15 10:30:45] INFO [Mod" << i << "] Message " << i << "\n";
        }
    }

    void TearDown() override {
        for (const auto& f : tmpFiles_) {
            std::filesystem::remove(f);
        }
    }
};

TEST_F(AsyncParserMultipleTest, ParseMultiple) {
    AsyncLogParser parser;
    auto futures = parser.parseMultiple(tmpFiles_);

    ASSERT_EQ(futures.size(), 3u);
    for (std::size_t i = 0; i < futures.size(); ++i) {
        auto entries = futures[i].get();
        ASSERT_EQ(entries.size(), 1u);
        EXPECT_EQ(entries[0].level, LogLevel::Info);
        EXPECT_EQ(entries[0].message, "Message " + std::to_string(i));
    }
}

TEST(AsyncParserEmptyTest, ParseMultipleEmpty) {
    AsyncLogParser parser;
    auto futures = parser.parseMultiple({});
    EXPECT_TRUE(futures.empty());
}

// --- GenericAsyncParser<CsvFormat> test ---

class AsyncCsvParserTest : public ::testing::Test {
protected:
    std::filesystem::path tmpFile_;

    void SetUp() override {
        tmpFile_ = std::filesystem::temp_directory_path() / "async_csv_test.csv";
        std::ofstream out(tmpFile_);
        out << "2024-01-15 10:30:45,INFO,Network,Connected\n";
        out << "2024-01-15 10:31:00,ERROR,,Disk full\n";
    }

    void TearDown() override {
        std::filesystem::remove(tmpFile_);
    }
};

TEST_F(AsyncCsvParserTest, ParseAsyncCsv) {
    GenericAsyncParser<CsvFormat> parser;
    auto future = parser.parseAsync(tmpFile_);
    auto entries = future.get();

    ASSERT_EQ(entries.size(), 2u);
    EXPECT_EQ(entries[0].level, LogLevel::Info);
    EXPECT_EQ(entries[0].message, "Connected");
    EXPECT_EQ(entries[1].level, LogLevel::Error);
    EXPECT_EQ(entries[1].message, "Disk full");
    EXPECT_FALSE(entries[1].module.has_value());
}
