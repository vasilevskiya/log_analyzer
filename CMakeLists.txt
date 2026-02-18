cmake_minimum_required(VERSION 3.20)
project(LogAnalyzer VERSION 1.0.0 LANGUAGES CXX)

# Modern C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Main executable
add_executable(log_analyzer
    src/main.cpp
    src/log_cache.cpp
    src/log_parser.cpp
    src/log_entry.cpp
    src/statistics.cpp
)

target_include_directories(log_analyzer PRIVATE include)

# Enable sanitizers in debug mode (optional but recommended)
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT MSVC)
    target_compile_options(log_analyzer PRIVATE -fsanitize=address,undefined)
    target_link_options(log_analyzer PRIVATE -fsanitize=address,undefined)
endif()
