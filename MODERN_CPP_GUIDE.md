# Modern C++ Learning Guide - For C++ Developers Returning After 2014

## Quick Reference: What's Changed

### Memory Management Revolution
**Then (2001-2014):**
```cpp
// Manual memory management
Widget* widget = new Widget();
// ... use widget
delete widget;

// Manual array management
int* arr = new int[100];
// ... use arr
delete[] arr;
```

**Now (2014+):**
```cpp
// Smart pointers handle cleanup automatically
auto widget = std::make_unique<Widget>();  // Single owner
auto shared = std::make_shared<Widget>();   // Multiple owners
// Automatic deletion when out of scope

// Use std::vector instead of raw arrays
std::vector<int> arr(100);
// Automatic cleanup, bounds checking, etc.
```

### Move Semantics - The Game Changer
**New Concept:** Objects can be "moved" instead of copied, transferring ownership efficiently.

```cpp
// Returning large objects is now cheap!
std::vector<int> createLargeVector() {
    std::vector<int> v(1000000);
    return v;  // Moved, not copied! (RVO or move)
}

// Move-only types
std::unique_ptr<int> p1 = std::make_unique<int>(42);
std::unique_ptr<int> p2 = std::move(p1);  // Ownership transferred
// p1 is now empty, p2 owns the int
```

### Type Inference with `auto`
**Then:**
```cpp
std::vector<int>::iterator it = vec.begin();
std::map<std::string, std::vector<int>>::const_iterator mit = m.find("key");
```

**Now:**
```cpp
auto it = vec.begin();        // Compiler deduces type
auto mit = m.find("key");     // Much cleaner!

// But don't overuse - clarity matters
int count = 42;               // Clear
auto count = 42;              // Less clear what type this is
```

### Range-Based For Loops
**Then:**
```cpp
for (std::vector<int>::iterator it = vec.begin(); 
     it != vec.end(); ++it) {
    std::cout << *it << "\n";
}
```

**Now:**
```cpp
for (const auto& item : vec) {
    std::cout << item << "\n";
}

// Or with structured bindings (C++17)
std::map<std::string, int> m;
for (const auto& [key, value] : m) {
    std::cout << key << ": " << value << "\n";
}
```

### Lambda Expressions
**Then:**
```cpp
// Had to define separate function or functor class
struct Comparator {
    bool operator()(int a, int b) const { return a > b; }
};
std::sort(vec.begin(), vec.end(), Comparator());
```

**Now:**
```cpp
// Inline anonymous functions
std::sort(vec.begin(), vec.end(), 
    [](int a, int b) { return a > b; });

// Can capture variables from enclosing scope
int threshold = 42;
auto filtered = std::ranges::copy_if(vec, result.begin(),
    [threshold](int x) { return x > threshold; });
```

### std::optional - No More Null Pointers for "Maybe" Values
**Then:**
```cpp
// Used nullptr or sentinel values
Widget* findWidget(int id) {
    // ... search
    return nullptr;  // if not found
}

Widget* w = findWidget(42);
if (w != nullptr) {
    w->doSomething();
}
```

**Now:**
```cpp
std::optional<Widget> findWidget(int id) {
    // ... search
    if (found) {
        return widget;
    }
    return std::nullopt;
}

if (auto w = findWidget(42); w.has_value()) {
    w->doSomething();
}
// Or more concisely
if (auto w = findWidget(42)) {  // optional converts to bool
    w->doSomething();
}
```

### String View - Zero-Copy String References
**Then:**
```cpp
void processString(const std::string& str) {
    // What if caller has a char* or substring?
    // They'd have to create a std::string (allocation!)
}
```

**Now:**
```cpp
void processString(std::string_view str) {
    // Can accept: std::string, const char*, 
    // or substring without allocation
}

std::string s = "Hello World";
processString(s);              // No copy
processString("literal");      // No copy
processString(s.substr(0, 5)); // C++20: no copy!

// WARNING: Don't store string_view - it doesn't own data
std::string_view getDangerous() {
    std::string temp = "bad";
    return temp;  // DANGER: temp is destroyed!
}
```

### std::span - Non-Owning Array View
**Then:**
```cpp
void process(const std::vector<int>& vec);  // Only works with vector
void process(const int* arr, size_t len);   // Manual size tracking
```

**Now:**
```cpp
void process(std::span<const int> data) {
    // Works with vector, array, C-array, etc.
    for (int x : data) { /* ... */ }
}

std::vector<int> vec = {1, 2, 3};
std::array<int, 3> arr = {1, 2, 3};
int carray[] = {1, 2, 3};

process(vec);     // All work!
process(arr);
process(carray);
```

