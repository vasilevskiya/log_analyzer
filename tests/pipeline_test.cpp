#include <gtest/gtest.h>
#include "filter.h"

#include <ranges>
#include <string>
#include <vector>

using namespace LogAnalyzer;

namespace {

LogEntry makeEntry(LogLevel level, const std::string& message,
                   std::optional<std::string> module = std::nullopt,
                   int minuteOffset = 0) {
    std::tm tm = {};
    tm.tm_year = 2024 - 1900;
    tm.tm_mon = 0;
    tm.tm_mday = 15;
    tm.tm_hour = 10;
    tm.tm_min = minuteOffset;
    tm.tm_isdst = -1;
    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    return {tp, level, message, std::move(module)};
}

std::vector<LogEntry> sampleEntries() {
    return {
        makeEntry(LogLevel::Info,     "startup",       "Network",  0),
        makeEntry(LogLevel::Warning,  "high latency",  "Network",  1),
        makeEntry(LogLevel::Error,    "login failed",  "Auth",     2),
        makeEntry(LogLevel::Debug,    "trace",         std::nullopt, 3),
        makeEntry(LogLevel::Critical, "disk full",     "Storage",  4),
        makeEntry(LogLevel::Error,    "timeout",       "Network",  5),
    };
}

} // anonymous namespace

// --- in_level adaptor ---

TEST(Pipeline, InLevelFiltersExactLevel) {
    auto entries = sampleEntries();
    std::vector<std::string> messages;
    for (const auto& e : entries | in_level(LogLevel::Error)) {
        messages.push_back(e.message);
    }
    ASSERT_EQ(messages.size(), 2u);
    EXPECT_EQ(messages[0], "login failed");
    EXPECT_EQ(messages[1], "timeout");
}

// --- at_least_level adaptor ---

TEST(Pipeline, AtLeastLevelFiltersWarningAndAbove) {
    auto entries = sampleEntries();
    std::vector<std::string> messages;
    for (const auto& e : entries | at_least_level(LogLevel::Warning)) {
        messages.push_back(e.message);
    }
    ASSERT_EQ(messages.size(), 4u);
    EXPECT_EQ(messages[0], "high latency");
    EXPECT_EQ(messages[1], "login failed");
    EXPECT_EQ(messages[2], "disk full");
    EXPECT_EQ(messages[3], "timeout");
}

// --- in_module adaptor ---

TEST(Pipeline, InModuleFiltersMatchingModule) {
    auto entries = sampleEntries();
    std::vector<std::string> messages;
    for (const auto& e : entries | in_module("Network")) {
        messages.push_back(e.message);
    }
    ASSERT_EQ(messages.size(), 3u);
    EXPECT_EQ(messages[0], "startup");
    EXPECT_EQ(messages[1], "high latency");
    EXPECT_EQ(messages[2], "timeout");
}

// --- Chaining multiple adaptors ---

TEST(Pipeline, ChainModuleAndLevel) {
    auto entries = sampleEntries();
    auto pipeline = entries
        | in_module("Network")
        | at_least_level(LogLevel::Error);

    std::vector<std::string> messages;
    for (const auto& e : pipeline) {
        messages.push_back(e.message);
    }
    ASSERT_EQ(messages.size(), 1u);
    EXPECT_EQ(messages[0], "timeout");
}

// --- extract_messages adaptor ---

TEST(Pipeline, ExtractMessagesProducesPairs) {
    auto entries = sampleEntries();
    auto pipeline = entries
        | at_least_level(LogLevel::Critical)
        | extract_messages();

    std::vector<std::pair<LogEntry::TimePoint, std::string_view>> result;
    for (auto pair : pipeline) {
        result.push_back(pair);
    }
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].second, "disk full");
}

// --- Composing with std::views::take ---

TEST(Pipeline, TakeLimitsResults) {
    auto entries = sampleEntries();
    auto pipeline = entries
        | at_least_level(LogLevel::Warning)
        | std::views::take(2);

    std::vector<std::string> messages;
    for (const auto& e : pipeline) {
        messages.push_back(e.message);
    }
    ASSERT_EQ(messages.size(), 2u);
    EXPECT_EQ(messages[0], "high latency");
    EXPECT_EQ(messages[1], "login failed");
}

// --- Laziness verification ---

TEST(Pipeline, PipelineIsLazy) {
    auto entries = sampleEntries();
    int evalCount = 0;

    // Create a pipeline but don't iterate it.
    auto pipeline = entries
        | std::views::filter([&evalCount](const LogEntry& e) {
            ++evalCount;
            return e.level >= LogLevel::Warning;
        })
        | std::views::take(2);

    // No iteration has happened yet.
    EXPECT_EQ(evalCount, 0);

    // Now iterate — the filter runs only as needed.
    std::vector<std::string> messages;
    for (const auto& e : pipeline) {
        messages.push_back(e.message);
    }
    ASSERT_EQ(messages.size(), 2u);
    // The filter evaluated some elements but not necessarily all 6.
    EXPECT_GT(evalCount, 0);
    EXPECT_LE(evalCount, static_cast<int>(entries.size()));
}

// --- Empty result ---

TEST(Pipeline, NoMatchReturnsEmpty) {
    auto entries = sampleEntries();
    auto pipeline = entries | in_module("NonExistent");

    int count = 0;
    for ([[maybe_unused]] const auto& e : pipeline) {
        ++count;
    }
    EXPECT_EQ(count, 0);
}
