#pragma once

#include "log_entry.h"
#include <algorithm>
#include <concepts>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace LogAnalyzer {

// Generic filter: works with any range whose elements satisfy the predicate.
template<std::ranges::range Container, typename Predicate>
    requires std::predicate<Predicate, std::ranges::range_value_t<Container>>
auto filter(const Container& c, Predicate pred) {
    std::vector<std::ranges::range_value_t<Container>> result;
    std::ranges::copy_if(c, std::back_inserter(result), pred);
    return result;
}

// --- Predicate factories for common LogEntry filters ---

// Returns a predicate that matches entries with the given level.
inline auto byLevel(LogLevel level) {
    return [level](const LogEntry& e) { return e.level == level; };
}

// Returns a predicate that matches entries whose module equals the given name.
inline auto byModule(std::string_view module) {
    return [m = std::string(module)](const LogEntry& e) {
        return e.module.has_value() && *e.module == m;
    };
}

// Returns a predicate that matches entries after the given time point.
inline auto afterTime(LogEntry::TimePoint cutoff) {
    return [cutoff](const LogEntry& e) { return e.timestamp > cutoff; };
}

// Returns a predicate that matches entries before the given time point.
inline auto beforeTime(LogEntry::TimePoint cutoff) {
    return [cutoff](const LogEntry& e) { return e.timestamp < cutoff; };
}

} // namespace LogAnalyzer
