# Building on macOS

## Quick Fix

The error you're seeing means g++ can't find the C++ standard library. On macOS, use **clang++** instead:

```bash
cd /Users/sasha/dev/cpptest/log_analyzer
clang++ -std=c++20 -Wall -Wextra -Iinclude -o log_analyzer src/*.cpp
./log_analyzer data/sample.log
```

## Or Use the Build Script

```bash
chmod +x build.sh
./build.sh
```

## Why This Happens

On macOS, `g++` is often just a symlink to `clang++`, but sometimes it's configured incorrectly or pointing to an incomplete installation. The solution is to use `clang++` directly, which is Apple's recommended compiler.

## If You Want Real GCC

If you specifically want to use actual GCC (not Apple's clang), install via Homebrew:

```bash
# Install GCC
brew install gcc

# Use it (version numbers may vary)
g++-13 -std=c++20 -Wall -Wextra -Iinclude -o log_analyzer src/*.cpp
# or
g++-14 -std=c++20 -Wall -Wextra -Iinclude -o log_analyzer src/*.cpp
```

## Recommended: Use Homebrew LLVM (Best C++20 Support)

For the best modern C++ support on macOS:

```bash
# Install LLVM
brew install llvm

# Use it
/opt/homebrew/opt/llvm/bin/clang++ -std=c++20 -Wall -Wextra -Iinclude -o log_analyzer src/*.cpp
```

## Verify Your Compiler

Check what you have:

```bash
# Check clang++ version
clang++ --version

# Check if Homebrew GCC is installed
brew list | grep gcc

# Find all C++ compilers
which -a g++ clang++
```

## Bottom Line for Now

**Just use this command:**
```bash
clang++ -std=c++20 -Wall -Wextra -Iinclude -o log_analyzer src/*.cpp
```

It will work immediately on any Mac with Xcode Command Line Tools installed.
