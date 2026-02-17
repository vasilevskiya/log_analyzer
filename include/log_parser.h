#pragma once

#include "log_entry.h"
#include <vector>
#include <string_view>
#include <span>
#include <filesystem>
#include <memory>

namespace LogAnalyzer {

// Modern class using RAII with smart pointers and filesystem
class LogParser {
public:
    // Constructor using std::filesystem::path (C++17)
    explicit LogParser(const std::filesystem::path& logFile);
    
    // Deleted copy operations (expensive to copy), move-only type
    LogParser(const LogParser&) = delete;
    LogParser& operator=(const LogParser&) = delete;
    LogParser(LogParser&&) = default;
    LogParser& operator=(LogParser&&) = default;
    
    // Parse all entries - returns by value (move semantics make this efficient)
    [[nodiscard]] std::vector<LogEntry> parseAll();
    
    // Parse single line - demonstrates string_view for non-owning string refs
    [[nodiscard]] static std::optional<LogEntry> parseLine(std::string_view line);
    
    // Get file size using filesystem library
    [[nodiscard]] std::uintmax_t getFileSize() const;
    
private:
    std::filesystem::path filePath_;
    
    // Parse timestamp from string
    static std::optional<LogEntry::TimePoint> parseTimestamp(std::string_view timeStr);
};

// Free function demonstrating std::span (non-owning view of contiguous data)
[[nodiscard]] std::vector<LogEntry> filterByLevel(
    std::span<const LogEntry> entries, 
    LogLevel level
);

} // namespace LogAnalyzer
