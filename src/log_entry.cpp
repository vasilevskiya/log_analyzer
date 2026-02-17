#include "log_entry.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

namespace LogAnalyzer {

std::optional<LogLevel> parseLogLevel(std::string_view level) {
    // Create a string for case-insensitive comparison
    std::string levelUpper;
    levelUpper.reserve(level.size());
    
    // Range-based for loop with structured binding (C++20)
    for (char c : level) {
        levelUpper += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    
    // Use if-else chain (could also use map for lookup)
    if (levelUpper == "DEBUG")    return LogLevel::Debug;
    if (levelUpper == "INFO")     return LogLevel::Info;
    if (levelUpper == "WARNING")  return LogLevel::Warning;
    if (levelUpper == "ERROR")    return LogLevel::Error;
    if (levelUpper == "CRITICAL") return LogLevel::Critical;
    
    // Return empty optional on failure
    return std::nullopt;
}

std::string LogEntry::formatTimestamp() const {
    // Modern chrono conversion
    auto time = std::chrono::system_clock::to_time_t(timestamp);
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

} // namespace LogAnalyzer
