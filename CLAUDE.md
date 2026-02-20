# Log Analyzer - Project Context

## Overview
A C++ log analyzer project used for learning/practicing modern C++ features (C++20).
Parses log files, filters entries, and computes statistics.

## Build
- **Build system:** Bazel (CMake was removed)
- **C++ standard:** C++20 (`--cxxopt=-std=c++20` in .bazelrc)
- **Build command:** `bazel build //...`
- **Test command:** `bazel test //...`
- **Quick build script:** `./build.sh`

## Project Structure
- `include/` — Headers (log_entry.h, statistics.h)
- `src/` — Implementation files
- `tests/` — Google Test unit tests
- `data/sample.log` — Sample log data
- `EXERCISES.md` — Practice exercises for modern C++ features

## Key Features Implemented
- LogEntry struct with std::chrono timestamps, std::optional fields
- LogLevel enum class with string conversion
- LogCache with std::unique_ptr ownership
- GenericParser template with concept constraints
- Generic filter() template
- Async log parsing with std::async/std::future
- Namespace: `LogAnalyzer`

## Conventions
- Use modern C++ idioms (smart pointers, std::optional, concepts, etc.)
- Follow Rule of Zero where possible
- Use [[nodiscard]] for functions with important return values
- PR workflow: feature branches merged via GitHub PRs
