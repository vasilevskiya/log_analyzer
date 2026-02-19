#pragma once

#include "log_entry.h"
#include <concepts>
#include <optional>
#include <string_view>

namespace LogAnalyzer {

// C++20 concept: a FormatPolicy must provide a static parseLine method
template<typename T>
concept FormatPolicy = requires(std::string_view line) {
    { T::parseLine(line) } -> std::same_as<std::optional<LogEntry>>;
};

// Policy for bracket-delimited log format:
//   [YYYY-MM-DD HH:MM:SS] LEVEL [Module] Message
struct BracketFormat {
    [[nodiscard]] static std::optional<LogEntry> parseLine(std::string_view line);

private:
    static std::optional<LogEntry::TimePoint> parseTimestamp(std::string_view timeStr);
};

// Policy for CSV log format:
//   timestamp,LEVEL,module,message
// Lines starting with '#' are treated as comments and skipped.
// Empty module field results in nullopt.
struct CsvFormat {
    [[nodiscard]] static std::optional<LogEntry> parseLine(std::string_view line);

private:
    static std::optional<LogEntry::TimePoint> parseTimestamp(std::string_view timeStr);
};

} // namespace LogAnalyzer
