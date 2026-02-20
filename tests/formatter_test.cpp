#include <gtest/gtest.h>
#include "formatter.h"

using namespace LogAnalyzer;

namespace {

LogEntry makeEntry(LogLevel level, const std::string& message,
                   std::optional<std::string> module = std::nullopt) {
    // Use a fixed time point for predictable output
    auto time = std::chrono::system_clock::from_time_t(1700000000); // 2023-11-14 22:13:20 UTC
    return LogEntry(time, level, message, std::move(module));
}

std::string expectedTimestamp() {
    // Format the same fixed time through LogEntry::formatTimestamp
    auto entry = makeEntry(LogLevel::Info, "");
    return entry.formatTimestamp();
}

// --- JsonFormatter tests ---

TEST(JsonFormatterTest, WithModule) {
    JsonFormatter fmt;
    auto entry = makeEntry(LogLevel::Error, "disk full", "Storage");
    auto result = fmt.format(entry);
    auto ts = expectedTimestamp();

    EXPECT_EQ(result, "{\"timestamp\":\"" + ts +
                      "\",\"level\":\"ERROR\",\"message\":\"disk full\",\"module\":\"Storage\"}");
}

TEST(JsonFormatterTest, WithoutModule) {
    JsonFormatter fmt;
    auto entry = makeEntry(LogLevel::Info, "started");
    auto result = fmt.format(entry);
    auto ts = expectedTimestamp();

    EXPECT_EQ(result, "{\"timestamp\":\"" + ts +
                      "\",\"level\":\"INFO\",\"message\":\"started\",\"module\":null}");
}

// --- XmlFormatter tests ---

TEST(XmlFormatterTest, WithModule) {
    XmlFormatter fmt;
    auto entry = makeEntry(LogLevel::Warning, "low memory", "System");
    auto result = fmt.format(entry);
    auto ts = expectedTimestamp();

    EXPECT_EQ(result, "<entry><timestamp>" + ts +
                      "</timestamp><level>WARNING</level>"
                      "<message>low memory</message>"
                      "<module>System</module></entry>");
}

TEST(XmlFormatterTest, WithoutModule) {
    XmlFormatter fmt;
    auto entry = makeEntry(LogLevel::Debug, "trace");
    auto result = fmt.format(entry);
    auto ts = expectedTimestamp();

    EXPECT_EQ(result, "<entry><timestamp>" + ts +
                      "</timestamp><level>DEBUG</level>"
                      "<message>trace</message></entry>");
}

// --- PlainTextFormatter tests ---

TEST(PlainTextFormatterTest, WithModule) {
    PlainTextFormatter fmt;
    auto entry = makeEntry(LogLevel::Critical, "crash", "Kernel");
    auto result = fmt.format(entry);
    auto ts = expectedTimestamp();

    EXPECT_EQ(result, "[" + ts + "] CRITICAL [Kernel] crash");
}

TEST(PlainTextFormatterTest, WithoutModule) {
    PlainTextFormatter fmt;
    auto entry = makeEntry(LogLevel::Info, "hello");
    auto result = fmt.format(entry);
    auto ts = expectedTimestamp();

    EXPECT_EQ(result, "[" + ts + "] INFO hello");
}

// --- LogFormatter (std::visit) tests ---

TEST(LogFormatterTest, VisitJson) {
    LogFormatter formatter(JsonFormatter{});
    auto entry = makeEntry(LogLevel::Info, "test", "App");
    auto result = formatter.format(entry);

    EXPECT_NE(result.find("\"level\":\"INFO\""), std::string::npos);
    EXPECT_NE(result.find("\"module\":\"App\""), std::string::npos);
}

TEST(LogFormatterTest, VisitXml) {
    LogFormatter formatter(XmlFormatter{});
    auto entry = makeEntry(LogLevel::Error, "fail");
    auto result = formatter.format(entry);

    EXPECT_NE(result.find("<level>ERROR</level>"), std::string::npos);
    EXPECT_EQ(result.find("<module>"), std::string::npos);
}

TEST(LogFormatterTest, VisitPlainText) {
    LogFormatter formatter(PlainTextFormatter{});
    auto entry = makeEntry(LogLevel::Warning, "warn", "Net");
    auto result = formatter.format(entry);

    EXPECT_NE(result.find("WARNING"), std::string::npos);
    EXPECT_NE(result.find("[Net]"), std::string::npos);
}

// --- All log levels format correctly ---

TEST(FormatterTest, AllLogLevels) {
    PlainTextFormatter fmt;
    std::vector<std::pair<LogLevel, std::string>> levels = {
        {LogLevel::Debug, "DEBUG"},
        {LogLevel::Info, "INFO"},
        {LogLevel::Warning, "WARNING"},
        {LogLevel::Error, "ERROR"},
        {LogLevel::Critical, "CRITICAL"},
    };

    for (const auto& [level, expected] : levels) {
        auto entry = makeEntry(level, "msg");
        auto result = fmt.format(entry);
        EXPECT_NE(result.find(expected), std::string::npos)
            << "Level " << expected << " not found in: " << result;
    }
}

} // namespace
