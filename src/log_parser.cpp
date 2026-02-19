#include "log_parser.h"
#include <algorithm>

namespace LogAnalyzer {

std::vector<LogEntry> filterByLevel(std::span<const LogEntry> entries, LogLevel level) {
    std::vector<LogEntry> result;

    // Using std::ranges::copy_if with lambda (C++20)
    std::ranges::copy_if(entries, std::back_inserter(result),
        [level](const auto& entry) { return entry.level == level; });

    return result;
}

} // namespace LogAnalyzer
