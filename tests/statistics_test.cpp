#include <gtest/gtest.h>
#include "statistics.h"

using namespace LogAnalyzer;

namespace {

LogEntry::TimePoint makeTime(int year, int mon, int day,
                             int hour = 0, int min = 0, int sec = 0) {
    std::tm tm = {};
    tm.tm_year = year - 1900;
    tm.tm_mon = mon - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;
    tm.tm_isdst = -1;
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::vector<LogEntry> makeSampleEntries() {
    return {
        {makeTime(2024, 1, 1, 8, 0, 0),  LogLevel::Info,     "a", std::string("Network")},
        {makeTime(2024, 1, 1, 9, 0, 0),  LogLevel::Info,     "b", std::string("Network")},
        {makeTime(2024, 1, 1, 10, 0, 0), LogLevel::Error,    "c", std::string("Disk")},
        {makeTime(2024, 1, 1, 11, 0, 0), LogLevel::Warning,  "d", std::string("Auth")},
        {makeTime(2024, 1, 1, 12, 0, 0), LogLevel::Debug,    "e", std::nullopt},
        {makeTime(2024, 1, 1, 13, 0, 0), LogLevel::Critical, "f", std::string("Disk")},
    };
}

} // namespace

// --- getLevelCounts tests ---

TEST(Statistics, LevelCounts) {
    auto entries = makeSampleEntries();
    Statistics stats(entries);
    auto counts = stats.getLevelCounts();

    EXPECT_EQ(counts[LogLevel::Info], 2u);
    EXPECT_EQ(counts[LogLevel::Error], 1u);
    EXPECT_EQ(counts[LogLevel::Warning], 1u);
    EXPECT_EQ(counts[LogLevel::Debug], 1u);
    EXPECT_EQ(counts[LogLevel::Critical], 1u);
}

// --- getTopModules tests ---

TEST(Statistics, TopModulesSortedDescending) {
    auto entries = makeSampleEntries();
    Statistics stats(entries);
    auto top = stats.getTopModules(10);

    ASSERT_GE(top.size(), 2u);
    // Network has 2 entries, Disk has 2, Auth has 1
    EXPECT_EQ(top[0].second, 2u);
    EXPECT_EQ(top[1].second, 2u);
    EXPECT_EQ(top[2].first, "Auth");
    EXPECT_EQ(top[2].second, 1u);
}

TEST(Statistics, TopModulesRespectsLimit) {
    auto entries = makeSampleEntries();
    Statistics stats(entries);
    auto top = stats.getTopModules(1);

    EXPECT_EQ(top.size(), 1u);
}

// --- getEntriesInRange tests ---

TEST(Statistics, EntriesInRange) {
    auto entries = makeSampleEntries();
    Statistics stats(entries);

    auto start = makeTime(2024, 1, 1, 9, 0, 0);
    auto end = makeTime(2024, 1, 1, 11, 0, 0);
    auto filtered = stats.getEntriesInRange(start, end);

    ASSERT_EQ(filtered.size(), 3u);  // 9:00, 10:00, 11:00 (inclusive)
    EXPECT_EQ(filtered[0].message, "b");
    EXPECT_EQ(filtered[1].message, "c");
    EXPECT_EQ(filtered[2].message, "d");
}

// --- empty entries edge case ---

TEST(Statistics, EmptyEntries) {
    std::vector<LogEntry> empty;
    Statistics stats(empty);

    EXPECT_TRUE(stats.getLevelCounts().empty());
    EXPECT_TRUE(stats.getTopModules().empty());

    auto start = makeTime(2024, 1, 1);
    auto end = makeTime(2024, 12, 31);
    EXPECT_TRUE(stats.getEntriesInRange(start, end).empty());
}
