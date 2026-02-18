#include "log_cache.h"
#include "log_parser.h"
#include "statistics.h"
#include <iostream>
#include <filesystem>
#include <memory>
#include <ranges>  // For std::views

using namespace LogAnalyzer;
namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    try {
        // Use string_view for command-line args
        if (argc < 2) {
            std::cerr << "Usage: " << argv[0] << " <log_file>\n";
            return 1;
        }

        fs::path logFile = argv[1];

        // Check if file exists using filesystem library
        if (!fs::exists(logFile)) {
            std::cerr << "Error: File '" << logFile << "' does not exist.\n";
            return 1;
        }

        std::cout << "Analyzing log file: " << logFile << "\n";
        std::cout << "File size: " << fs::file_size(logFile) << " bytes\n";

        // Use LogCache with std::unique_ptr ownership of parsed data
        LogCache cache;

        // First call parses the file and caches the result
        auto entries = cache.getEntries(logFile);

        std::cout << "Parsed " << entries.size() << " log entries.\n";

        // Second call demonstrates cache hit (no re-parsing)
        auto entriesAgain = cache.getEntries(logFile);
        std::cout << "Cache hit: " << (entries.data() == entriesAgain.data() ? "yes" : "no")
                  << " (" << cache.size() << " file(s) cached)\n";
        
        if (entries.empty()) {
            std::cout << "No valid entries found.\n";
            return 0;
        }
        
        // Create statistics using span (non-owning view)
        Statistics stats(entries);
        stats.printSummary();
        
        // Demonstrate filtering with std::span
        std::cout << "\n=== Error Entries ===\n";
        auto errors = filterByLevel(entries, LogLevel::Error);
        
        // Range-based for with structured binding and auto
        for (const auto& error : errors | std::views::take(5)) {
            std::cout << "[" << error.formatTimestamp() << "] ";
            if (error.module) {  // Check optional
                std::cout << "[" << *error.module << "] ";
            }
            std::cout << error.message << "\n";
        }
        
        if (errors.size() > 5) {
            std::cout << "... and " << (errors.size() - 5) << " more errors.\n";
        }
        
        // Demonstrate time-based filtering
        if (!entries.empty()) {
            auto startTime = entries.front().timestamp;
            auto endTime = entries.back().timestamp;
            
            // Calculate midpoint using chrono
            auto duration = endTime - startTime;
            auto midTime = startTime + duration / 2;
            
            auto recentEntries = stats.getEntriesInRange(midTime, endTime);
            std::cout << "\nEntries in second half: " << recentEntries.size() << "\n";
        }
        
    } catch (const std::exception& ex) {
        // Modern exception handling
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    
    return 0;
}
