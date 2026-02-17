#include "statistics.h"
#include <algorithm>
#include <ranges>
#include <iostream>
#include <iomanip>

namespace LogAnalyzer {

Statistics::Statistics(std::span<const LogEntry> entries)
    : entries_(entries.begin(), entries.end()) {
    // Copy the span into our vector
}

std::map<LogLevel, size_t> Statistics::getLevelCounts() const {
    std::map<LogLevel, size_t> counts;
    
    // Range-based for with structured binding
    for (const auto& entry : entries_) {
        counts[entry.level]++;
    }
    
    return counts;
}

std::vector<LogEntry> Statistics::getEntriesInRange(
    LogEntry::TimePoint start,
    LogEntry::TimePoint end) const {
    
    std::vector<LogEntry> result;
    
    // Using ranges view and filter (C++20)
    auto inRange = entries_ 
        | std::views::filter([start, end](const auto& entry) {
            return entry.timestamp >= start && entry.timestamp <= end;
          });
    
    // Copy filtered results
    std::ranges::copy(inRange, std::back_inserter(result));
    
    return result;
}

std::vector<std::pair<std::string, size_t>> Statistics::getTopModules(size_t count) const {
    // Count modules
    std::map<std::string, size_t> moduleCounts;
    
    for (const auto& entry : entries_) {
        // Using std::optional - only count if module exists
        if (entry.module.has_value()) {
            moduleCounts[*entry.module]++;
        }
    }
    
    // Convert to vector for sorting
    std::vector<std::pair<std::string, size_t>> modules(
        moduleCounts.begin(), 
        moduleCounts.end()
    );
    
    // Sort by count descending using lambda comparator
    std::ranges::sort(modules, [](const auto& a, const auto& b) {
        return a.second > b.second;  // Descending order
    });
    
    // Return top N using ranges::take_view
    if (modules.size() > count) {
        modules.resize(count);
    }
    
    return modules;
}

void Statistics::printSummary() const {
    std::cout << "\n=== Log Analysis Summary ===\n\n";
    std::cout << "Total entries: " << entries_.size() << "\n\n";
    
    // Print level counts using structured bindings
    std::cout << "Entries by level:\n";
    for (const auto& [level, count] : getLevelCounts()) {
        std::cout << "  " << std::setw(10) << std::left << toString(level) 
                  << ": " << count << "\n";
    }
    
    // Print top modules
    std::cout << "\nTop modules:\n";
    auto topModules = getTopModules(5);
    for (const auto& [module, count] : topModules) {
        std::cout << "  " << std::setw(20) << std::left << module 
                  << ": " << count << "\n";
    }
    
    // Find error entries using ranges
    auto errorCount = std::ranges::count_if(entries_, 
        [](const auto& e) { 
            return e.level == LogLevel::Error || e.level == LogLevel::Critical; 
        });
    
    std::cout << "\nCritical issues (Error/Critical): " << errorCount << "\n";
}

} // namespace LogAnalyzer
