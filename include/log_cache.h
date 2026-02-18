#pragma once

#include "log_entry.h"
#include <vector>
#include <map>
#include <span>
#include <memory>
#include <filesystem>

namespace LogAnalyzer {

// Caching system using std::unique_ptr to manage ownership of parsed log data.
// Demonstrates smart pointer usage: unique_ptr owns the cached vectors,
// while callers receive non-owning std::span views.
class LogCache {
public:
    LogCache() = default;

    // Move-only (owns unique_ptrs)
    LogCache(const LogCache&) = delete;
    LogCache& operator=(const LogCache&) = delete;
    LogCache(LogCache&&) = default;
    LogCache& operator=(LogCache&&) = default;

    // Get entries for a file — parses on first call, returns cached on subsequent calls.
    // Invalidates cache if file has been modified since last parse.
    std::span<const LogEntry> getEntries(const std::filesystem::path& logFile);

    // Evict a specific file from cache
    void evict(const std::filesystem::path& logFile);

    // Clear entire cache
    void clear();

    // Number of cached files
    [[nodiscard]] size_t size() const;

private:
    struct CacheEntry {
        std::unique_ptr<std::vector<LogEntry>> entries;
        std::filesystem::file_time_type lastModified;
    };
    std::map<std::filesystem::path, CacheEntry> cache_;
};

} // namespace LogAnalyzer
