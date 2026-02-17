#include "log_parser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>

namespace LogAnalyzer {

LogParser::LogParser(const std::filesystem::path& logFile)
    : filePath_(logFile) {
    // Check if file exists using filesystem
    if (!std::filesystem::exists(filePath_)) {
        throw std::runtime_error("Log file does not exist: " + filePath_.string());
    }
}

std::vector<LogEntry> LogParser::parseAll() {
    std::vector<LogEntry> entries;
    std::ifstream file(filePath_);
    
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filePath_.string());
    }
    
    // Reserve space to avoid reallocations (if we know approx size)
    if (auto size = getFileSize(); size > 0) {
        entries.reserve(size / 100);  // Rough estimate
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Use structured binding with if-initializer (C++17)
        if (auto entry = parseLine(line); entry.has_value()) {
            entries.push_back(std::move(*entry));  // Move from optional
        }
    }
    
    return entries;  // RVO/move semantics handle efficiency
}

std::optional<LogEntry> LogParser::parseLine(std::string_view line) {
    // Simple format: [TIMESTAMP] LEVEL [MODULE] Message
    // Example: [2024-01-15 10:30:45] INFO [Network] Connection established
    
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
    
    // Use aggregate initialization and move semantics
    return LogEntry{*timestamp, *level, std::move(message), std::move(module)};
}

std::optional<LogEntry::TimePoint> LogParser::parseTimestamp(std::string_view timeStr) {
    // Simple parser for format: YYYY-MM-DD HH:MM:SS
    std::tm tm = {};
    std::string timeString{timeStr};  // Convert string_view to string
    std::istringstream ss{timeString}; // Use braces to avoid most vexing parse
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    if (ss.fail()) {
        return std::nullopt;
    }
    
    auto time = std::mktime(&tm);
    return std::chrono::system_clock::from_time_t(time);
}

std::uintmax_t LogParser::getFileSize() const {
    // Modern filesystem operations
    std::error_code ec;
    auto size = std::filesystem::file_size(filePath_, ec);
    return ec ? 0 : size;
}

std::vector<LogEntry> filterByLevel(std::span<const LogEntry> entries, LogLevel level) {
    std::vector<LogEntry> result;
    
    // Using std::ranges::copy_if with lambda (C++20)
    std::ranges::copy_if(entries, std::back_inserter(result),
        [level](const auto& entry) { return entry.level == level; });
    
    return result;
}

} // namespace LogAnalyzer
