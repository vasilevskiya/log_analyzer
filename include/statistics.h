#pragma once

#include "log_entry.h"
#include <vector>
#include <map>
#include <span>

namespace LogAnalyzer {

// Statistics aggregator using modern containers and algorithms
class Statistics {
public:
    // Constructor from span (can accept vector, array, etc.)
    explicit Statistics(std::span<const LogEntry> entries);
    
    // Get count by level - returns map
    [[nodiscard]] std::map<LogLevel, size_t> getLevelCounts() const;
    
    // Get entries in time range using std::chrono
    [[nodiscard]] std::vector<LogEntry> getEntriesInRange(
        LogEntry::TimePoint start,
        LogEntry::TimePoint end
    ) const;
    
    // Get most common modules - demonstrates structured bindings return
    [[nodiscard]] std::vector<std::pair<std::string, size_t>> getTopModules(size_t count = 5) const;
    
    // Print summary - demonstrates ranges and modern algorithms
    void printSummary() const;
    
private:
    std::vector<LogEntry> entries_;
};

} // namespace LogAnalyzer
