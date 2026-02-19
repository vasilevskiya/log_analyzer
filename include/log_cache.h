#pragma once

#include "log_parser.h"
#include <vector>
#include <map>
#include <span>
#include <memory>
#include <filesystem>

namespace LogAnalyzer {

// Policy-based caching template: uses GenericParser<Policy> internally.
template<FormatPolicy Policy>
class GenericLogCache {
public:
    GenericLogCache() = default;

    // Move-only (owns unique_ptrs)
    GenericLogCache(const GenericLogCache&) = delete;
    GenericLogCache& operator=(const GenericLogCache&) = delete;
    GenericLogCache(GenericLogCache&&) = default;
    GenericLogCache& operator=(GenericLogCache&&) = default;

    // Get entries for a file — parses on first call, returns cached on subsequent calls.
    // Invalidates cache if file has been modified since last parse.
    std::span<const LogEntry> getEntries(const std::filesystem::path& logFile) {
        auto modTime = std::filesystem::last_write_time(logFile);

        auto it = cache_.find(logFile);
        if (it != cache_.end() && it->second.lastModified == modTime) {
            return *it->second.entries;
        }

        GenericParser<Policy> parser(logFile);
        auto parsed = parser.parseAll();

        auto entries = std::make_unique<std::vector<LogEntry>>(std::move(parsed));

        auto& stored = cache_[logFile] = CacheEntry{std::move(entries), modTime};

        return *stored.entries;
    }

    // Evict a specific file from cache
    void evict(const std::filesystem::path& logFile) {
        cache_.erase(logFile);
    }

    // Clear entire cache
    void clear() {
        cache_.clear();
    }

    // Number of cached files
    [[nodiscard]] size_t size() const {
        return cache_.size();
    }

private:
    struct CacheEntry {
        std::unique_ptr<std::vector<LogEntry>> entries;
        std::filesystem::file_time_type lastModified;
    };
    std::map<std::filesystem::path, CacheEntry> cache_;
};

// Backward-compatible type alias
using LogCache = GenericLogCache<BracketFormat>;

} // namespace LogAnalyzer
