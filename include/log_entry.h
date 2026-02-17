#pragma once

#include <string>
#include <optional>
#include <chrono>
#include <string_view>

namespace LogAnalyzer {

// Enum class (type-safe enums introduced in C++11)
enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

// Convert string to LogLevel - demonstrates std::optional for failure cases
std::optional<LogLevel> parseLogLevel(std::string_view level);

// Convert LogLevel to string - constexpr enables compile-time evaluation
constexpr std::string_view toString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:    return "DEBUG";
        case LogLevel::Info:     return "INFO";
        case LogLevel::Warning:  return "WARNING";
        case LogLevel::Error:    return "ERROR";
        case LogLevel::Critical: return "CRITICAL";
    }
    return "UNKNOWN";
}

// Modern struct with default member initializers
struct LogEntry {
    using TimePoint = std::chrono::system_clock::time_point;
    
    TimePoint timestamp{};
    LogLevel level{LogLevel::Info};
    std::string message;
    std::optional<std::string> module;  // Optional field
    
    // Default constructor with member initializers
    LogEntry() = default;
    
    // Constructor using designated initializers style
    LogEntry(TimePoint ts, LogLevel lvl, std::string msg, 
             std::optional<std::string> mod = std::nullopt)
        : timestamp(ts), level(lvl), message(std::move(msg)), module(std::move(mod)) {}
    
    // Defaulted special members (Rule of Zero when possible)
    LogEntry(const LogEntry&) = default;
    LogEntry(LogEntry&&) = default;
    LogEntry& operator=(const LogEntry&) = default;
    LogEntry& operator=(LogEntry&&) = default;
    ~LogEntry() = default;
    
    // Helper method using std::chrono
    [[nodiscard]] std::string formatTimestamp() const;
};

} // namespace LogAnalyzer
