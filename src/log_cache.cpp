#include "log_cache.h"
#include "log_parser.h"

namespace LogAnalyzer {

std::span<const LogEntry> LogCache::getEntries(const std::filesystem::path& logFile) {
    auto modTime = std::filesystem::last_write_time(logFile);

    auto it = cache_.find(logFile);
    if (it != cache_.end() && it->second.lastModified == modTime) {
        // Cache hit — return non-owning view
        return *it->second.entries;
    }

    // Cache miss or stale — parse and store
    LogParser parser(logFile);
    auto parsed = parser.parseAll();

    auto entries = std::make_unique<std::vector<LogEntry>>(std::move(parsed));

    auto& stored = cache_[logFile] = CacheEntry{std::move(entries), modTime};

    return *stored.entries;
}

void LogCache::evict(const std::filesystem::path& logFile) {
    cache_.erase(logFile);
}

void LogCache::clear() {
    cache_.clear();
}

size_t LogCache::size() const {
    return cache_.size();
}

} // namespace LogAnalyzer