### Structured Bindings - Unpack Multiple Return Values
**Then:**
```cpp
std::pair<bool, int> divide(int a, int b) {
    if (b == 0) return {false, 0};
    return {true, a / b};
}

auto result = divide(10, 2);
if (result.first) {
    std::cout << result.second;  // Ugly .first, .second
}
```

**Now:**
```cpp
auto [success, value] = divide(10, 2);  // Unpack directly
if (success) {
    std::cout << value;  // Named variables!
}

// Works with maps too
std::map<std::string, int> m;
for (const auto& [key, value] : m) {
    // Much clearer than it->first, it->second
}
```

### Filesystem Library
**Then:**
```cpp
#include <sys/stat.h>  // Platform-specific
#include <dirent.h>

struct stat st;
stat("file.txt", &st);
long size = st.st_size;
```

**Now:**
```cpp
#include <filesystem>
namespace fs = std::filesystem;

auto size = fs::file_size("file.txt");
if (fs::exists("file.txt")) {
    // ...
}

// Iterate directory
for (const auto& entry : fs::directory_iterator("path")) {
    std::cout << entry.path() << "\n";
}
```

### Ranges - Composable Algorithms
**Then:**
```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};
std::vector<int> temp;

// Filter even numbers
std::copy_if(vec.begin(), vec.end(), std::back_inserter(temp),
    [](int x) { return x % 2 == 0; });

// Square them
std::vector<int> result;
std::transform(temp.begin(), temp.end(), std::back_inserter(result),
    [](int x) { return x * x; });
```

**Now:**
```cpp
auto result = vec 
    | std::views::filter([](int x) { return x % 2 == 0; })
    | std::views::transform([](int x) { return x * x; });
// Lazy evaluation! Nothing computed until you iterate
```

## Common Modern Patterns

### RAII Everywhere
Modern C++ takes RAII to the extreme. Every resource is managed by an object:

```cpp
// Files
std::ifstream file("data.txt");  // Opens in constructor
// Automatically closes in destructor

// Locks
std::mutex m;
{
    std::lock_guard lock(m);  // Locks in constructor
    // Critical section
}  // Unlocks in destructor

// Memory
auto ptr = std::make_unique<Widget>();  // Allocated
// Automatically deleted when ptr goes out of scope
```

### Prefer Value Semantics
Modern C++ encourages value semantics over pointers:

```cpp
// Old way - pointers everywhere
Widget* create() { return new Widget(); }
void process(Widget* w);

// Modern way - values with move semantics
Widget create() { return Widget(); }  // Cheap to return by value!
void process(Widget w);               // Or const Widget&
```

### Use Standard Algorithms
Don't write loops when standard algorithms exist:

```cpp
// Instead of:
int sum = 0;
for (const auto& x : vec) {
    sum += x;
}

// Use:
int sum = std::accumulate(vec.begin(), vec.end(), 0);

// Or ranges (C++20):
int sum = std::ranges::fold_left(vec, 0, std::plus{});
```

## Pitfalls for Returning Developers

1. **Don't use `new`/`delete` directly**
   - Use `std::make_unique`/`std::make_shared`
   - Or just use value semantics

2. **`string_view` doesn't own its data**
   ```cpp
   std::string_view bad() {
       std::string s = "temp";
       return s;  // DANGER! Returns view to destroyed string
   }
   ```

3. **Moving leaves objects in valid but unspecified state**
   ```cpp
   std::vector<int> v1 = {1, 2, 3};
   auto v2 = std::move(v1);
   // v1 is now empty (or could be in some other valid state)
   // Don't use v1 again!
   ```

4. **`auto` can hide copies**
   ```cpp
   auto copy = getVector();      // Makes a copy
   auto& ref = getVector();      // Reference - might dangle!
   const auto& ref = getVector(); // Const reference
   auto&& universal = getVector(); // Universal reference
   ```

5. **Ranges are lazy**
   ```cpp
   auto v = vec | std::views::filter(...);
   // Nothing computed yet!
   // Computation happens when you iterate over v
   ```

## What to Learn Next

1. **C++11**: Move semantics, lambdas, auto, smart pointers
2. **C++14**: Generic lambdas, binary literals
3. **C++17**: optional, string_view, structured bindings, filesystem
4. **C++20**: Ranges, concepts, coroutines, modules
5. **C++23**: std::expected, std::flat_map, more ranges

## Resources

- **Books**: "Effective Modern C++" by Scott Meyers
- **Online**: cppreference.com, C++ Core Guidelines
- **Practice**: LeetCode, Codewars with modern C++

## Your Project Features Map

| File | Key Modern Features |
|------|-------------------|
| log_entry.h | enum class, constexpr, optional, chrono |
| log_parser.h | filesystem, span, move-only types |
| statistics.h | ranges, algorithms, structured bindings |
| main.cpp | auto, range-for, if-initializers |
| All | RAII, value semantics, [[nodiscard]] |
