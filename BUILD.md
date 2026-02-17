# Building the Log Analyzer

Multiple ways to build this project, from simple to full-featured.

## Option 1: Quick Build (g++ directly)

Fastest way to get started:

```bash
cd log_analyzer
g++ -std=c++20 -Wall -Wextra -Iinclude -o log_analyzer src/*.cpp

# Run it
./log_analyzer data/sample.log
```

### With optimizations:
```bash
g++ -std=c++20 -O3 -Wall -Wextra -Iinclude -o log_analyzer src/*.cpp
```

### With debug symbols and sanitizers:
```bash
g++ -std=c++20 -g -O0 -Wall -Wextra -fsanitize=address,undefined \
    -Iinclude -o log_analyzer src/*.cpp

./log_analyzer data/sample.log
```

## Option 2: CMake Build (Recommended)

Professional build system, works across platforms.

### Install CMake (if needed):
```bash
# Ubuntu/Debian
sudo apt install cmake

# macOS
brew install cmake

# Or download from https://cmake.org/download/
```

### Build:
```bash
cd log_analyzer
mkdir build && cd build
cmake ..
cmake --build .

# Run
./log_analyzer ../data/sample.log
```

### Build types:
```bash
# Debug (with sanitizers on Linux/Mac)
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# Release (optimized)
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .

# With verbose output
cmake --build . --verbose
```

### Generate compile_commands.json (for IDE/tools):
```bash
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
```

## Option 3: Makefile Build

Traditional Unix build (create your own Makefile):

```makefile
# Makefile
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Iinclude
LDFLAGS = 

# Debug build
DEBUGFLAGS = -g -O0 -fsanitize=address,undefined

# Release build
RELEASEFLAGS = -O3 -DNDEBUG

SOURCES = src/main.cpp src/log_parser.cpp src/log_entry.cpp src/statistics.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = log_analyzer

.PHONY: all clean debug release

all: release

debug: CXXFLAGS += $(DEBUGFLAGS)
debug: LDFLAGS += $(DEBUGFLAGS)
debug: $(TARGET)

release: CXXFLAGS += $(RELEASEFLAGS)
release: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET) data/sample.log
```

Then:
```bash
make          # Build release
make debug    # Build debug
make run      # Build and run
make clean    # Clean
```

## Compiler Support

### Minimum Requirements:
- **GCC**: 10+ (for C++20 ranges)
- **Clang**: 13+ (for C++20 ranges)  
- **MSVC**: Visual Studio 2019 16.10+ (for C++20 ranges)

### Check your compiler version:
```bash
g++ --version
clang++ --version
cl          # On Windows
```

### Install newer compiler (Ubuntu example):
```bash
# GCC 13
sudo apt install g++-13
g++-13 -std=c++20 ...

# Or use Clang
sudo apt install clang-15
clang++-15 -std=c++20 ...
```

## Platform-Specific Notes

### Linux
Works out of the box with modern GCC/Clang:
```bash
g++ -std=c++20 -Wall -Wextra -Iinclude -o log_analyzer src/*.cpp
```

### macOS
Use Homebrew's LLVM for best C++20 support:
```bash
brew install llvm
/opt/homebrew/opt/llvm/bin/clang++ -std=c++20 -Wall -Wextra -Iinclude \
    -o log_analyzer src/*.cpp
```

### Windows (MinGW)
```bash
g++ -std=c++20 -Wall -Wextra -Iinclude -o log_analyzer.exe src/*.cpp
```

### Windows (MSVC)
```bash
cl /std:c++20 /EHsc /W4 /I include src/*.cpp
```

Or use CMake with Visual Studio:
```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

## Development Setup

### VS Code
Install extensions:
- C/C++ (Microsoft)
- CMake Tools (Microsoft)
- clangd (for better C++ support)

Then use `tasks.json`:
```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build",
            "type": "shell",
            "command": "g++",
            "args": [
                "-std=c++20",
                "-Wall",
                "-Wextra",
                "-Iinclude",
                "-g",
                "src/*.cpp",
                "-o",
                "log_analyzer"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            }
        }
    ]
}
```

### CLion
Open the project folder - CMakeLists.txt is automatically detected.

### Visual Studio
Open folder or generate VS project:
```bash
cmake .. -G "Visual Studio 17 2022"
# Then open .sln file
```

## Troubleshooting

### "ranges not found" or similar
- Update compiler to GCC 10+, Clang 13+, or MSVC 2019 16.10+
- Some systems need: `#include <ranges>` and link with `-lstdc++`

### "filesystem not found"
- GCC 8 needs: `-lstdc++fs`
- GCC 9+ includes it by default

### Linker errors on Linux
```bash
# If you see undefined references to filesystem
g++ ... -lstdc++fs

# For threading
g++ ... -pthread
```

### macOS "cannot find <filesystem>"
```bash
# Use newer clang from Homebrew
brew install llvm
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
```

### Windows path separators
The code uses `std::filesystem` which handles Windows paths automatically.

## Performance Tips

### Compile with optimizations:
```bash
g++ -std=c++20 -O3 -march=native -Iinclude -o log_analyzer src/*.cpp
```

### Link-Time Optimization (LTO):
```bash
g++ -std=c++20 -O3 -flto -Iinclude -o log_analyzer src/*.cpp
```

### Profile-Guided Optimization:
```bash
# 1. Compile with profiling
g++ -std=c++20 -O3 -fprofile-generate -Iinclude -o log_analyzer src/*.cpp

# 2. Run with representative data
./log_analyzer large.log

# 3. Recompile with profile data
g++ -std=c++20 -O3 -fprofile-use -Iinclude -o log_analyzer src/*.cpp
```

## Testing Your Build

Create a test script:
```bash
#!/bin/bash
# test.sh

echo "Building..."
g++ -std=c++20 -Wall -Wextra -Werror -Iinclude -o log_analyzer src/*.cpp

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Running tests..."
    ./log_analyzer data/sample.log
    echo "Exit code: $?"
else
    echo "Build failed!"
    exit 1
fi
```

Run it:
```bash
chmod +x test.sh
./test.sh
```

## Creating Larger Test Files

Generate a bigger log file for testing:
```bash
# Repeat the sample 1000 times
for i in {1..1000}; do 
    cat data/sample.log >> data/large.log
done

# Test performance
time ./log_analyzer data/large.log
```

## Next Steps

After building successfully:
1. Try modifying the code
2. Add features from EXERCISES.md
3. Profile the code (`perf` on Linux, Instruments on macOS)
4. Try different compilers and compare
5. Experiment with compiler flags and optimizations
