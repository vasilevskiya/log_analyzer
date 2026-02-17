# Practical Exercises - Modern C++ Practice

Work through these exercises to practice modern C++ features. Each builds on the log analyzer project.

## Exercise 1: Add Smart Pointers and Caching

**Goal:** Practice `std::unique_ptr`, `std::shared_ptr`, and move semantics

**Task:** Add a caching mechanism for parsed log entries.

```cpp
// In log_parser.h, add:
class LogCache {
public:
    // Store unique ownership of entries
    void addEntry(std::unique_ptr<LogEntry> entry);
    
    // Return shared ownership for multiple users
    std::shared_ptr<LogEntry> getEntry(size_t index) const;
    
    // Clear cache (practice move semantics)
    std::vector<std::unique_ptr<LogEntry>> takeAll();
    
private:
    std::vector<std::unique_ptr<LogEntry>> cache_;
};
```

**Learning points:**
- When to use `unique_ptr` vs `shared_ptr`
- How `std::move` transfers ownership
- RAII for resource management

---

## Exercise 2: Template Functions for Filtering

**Goal:** Practice templates and generic programming

**Task:** Create generic filter functions that work with any predicate.

```cpp
// In a new file filter.h:
template<typename Container, typename Predicate>
auto filter(const Container& c, Predicate pred) {
    Container result;
    std::ranges::copy_if(c, std::back_inserter(result), pred);
    return result;
}

// Usage:
auto errors = filter(entries, 
    [](const LogEntry& e) { return e.level == LogLevel::Error; });

auto recent = filter(entries,
    [cutoff](const LogEntry& e) { return e.timestamp > cutoff; });
```

**Extension:** Add C++20 concepts to constrain the template:
```cpp
template<std::ranges::range Container, typename Predicate>
requires std::predicate<Predicate, std::ranges::range_value_t<Container>>
auto filter(const Container& c, Predicate pred) {
    // ...
}
```

**Learning points:**
- Template syntax and usage
- Generic programming patterns
- C++20 concepts (optional)

---

## Exercise 3: Async Log Parsing with std::async

**Goal:** Practice concurrency with futures and async

**Task:** Parse large log files in parallel.

```cpp
// In log_parser.h:
#include <future>

class AsyncLogParser {
public:
    // Start async parse, return future
    std::future<std::vector<LogEntry>> parseAsync(const fs::path& file);
    
    // Parse multiple files concurrently
    std::vector<std::future<std::vector<LogEntry>>> 
        parseMultiple(const std::vector<fs::path>& files);
};
```

**Learning points:**
- `std::async` and `std::future`
- Thread safety considerations
- When to use concurrency

---

## Exercise 4: Custom Iterator for Log Files

**Goal:** Practice iterators and the iterator protocol

**Task:** Create an iterator that lazily reads log entries one at a time.

```cpp
// In log_iterator.h:
class LogIterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = LogEntry;
    using difference_type = std::ptrdiff_t;
    using pointer = const LogEntry*;
    using reference = const LogEntry&;
    
    // Iterator operations
    LogIterator& operator++();
    LogEntry operator*() const;
    bool operator==(const LogIterator& other) const;
    
private:
    std::ifstream stream_;
    std::optional<LogEntry> current_;
};

// Usage:
for (const auto& entry : LogFile("data.log")) {
    // Process one entry at a time, memory efficient!
}
```

**Learning points:**
- Iterator protocol and concepts
- Lazy evaluation patterns
- Resource-efficient processing

---

## Exercise 5: Visitor Pattern with std::variant

**Goal:** Practice `std::variant` and `std::visit`

**Task:** Create different log formatters using variants.

```cpp
// In formatter.h:
#include <variant>

struct JsonFormatter { /* ... */ };
struct XmlFormatter { /* ... */ };
struct PlainTextFormatter { /* ... */ };

using Formatter = std::variant<JsonFormatter, XmlFormatter, PlainTextFormatter>;

class LogFormatter {
public:
    explicit LogFormatter(Formatter fmt) : formatter_(std::move(fmt)) {}
    
    std::string format(const LogEntry& entry) {
        return std::visit([&entry](auto& formatter) {
            return formatter.format(entry);
        }, formatter_);
    }
    
private:
    Formatter formatter_;
};
```

**Learning points:**
- Type-safe unions with `std::variant`
- Visitor pattern with `std::visit`
- Generic lambdas with `auto`

---

## Exercise 6: Custom Allocator

**Goal:** Practice allocators and memory management

**Task:** Create a pool allocator for log entries to reduce allocation overhead.

```cpp
// In pool_allocator.h:
template<typename T>
class PoolAllocator {
public:
    using value_type = T;
    
    PoolAllocator(size_t poolSize = 1024);
    
    T* allocate(size_t n);
    void deallocate(T* p, size_t n);
    
private:
    std::vector<T> pool_;
    size_t nextFree_ = 0;
};

// Usage:
std::vector<LogEntry, PoolAllocator<LogEntry>> entries;
```

