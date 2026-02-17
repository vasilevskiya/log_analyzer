#include <gtest/gtest.h>
#include "log_parser.h"

#include <fstream>
#include <filesystem>

using namespace LogAnalyzer;

// --- parseLine tests ---

TEST(ParseLine, ValidLineWithModule) {
    auto result = LogParser::parseLine(
        "[2024-01-15 10:30:45] INFO [Network] Connection established");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->level, LogLevel::Info);
    EXPECT_EQ(result->message, "Connection established");
    ASSERT_TRUE(result->module.has_value());
    EXPECT_EQ(*result->module, "Network");
}

TEST(ParseLine, ValidLineWithoutModule) {
    auto result = LogParser::parseLine(
        "[2024-01-15 10:30:45] ERROR Something went wrong");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->level, LogLevel::Error);
    EXPECT_EQ(result->message, "Something went wrong");
    EXPECT_FALSE(result->module.has_value());
}

TEST(ParseLine, EmptyLine) {
    EXPECT_EQ(LogParser::parseLine(""), std::nullopt);
}

TEST(ParseLine, MissingTimestampBracket) {
    EXPECT_EQ(LogParser::parseLine("2024-01-15 10:30:45] INFO msg"), std::nullopt);
}

TEST(ParseLine, NoClosingBracket) {
    EXPECT_EQ(LogParser::parseLine("[2024-01-15 10:30:45 INFO msg"), std::nullopt);
}

TEST(ParseLine, InvalidLevel) {
    EXPECT_EQ(LogParser::parseLine("[2024-01-15 10:30:45] TRACE msg"), std::nullopt);
}

TEST(ParseLine, InvalidTimestamp) {
    EXPECT_EQ(LogParser::parseLine("[not-a-timestamp] INFO msg"), std::nullopt);
}

// --- filterByLevel tests ---

TEST(FilterByLevel, FiltersMixed) {
    std::tm tm = {};
    tm.tm_year = 2024 - 1900;
    tm.tm_mon = 0;
    tm.tm_mday = 1;
    tm.tm_isdst = -1;
    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

    std::vector<LogEntry> entries = {
        {tp, LogLevel::Info, "msg1"},
        {tp, LogLevel::Error, "msg2"},
        {tp, LogLevel::Info, "msg3"},
        {tp, LogLevel::Debug, "msg4"},
    };

    auto infos = filterByLevel(entries, LogLevel::Info);
    ASSERT_EQ(infos.size(), 2u);
    EXPECT_EQ(infos[0].message, "msg1");
    EXPECT_EQ(infos[1].message, "msg3");

    auto errors = filterByLevel(entries, LogLevel::Error);
    ASSERT_EQ(errors.size(), 1u);
    EXPECT_EQ(errors[0].message, "msg2");

    auto warnings = filterByLevel(entries, LogLevel::Warning);
    EXPECT_TRUE(warnings.empty());
}

// --- LogParser constructor and parseAll tests ---

class LogParserFileTest : public ::testing::Test {
protected:
    std::filesystem::path tmpFile_;

    void SetUp() override {
        tmpFile_ = std::filesystem::temp_directory_path() / "log_parser_test.log";
    }

    void TearDown() override {
        std::filesystem::remove(tmpFile_);
    }
};

TEST(LogParserConstructor, ThrowsOnNonExistentFile) {
    EXPECT_THROW(LogParser("/tmp/nonexistent_log_file_abc123.log"),
                 std::runtime_error);
}

TEST_F(LogParserFileTest, ParseAllReturnsEntries) {
    {
        std::ofstream out(tmpFile_);
        out << "[2024-01-15 10:30:45] INFO [Network] Connected\n";
        out << "this line is invalid\n";
        out << "[2024-01-15 10:31:00] ERROR Disk full\n";
    }

    LogParser parser(tmpFile_);
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
