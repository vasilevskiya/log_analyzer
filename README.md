# Log Analyzer - Modern C++ Practice Project

A simple log file analyzer demonstrating modern C++ (C++11 through C++20) idioms and best practices.

## Modern C++ Features Demonstrated

### C++11 Features
- **Move Semantics**: Efficient transfer of resources (see `LogParser` move constructor)
- **Smart Pointers**: Not explicitly used here, but pattern shows RAII
- **`auto` keyword**: Type inference throughout (see `main.cpp`)
- **Range-based for loops**: Simplified iteration (everywhere)
- **`nullptr`**: Type-safe null pointer
- **Enum classes**: Type-safe enumerations (`LogLevel`)
- **Default/delete functions**: Control of special members
- **Lambda expressions**: Used in algorithms and filters
- **`constexpr`**: Compile-time evaluation (`toString`)
- **Uniform initialization**: `{}`-style initialization

### C++14 Features
- **`[[deprecated]]` attribute**: Not used, but you can mark old functions
- **Generic lambdas**: `auto` parameters in lambdas

### C++17 Features
- **`std::optional`**: Represents values that may not exist
- **`std::string_view`**: Non-owning string references (zero-copy)
- **Structured bindings**: `auto [key, value] = pair;`
- **`std::filesystem`**: Modern file operations
- **If-initializers**: `if (auto x = foo(); x.has_value())`
- **`[[nodiscard]]`**: Warn if return value ignored

### C++20 Features
- **Ranges**: `std::ranges::` algorithms and views
- **`std::span`**: Non-owning view of contiguous data
- **Concepts**: Not used here, but available for templates
- **Three-way comparison**: Not used, but available

## Building the Project

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
cmake --build .

# Run
./log_analyzer ../data/sample.log
```

### With Debug Sanitizers (Linux/macOS)
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
./log_analyzer ../data/sample.log
```

## Project Structure

```
log_analyzer/
├── CMakeLists.txt          # Modern CMake configuration
├── include/                # Header files
│   ├── log_entry.h        # Data structure with enum class, optional
│   ├── log_parser.h       # Parser with filesystem, span
│   └── statistics.h       # Analytics with ranges, algorithms
├── src/                   # Implementation files
│   ├── log_entry.cpp
│   ├── log_parser.cpp
│   ├── statistics.cpp
│   └── main.cpp
└── data/                  # Sample data
    └── sample.log
```

## Key Concepts to Study

1. **RAII (Resource Acquisition Is Initialization)**
   - Resources tied to object lifetime
   - See `LogParser` with file handling

2. **Move Semantics**
   - `std::move()` transfers ownership
   - Move-only types (deleted copy constructor)
   - Return value optimization (RVO)

3. **Value Semantics vs Reference Semantics**
   - `std::string_view` - non-owning reference
   - `std::span` - non-owning view of arrays
   - When to copy vs reference

4. **Optional Values**
   - `std::optional<T>` for values that might not exist
   - Replaces null pointers or sentinel values
   - Check with `has_value()` or boolean conversion

5. **Modern Algorithms**
   - `std::ranges::` prefix for range-based algorithms
   - Views (lazy evaluation): `std::views::filter`, `std::views::take`
   - Easier to read and compose

6. **Filesystem Library**
   - Type-safe path handling
   - Platform-independent file operations
   - Error handling with `std::error_code`

## Exercises to Extend This Project

1. **Add Smart Pointers**: Create a caching system with `std::unique_ptr`
2. **Add Templates**: Make the parser generic for different log formats
3. **Add Concepts**: Constrain templates (C++20)
4. **Add Coroutines**: Async log parsing (C++20)
5. **Add Modules**: Convert to C++20 modules instead of headers
6. **Add unit tests**: Practice modern testing with Catch2 or Google Test

## Differences from Old C++

| Old C++ (pre-2011) | Modern C++ |
|--------------------|------------|
| Raw pointers | Smart pointers (`unique_ptr`, `shared_ptr`) |
| Manual loops | Range-based for, algorithms, ranges |
| `NULL` | `nullptr` |
| Error codes | Exceptions, `std::optional`, `std::expected` |
| String copies | `std::string_view` for read-only |
| `typedef` | `using` (clearer syntax) |
| Manual resource management | RAII everywhere |
| Verbose iterators | `auto`, structured bindings |

## Common Pitfalls

1. **Don't use `new`/`delete` directly** - Use smart pointers or containers
2. **Prefer `auto`** - But not for everything; clarity matters
3. **Use `std::move` carefully** - Only when you won't use the object again
4. **`string_view` doesn't own data** - Don't return from functions that create temporaries
5. **Ranges are lazy** - Views don't copy; ensure underlying data lives long enough

## Next Steps

1. Read through all the code and comments
2. Build and run the project
3. Modify it: add features, change parsing logic
4. Try the exercises above
5. Look at real-world modern C++ projects on GitHub