**Learning points:**
- Custom allocators
- Memory pools
- Performance optimization

---

## Exercise 7: Pipeline with Ranges

**Goal:** Master C++20 ranges and views

**Task:** Build a processing pipeline for log analysis.

```cpp
// Create a pipeline that:
auto pipeline = entries
    | std::views::filter([](const auto& e) { 
        return e.level >= LogLevel::Warning; 
      })
    | std::views::transform([](const auto& e) {
        return std::pair{e.timestamp, e.message};
      })
    | std::views::take(10);

// Process pipeline
for (const auto& [time, msg] : pipeline) {
    std::cout << formatTime(time) << ": " << msg << "\n";
}
```

**Extension:** Create custom view adaptors:
```cpp
auto in_module(std::string_view module) {
    return std::views::filter([module](const auto& e) {
        return e.module && *e.module == module;
    });
}

// Usage:
auto network_errors = entries 
    | in_module("Network") 
    | in_level(LogLevel::Error);
```

**Learning points:**
- Lazy evaluation with views
- Composable transformations
- Custom view adaptors

---

## Exercise 8: Error Handling with std::expected (C++23)

**Goal:** Practice modern error handling without exceptions

**Task:** Refactor parsing to use `std::expected` instead of throwing exceptions.

```cpp
// In C++23 or using a library like expected-lite:
#include <expected>

std::expected<LogEntry, ParseError> parseLine(std::string_view line);

// Usage:
auto result = parseLine(line);
if (result) {
    LogEntry entry = *result;
    // Use entry
} else {
    ParseError error = result.error();
    // Handle error
}

// Or with monadic operations:
auto entry = parseLine(line)
    .and_then([](LogEntry e) { return validate(e); })
    .or_else([](ParseError e) { return getDefault(); });
```

**Learning points:**
- `std::expected` for error handling
- Monadic error handling
- Alternatives to exceptions

---

## Exercise 9: Compile-Time String Processing

**Goal:** Practice `constexpr` and compile-time programming

**Task:** Create compile-time log level parsing.

```cpp
// In log_level.h:
constexpr LogLevel parseLogLevel(std::string_view str) {
    if (str == "DEBUG") return LogLevel::Debug;
    if (str == "INFO") return LogLevel::Info;
    // ...
    throw std::invalid_argument("Invalid log level");
}

// Usage at compile time:
constexpr auto level = parseLogLevel("ERROR");
static_assert(level == LogLevel::Error);

// Or create a user-defined literal:
constexpr LogLevel operator""_loglevel(const char* str, size_t len) {
    return parseLogLevel(std::string_view(str, len));
}

// Usage:
auto level = "ERROR"_loglevel;  // Compile-time conversion!
```

**Learning points:**
- `constexpr` functions
- Compile-time computation
- User-defined literals

---

## Exercise 10: Coroutines for Async Streaming

**Goal:** Practice C++20 coroutines

**Task:** Create an async log stream using coroutines.

```cpp
// In async_log_stream.h:
#include <coroutine>
#include <generator>  // C++23 or use a library

std::generator<LogEntry> streamLogFile(const fs::path& file) {
    std::ifstream f(file);
    std::string line;
    
    while (std::getline(f, line)) {
        if (auto entry = parseLine(line)) {
            co_yield *entry;
        }
    }
}

// Usage:
for (const auto& entry : streamLogFile("huge.log")) {
    // Process one at a time, async-friendly
    processEntry(entry);
}
```

**Learning points:**
- Coroutine basics
- Generator pattern
- Lazy async computation

---

## Challenge Project: Complete Log Analysis Tool

Combine multiple exercises to create a full-featured tool:

**Features:**
1. Multi-threaded parsing (`std::async`)
2. Custom allocator for performance
3. Range-based filtering and analysis
4. Multiple output formats (`std::variant` visitors)
5. Streaming large files (iterators/coroutines)
6. Comprehensive error handling (`std::expected`)

**Advanced:**
- Add C++20 modules instead of headers
- Use concepts to constrain templates
- Create a DSL for log queries using templates
- Add benchmark comparisons with old C++ style

---

## Tips for Working Through Exercises

1. **Start small**: Get it working with basic C++, then modernize
2. **Compile frequently**: Modern C++ error messages can be verbose
3. **Use cppreference.com**: Best resource for modern C++ features
4. **Enable warnings**: `-Wall -Wextra -Wpedantic`
5. **Try different compilers**: GCC, Clang, MSVC - each teaches you something
6. **Profile**: Modern C++ is often faster, but verify with benchmarks

## Next Steps After These Exercises

1. Read "Effective Modern C++" by Scott Meyers
2. Study the C++ Core Guidelines
3. Contribute to an open-source C++ project
4. Solve problems on Exercism or LeetCode using modern C++
5. Build a real project that interests you
