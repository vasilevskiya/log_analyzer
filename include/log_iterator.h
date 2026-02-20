#pragma once

#include "format_policies.h"
#include "log_entry.h"
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <optional>
#include <string>

namespace LogAnalyzer {

// Forward declaration
template<FormatPolicy Policy>
class LogFile;

// Input iterator that lazily reads and parses log entries one at a time.
template<FormatPolicy Policy>
class LogIterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = LogEntry;
    using difference_type = std::ptrdiff_t;
    using pointer = const LogEntry*;
    using reference = const LogEntry&;

    // Default constructor creates a sentinel (end) iterator.
    LogIterator() = default;

    // Construct an iterator that opens the file and reads the first entry.
    explicit LogIterator(const std::filesystem::path& path)
        : stream_(std::make_shared<std::ifstream>(path)) {
        if (stream_ && stream_->is_open()) {
            advance();
        } else {
            stream_.reset();
        }
    }

    [[nodiscard]] reference operator*() const { return *current_; }
    [[nodiscard]] pointer operator->() const { return &*current_; }

    LogIterator& operator++() {
        advance();
        return *this;
    }

    LogIterator operator++(int) {
        LogIterator tmp = *this;
        advance();
        return tmp;
    }

    [[nodiscard]] friend bool operator==(const LogIterator& lhs,
                                         const LogIterator& rhs) {
        return lhs.isEnd() && rhs.isEnd();
    }

    [[nodiscard]] friend bool operator!=(const LogIterator& lhs,
                                         const LogIterator& rhs) {
        return !(lhs == rhs);
    }

private:
    std::shared_ptr<std::ifstream> stream_;
    std::optional<LogEntry> current_;

    // Read lines until Policy::parseLine succeeds or EOF.
    void advance() {
        std::string line;
        while (stream_ && std::getline(*stream_, line)) {
            if (auto entry = Policy::parseLine(line); entry.has_value()) {
                current_ = std::move(*entry);
                return;
            }
        }
        // Reached EOF or stream error — become a sentinel.
        current_.reset();
        stream_.reset();
    }

    [[nodiscard]] bool isEnd() const { return !current_.has_value(); }
};

// Represents an iterable log file — provides begin()/end() for range-based for.
template<FormatPolicy Policy>
class LogFile {
public:
    explicit LogFile(std::filesystem::path path)
        : path_(std::move(path)) {}

    [[nodiscard]] LogIterator<Policy> begin() const {
        return LogIterator<Policy>(path_);
    }

    [[nodiscard]] LogIterator<Policy> end() const {
        return LogIterator<Policy>();
    }

private:
    std::filesystem::path path_;
};

// Convenience aliases
using BracketLogFile = LogFile<BracketFormat>;
using CsvLogFile = LogFile<CsvFormat>;

} // namespace LogAnalyzer
