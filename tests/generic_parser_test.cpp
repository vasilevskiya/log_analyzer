#include <gtest/gtest.h>
#include "log_parser.h"
#include "log_cache.h"

#include <fstream>
#include <filesystem>

using namespace LogAnalyzer;

// --- BracketFormat::parseLine tests ---

TEST(BracketFormatParseLine, ValidLineWithModule) {
    auto result = BracketFormat::parseLine(
        "[2024-01-15 10:30:45] INFO [Network] Connection established");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->level, LogLevel::Info);
    EXPECT_EQ(result->message, "Connection established");
    ASSERT_TRUE(result->module.has_value());
    EXPECT_EQ(*result->module, "Network");
}

TEST(BracketFormatParseLine, ValidLineWithoutModule) {
    auto result = BracketFormat::parseLine(
        "[2024-01-15 10:30:45] ERROR Something went wrong");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->level, LogLevel::Error);
    EXPECT_EQ(result->message, "Something went wrong");
    EXPECT_FALSE(result->module.has_value());
}

// --- CsvFormat::parseLine tests ---

TEST(CsvFormatParseLine, ValidLine) {
    auto result = CsvFormat::parseLine(
        "2024-01-15 10:30:45,INFO,Network,Connection established");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->level, LogLevel::Info);
    EXPECT_EQ(result->message, "Connection established");
    ASSERT_TRUE(result->module.has_value());
    EXPECT_EQ(*result->module, "Network");
}

TEST(CsvFormatParseLine, EmptyModuleGivesNullopt) {
    auto result = CsvFormat::parseLine(
        "2024-01-15 10:30:45,WARNING,,Something happened");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->level, LogLevel::Warning);
    EXPECT_EQ(result->message, "Something happened");
    EXPECT_FALSE(result->module.has_value());
}

TEST(CsvFormatParseLine, CommentLineSkipped) {
    EXPECT_EQ(CsvFormat::parseLine("# this is a comment"), std::nullopt);
}

TEST(CsvFormatParseLine, InvalidLevel) {
    EXPECT_EQ(CsvFormat::parseLine("2024-01-15 10:30:45,TRACE,Net,msg"), std::nullopt);
}

TEST(CsvFormatParseLine, MalformedTooFewCommas) {
    EXPECT_EQ(CsvFormat::parseLine("2024-01-15 10:30:45,INFO,no_message_field"), std::nullopt);
}

TEST(CsvFormatParseLine, MessageWithEmbeddedCommas) {
    auto result = CsvFormat::parseLine(
        "2024-01-15 10:30:45,INFO,Net,hello, world, again");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->message, "hello, world, again");
}

// --- GenericParser<CsvFormat> file tests ---

class CsvParserFileTest : public ::testing::Test {
protected:
    std::filesystem::path tmpFile_;

    void SetUp() override {
        tmpFile_ = std::filesystem::temp_directory_path() / "generic_parser_csv_test.csv";
    }

    void TearDown() override {
        std::filesystem::remove(tmpFile_);
    }
};

TEST_F(CsvParserFileTest, ParseAllReturnsCsvEntries) {
    {
        std::ofstream out(tmpFile_);
        out << "# comment line\n";
        out << "2024-01-15 10:30:45,INFO,Network,Connected\n";
        out << "this line is invalid\n";
        out << "2024-01-15 10:31:00,ERROR,,Disk full\n";
    }

    GenericParser<CsvFormat> parser(tmpFile_);
    auto entries = parser.parseAll();

    ASSERT_EQ(entries.size(), 2u);
    EXPECT_EQ(entries[0].level, LogLevel::Info);
    EXPECT_EQ(entries[0].message, "Connected");
    ASSERT_TRUE(entries[0].module.has_value());
    EXPECT_EQ(*entries[0].module, "Network");

    EXPECT_EQ(entries[1].level, LogLevel::Error);
    EXPECT_EQ(entries[1].message, "Disk full");
    EXPECT_FALSE(entries[1].module.has_value());
}

// --- GenericLogCache<CsvFormat> tests ---

class CsvLogCacheTest : public ::testing::Test {
protected:
    std::filesystem::path tmpFile_;

    void SetUp() override {
        tmpFile_ = std::filesystem::temp_directory_path() / "generic_cache_csv_test.csv";
        std::ofstream out(tmpFile_);
        out << "2024-01-15 10:30:45,INFO,Network,Connected\n";
        out << "2024-01-15 10:31:00,ERROR,,Disk full\n";
    }

    void TearDown() override {
        std::filesystem::remove(tmpFile_);
    }
};

TEST_F(CsvLogCacheTest, CacheMissAndHit) {
    GenericLogCache<CsvFormat> cache;
    EXPECT_EQ(cache.size(), 0u);

    auto entries1 = cache.getEntries(tmpFile_);
    EXPECT_EQ(cache.size(), 1u);
    ASSERT_EQ(entries1.size(), 2u);
    EXPECT_EQ(entries1[0].level, LogLevel::Info);
    EXPECT_EQ(entries1[1].level, LogLevel::Error);

    // Second call should be a cache hit (same data pointer)
    auto entries2 = cache.getEntries(tmpFile_);
    EXPECT_EQ(entries1.data(), entries2.data());
}
