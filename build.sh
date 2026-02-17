#!/bin/bash
# Build script for macOS/Linux

set -e  # Exit on error

echo "Attempting to build log_analyzer..."

# Compiler options to try (in order of preference for macOS)
COMPILERS=("clang++" "g++-14" "g++-13" "g++-12" "g++")
FLAGS="-std=c++20 -Wall -Wextra -Iinclude"
SOURCES="src/*.cpp"
OUTPUT="log_analyzer"

for compiler in "${COMPILERS[@]}"; do
    if command -v "$compiler" &> /dev/null; then
        echo "Trying $compiler..."
        if $compiler $FLAGS -o $OUTPUT $SOURCES 2>/dev/null; then
            echo "✓ Successfully built with $compiler"
            echo "Run with: ./$OUTPUT data/sample.log"
            exit 0
        fi
    fi
done

echo "❌ Failed to build with any available compiler."
echo ""
echo "Please try manually:"
echo "  clang++ -std=c++20 -Wall -Wextra -Iinclude -o log_analyzer src/*.cpp"
echo ""
echo "Or install a newer compiler:"
echo "  brew install llvm"
echo "  /opt/homebrew/opt/llvm/bin/clang++ -std=c++20 -Wall -Wextra -Iinclude -o log_analyzer src/*.cpp"
exit 1
