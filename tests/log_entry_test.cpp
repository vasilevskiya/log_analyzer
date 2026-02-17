#include <gtest/gtest.h>
#include "log_entry.h"

using namespace LogAnalyzer;

// --- parseLogLevel tests ---

TEST(ParseLogLevel, ValidLevels) {
    EXPECT_EQ(parseLogLevel("DEBUG"), LogLevel::Debug);
    EXPECT_EQ(parseLogLevel("INFO"), LogLevel::Info);
    EXPECT_EQ(parseLogLevel("WARNING"), LogLevel::Warning);
    EXPECT_EQ(parseLogLevel("ERROR"), LogLevel::Error);
    EXPECT_EQ(parseLogLevel("CRITICAL"), LogLevel::Critical);
}

TEST(ParseLogLevel, CaseInsensitive) {
    EXPECT_EQ(parseLogLevel("debug"), LogLevel::Debug);
    EXPECT_EQ(parseLogLevel("Info"), LogLevel::Info);
    EXPECT_EQ(parseLogLevel("wArNiNg"), LogLevel::Warning);
    EXPECT_EQ(parseLogLevel("error"), LogLevel::Error);
    EXPECT_EQ(parseLogLevel("Critical"), LogLevel::Critical);
}

TEST(ParseLogLevel, InvalidStrings) {
    EXPECT_EQ(parseLogLevel("TRACE"), std::nullopt);
    EXPECT_EQ(parseLogLevel("WARN"), std::nullopt);
    EXPECT_EQ(parseLogLevel("FATAL"), std::nullopt);
    EXPECT_EQ(parseLogLevel("gibberish"), std::nullopt);
}

TEST(ParseLogLevel, EmptyString) {
    EXPECT_EQ(parseLogLevel(""), std::nullopt);
}

// --- toString tests ---

TEST(ToString, AllLevels) {
    EXPECT_EQ(toString(LogLevel::Debug), "DEBUG");
    EXPECT_EQ(toString(LogLevel::Info), "INFO");
    EXPECT_EQ(toString(LogLevel::Warning), "WARNING");
    EXPECT_EQ(toString(LogLevel::Error), "ERROR");
    EXPECT_EQ(toString(LogLevel::Critical), "CRITICAL");
}

TEST(ToString, RoundTripsWithParseLogLevel) {
    for (auto level : {LogLevel::Debug, LogLevel::Info, LogLevel::Warning,
                       LogLevel::Error, LogLevel::Critical}) {
        auto str = toString(level);
        auto parsed = parseLogLevel(str);
        ASSERT_TRUE(parsed.has_value()) << "Failed to parse: " << str;
        EXPECT_EQ(*parsed, level);
    }
}

// --- formatTimestamp tests ---

TEST(FormatTimestamp, ProducesExpectedFormat) {
    std::tm tm = {};
    tm.tm_year = 2024 - 1900;
    tm.tm_mon = 0;   // January
    tm.tm_mday = 15;
    tm.tm_hour = 10;
    tm.tm_min = 30;
    tm.tm_sec = 45;
    tm.tm_isdst = -1;

    auto time = std::mktime(&tm);
    auto tp = std::chrono::system_clock::from_time_t(time);

    LogEntry entry{tp, LogLevel::Info, "test"};
    EXPECT_EQ(entry.formatTimestamp(), "2024-01-15 10:30:45");
}
