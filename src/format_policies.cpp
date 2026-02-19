#include "format_policies.h"
#include <sstream>
#include <iomanip>

namespace LogAnalyzer {

namespace {

// Shared timestamp parser for "YYYY-MM-DD HH:MM:SS" format
std::optional<LogEntry::TimePoint> parseTimestampString(std::string_view timeStr) {
    std::tm tm = {};
    std::string timeString{timeStr};
    std::istringstream ss{timeString};
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

    if (ss.fail()) {
        return std::nullopt;
    }

    auto time = std::mktime(&tm);
    return std::chrono::system_clock::from_time_t(time);
}

} // anonymous namespace

// --- BracketFormat ---

std::optional<LogEntry> BracketFormat::parseLine(std::string_view line) {
    if (line.empty() || line[0] != '[') {
        return std::nullopt;
    }

    // Find timestamp end
    auto timestampEnd = line.find(']');
    if (timestampEnd == std::string_view::npos) {
        return std::nullopt;
    }

    // Extract and parse timestamp
    auto timestampStr = line.substr(1, timestampEnd - 1);
    auto timestamp = parseTimestamp(timestampStr);
    if (!timestamp) {
        return std::nullopt;
    }

    // Move past timestamp
    line.remove_prefix(timestampEnd + 1);

    // Skip whitespace
    while (!line.empty() && std::isspace(static_cast<unsigned char>(line[0]))) {
        line.remove_prefix(1);
    }

    // Extract level
    auto levelEnd = line.find(' ');
    if (levelEnd == std::string_view::npos) {
        return std::nullopt;
    }

    auto levelStr = line.substr(0, levelEnd);
    auto level = parseLogLevel(levelStr);
    if (!level) {
        return std::nullopt;
    }

    // Move past level
    line.remove_prefix(levelEnd + 1);

    // Extract optional module
    std::optional<std::string> module;
    if (!line.empty() && line[0] == '[') {
        auto moduleEnd = line.find(']');
        if (moduleEnd != std::string_view::npos) {
            module = std::string(line.substr(1, moduleEnd - 1));
            line.remove_prefix(moduleEnd + 1);

            // Skip whitespace after module
            while (!line.empty() && std::isspace(static_cast<unsigned char>(line[0]))) {
                line.remove_prefix(1);
            }
        }
    }

    // Rest is the message
    std::string message(line);

    return LogEntry{*timestamp, *level, std::move(message), std::move(module)};
}

std::optional<LogEntry::TimePoint> BracketFormat::parseTimestamp(std::string_view timeStr) {
    return parseTimestampString(timeStr);
}

// --- CsvFormat ---

std::optional<LogEntry> CsvFormat::parseLine(std::string_view line) {
    // Skip empty lines and comments
    if (line.empty() || line[0] == '#') {
        return std::nullopt;
    }

    // Split on first 3 commas: timestamp,LEVEL,module,message
    // (message may contain commas)
    auto comma1 = line.find(',');
    if (comma1 == std::string_view::npos) {
        return std::nullopt;
    }

    auto comma2 = line.find(',', comma1 + 1);
    if (comma2 == std::string_view::npos) {
        return std::nullopt;
    }

    auto comma3 = line.find(',', comma2 + 1);
    if (comma3 == std::string_view::npos) {
        return std::nullopt;
    }

    auto timestampStr = line.substr(0, comma1);
    auto levelStr = line.substr(comma1 + 1, comma2 - comma1 - 1);
    auto moduleStr = line.substr(comma2 + 1, comma3 - comma2 - 1);
    auto messageStr = line.substr(comma3 + 1);

    // Parse timestamp
    auto timestamp = parseTimestamp(timestampStr);
    if (!timestamp) {
        return std::nullopt;
    }

    // Parse level
    auto level = parseLogLevel(levelStr);
    if (!level) {
        return std::nullopt;
    }

    // Module: empty string means no module
    std::optional<std::string> module;
    if (!moduleStr.empty()) {
        module = std::string(moduleStr);
    }

    std::string message(messageStr);

    return LogEntry{*timestamp, *level, std::move(message), std::move(module)};
}

std::optional<LogEntry::TimePoint> CsvFormat::parseTimestamp(std::string_view timeStr) {
    return parseTimestampString(timeStr);
}

} // namespace LogAnalyzer
