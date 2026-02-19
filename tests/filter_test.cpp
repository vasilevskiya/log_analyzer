#include <gtest/gtest.h>
#include "filter.h"
#include "log_parser.h"

#include <string>
#include <vector>

using namespace LogAnalyzer;

namespace {

// Helper: build a LogEntry with minimal boilerplate.
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

// A fixed set of entries used by most tests.
std::vector<LogEntry> sampleEntries() {
    return {
        makeEntry(LogLevel::Info,     "msg1", "Network",  0),
        makeEntry(LogLevel::Error,    "msg2", "Auth",     1),
        makeEntry(LogLevel::Info,     "msg3", "Network",  2),
        makeEntry(LogLevel::Debug,    "msg4", std::nullopt, 3),
        makeEntry(LogLevel::Warning,  "msg5", "Auth",     4),
    };
}

} // anonymous namespace

// --- Generic filter() tests ---

TEST(Filter, ByLevelPredicate) {
    auto entries = sampleEntries();

    auto infos = filter(entries, byLevel(LogLevel::Info));
    ASSERT_EQ(infos.size(), 2u);
    EXPECT_EQ(infos[0].message, "msg1");
    EXPECT_EQ(infos[1].message, "msg3");
}

TEST(Filter, ByModulePredicate) {
    auto entries = sampleEntries();

    auto authEntries = filter(entries, byModule("Auth"));
    ASSERT_EQ(authEntries.size(), 2u);
    EXPECT_EQ(authEntries[0].message, "msg2");
    EXPECT_EQ(authEntries[1].message, "msg5");
}

TEST(Filter, ByModuleNoMatch) {
    auto entries = sampleEntries();
    auto result = filter(entries, byModule("Database"));
    EXPECT_TRUE(result.empty());
}

TEST(Filter, AfterTimePredicate) {
    auto entries = sampleEntries();
    // cutoff at minute 2 — should match entries at minutes 3 and 4
    auto cutoff = entries[2].timestamp;

    auto recent = filter(entries, afterTime(cutoff));
    ASSERT_EQ(recent.size(), 2u);
    EXPECT_EQ(recent[0].message, "msg4");
    EXPECT_EQ(recent[1].message, "msg5");
}

TEST(Filter, BeforeTimePredicate) {
    auto entries = sampleEntries();
    auto cutoff = entries[2].timestamp;

    auto older = filter(entries, beforeTime(cutoff));
    ASSERT_EQ(older.size(), 2u);
    EXPECT_EQ(older[0].message, "msg1");
    EXPECT_EQ(older[1].message, "msg2");
}

TEST(Filter, CustomLambdaPredicate) {
    auto entries = sampleEntries();

    auto withModule = filter(entries,
        [](const LogEntry& e) { return e.module.has_value(); });
    ASSERT_EQ(withModule.size(), 4u);
    EXPECT_EQ(withModule[0].message, "msg1");
}

TEST(Filter, EmptyInput) {
    std::vector<LogEntry> empty;
    auto result = filter(empty, byLevel(LogLevel::Error));
    EXPECT_TRUE(result.empty());
}

TEST(Filter, NoMatch) {
    auto entries = sampleEntries();
    auto result = filter(entries, byLevel(LogLevel::Critical));
    EXPECT_TRUE(result.empty());
}

// --- Genericity: filter() works with non-LogEntry containers ---

TEST(Filter, WorksWithIntVector) {
    std::vector<int> nums = {1, 2, 3, 4, 5, 6};
    auto evens = filter(nums, [](int n) { return n % 2 == 0; });
    ASSERT_EQ(evens.size(), 3u);
    EXPECT_EQ(evens[0], 2);
    EXPECT_EQ(evens[1], 4);
    EXPECT_EQ(evens[2], 6);
}

TEST(Filter, WorksWithStringVector) {
    std::vector<std::string> words = {"apple", "banana", "avocado", "cherry"};
    auto aWords = filter(words,
        [](const std::string& s) { return s.front() == 'a'; });
    ASSERT_EQ(aWords.size(), 2u);
    EXPECT_EQ(aWords[0], "apple");
    EXPECT_EQ(aWords[1], "avocado");
}

// --- filterByLevel backward-compatibility ---

TEST(FilterByLevel, DelegatesToGenericFilter) {
    auto entries = sampleEntries();

    auto errors = filterByLevel(entries, LogLevel::Error);
    ASSERT_EQ(errors.size(), 1u);
    EXPECT_EQ(errors[0].message, "msg2");
}
