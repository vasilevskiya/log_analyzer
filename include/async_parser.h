#pragma once

#include "format_policies.h"
#include "log_parser.h"
#include <future>
#include <vector>
#include <filesystem>

namespace LogAnalyzer {

// Async wrapper around GenericParser — launches parsing on a separate thread.
template<FormatPolicy Policy>
class GenericAsyncParser {
public:
    // Parse a single file asynchronously; returns a future with the entries.
    [[nodiscard]] std::future<std::vector<LogEntry>>
    parseAsync(const std::filesystem::path& logFile) {
        return std::async(std::launch::async, [logFile] {
            GenericParser<Policy> parser(logFile);
            return parser.parseAll();
        });
    }

    // Parse multiple files concurrently; returns one future per file.
    [[nodiscard]] std::vector<std::future<std::vector<LogEntry>>>
    parseMultiple(const std::vector<std::filesystem::path>& files) {
        std::vector<std::future<std::vector<LogEntry>>> futures;
        futures.reserve(files.size());
        for (const auto& file : files) {
            futures.push_back(parseAsync(file));
        }
        return futures;
    }
};

// Backward-compatible alias
using AsyncLogParser = GenericAsyncParser<BracketFormat>;

} // namespace LogAnalyzer
