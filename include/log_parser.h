#pragma once

#include "filter.h"
#include "format_policies.h"
#include "log_entry.h"
#include <vector>
#include <string_view>
#include <span>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace LogAnalyzer {

// Policy-based class template: the FormatPolicy controls how each line is parsed.
template<FormatPolicy Policy>
class GenericParser {
public:
    explicit GenericParser(const std::filesystem::path& logFile)
        : filePath_(logFile) {
        if (!std::filesystem::exists(filePath_)) {
            throw std::runtime_error("Log file does not exist: " + filePath_.string());
        }
    }

    // Deleted copy operations (expensive to copy), move-only type
    GenericParser(const GenericParser&) = delete;
    GenericParser& operator=(const GenericParser&) = delete;
    GenericParser(GenericParser&&) = default;
    GenericParser& operator=(GenericParser&&) = default;

    // Parse all entries - returns by value (move semantics make this efficient)
    [[nodiscard]] std::vector<LogEntry> parseAll() {
        std::vector<LogEntry> entries;
        std::ifstream file(filePath_);

        if (!file) {
            throw std::runtime_error("Cannot open file: " + filePath_.string());
        }

        if (auto size = getFileSize(); size > 0) {
            entries.reserve(size / 100);
        }

        std::string line;
        while (std::getline(file, line)) {
            if (auto entry = parseLine(line); entry.has_value()) {
                entries.push_back(std::move(*entry));
            }
        }

        return entries;
    }

    // Parse single line - static pass-through to policy
    [[nodiscard]] static std::optional<LogEntry> parseLine(std::string_view line) {
        return Policy::parseLine(line);
    }

    // Get file size using filesystem library
    [[nodiscard]] std::uintmax_t getFileSize() const {
        std::error_code ec;
        auto size = std::filesystem::file_size(filePath_, ec);
        return ec ? 0 : size;
    }

private:
    std::filesystem::path filePath_;
};

// Backward-compatible type alias
using LogParser = GenericParser<BracketFormat>;

// Backward-compatible wrapper — delegates to the generic filter() with byLevel().
[[nodiscard]] inline std::vector<LogEntry> filterByLevel(
    std::span<const LogEntry> entries,
    LogLevel level) {
    return filter(entries, byLevel(level));
}

} // namespace LogAnalyzer
