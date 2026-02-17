# Modern C++ Quick Reference - Cheat Sheet

## Smart Pointers
```cpp
// Unique ownership (move-only)
auto ptr = std::make_unique<Widget>();
auto moved = std::move(ptr);  // Transfer ownership

// Shared ownership (reference counted)
auto shared = std::make_shared<Widget>();
auto copy = shared;  // Both own the Widget

// Check before use
if (ptr) {
    ptr->doSomething();
}

// Custom deleter
auto file = std::unique_ptr<FILE, decltype(&fclose)>(
    fopen("f.txt", "r"), &fclose);
```

## Type Deduction
```cpp
auto x = 42;              // int
auto& r = x;              // int&
const auto& cr = x;       // const int&
auto&& ur = getWidget();  // Universal reference

// Trailing return type
auto divide(int a, int b) -> std::optional<int> {
    if (b == 0) return std::nullopt;
    return a / b;
}
```

## Loops
```cpp
// Range-based for
for (const auto& item : container) { }
for (auto& item : container) { }      // Modify
for (auto&& item : container) { }     // Universal ref

// With structured binding
for (const auto& [key, value] : map) { }
```

## Lambdas
```cpp
// Basic
auto sum = [](int a, int b) { return a + b; };

// Capture by value
int x = 5;
auto f = [x]() { return x; };

// Capture by reference
auto f = [&x]() { x++; };

// Capture all by value/reference
auto f = [=]() { /* ... */ };
auto f = [&]() { /* ... */ };

// Mutable lambda
auto f = [x]() mutable { x++; return x; };

// Generic lambda (C++14)
auto f = [](auto a, auto b) { return a + b; };

// Init capture (C++14)
auto f = [x = getValue()]() { return x * 2; };
```

## Optional
```cpp
std::optional<int> maybeInt;

// Check
if (maybeInt.has_value()) { }
if (maybeInt) { }  // Converts to bool

// Access
int val = maybeInt.value();     // Throws if empty
int val = *maybeInt;            // UB if empty
int val = maybeInt.value_or(0); // Safe with default

// Set
maybeInt = 42;
maybeInt = std::nullopt;  // Clear
```

## String View
```cpp
void process(std::string_view sv) {
    // sv.data(), sv.size(), sv.substr()
}

std::string s = "Hello";
process(s);           // No copy
process("literal");   // No copy
process(s.substr(0, 3)); // No copy (C++20)

// WARNING: Don't store dangling views
std::string_view bad = std::string("temp");  // DANGLING!
```

## Span
```cpp
void process(std::span<const int> data) {
    for (int x : data) { }
}

std::vector<int> v = {1, 2, 3};
std::array<int, 3> a = {1, 2, 3};
int arr[] = {1, 2, 3};

process(v);    // All work
process(a);
process(arr);
```

## Structured Bindings
```cpp
auto [a, b] = std::pair{1, 2};
auto [a, b, c] = std::tuple{1, 2, 3};
auto [a, b] = std::array{1, 2};

// With maps
std::map<int, std::string> m;
for (auto& [key, value] : m) { }

// With custom types
struct Point { int x, y; };
auto [x, y] = Point{1, 2};
```

## Ranges & Views (C++20)
```cpp
#include <ranges>
namespace rv = std::views;

// Views are lazy
auto evens = vec | rv::filter([](int x) { return x % 2 == 0; });

// Compose views
auto result = vec
    | rv::filter([](int x) { return x > 0; })
    | rv::transform([](int x) { return x * 2; })
    | rv::take(10);

// Common views
rv::filter(pred)
rv::transform(func)
rv::take(n)
rv::drop(n)
rv::reverse
rv::split(delimiter)
```

## Algorithms
```cpp
#include <algorithm>

// Modern style with ranges
std::ranges::sort(vec);
std::ranges::find(vec, value);
std::ranges::count_if(vec, pred);
std::ranges::copy(src, dst);

// Classic style
std::sort(vec.begin(), vec.end());
std::find(vec.begin(), vec.end(), value);
std::count_if(vec.begin(), vec.end(), pred);
std::copy(src.begin(), src.end(), dst.begin());
```

## Filesystem
```cpp
#include <filesystem>
namespace fs = std::filesystem;

// Check existence
if (fs::exists(path)) { }

// File info
auto size = fs::file_size(path);
auto time = fs::last_write_time(path);

// Iterate directory
for (auto& entry : fs::directory_iterator(path)) {
    if (entry.is_regular_file()) {
        std::cout << entry.path() << '\n';
    }
}

// Create/remove
fs::create_directory(path);
fs::remove(path);
fs::copy(src, dst);
```

## Chrono
```cpp
#include <chrono>
using namespace std::chrono;

// Time points
auto now = system_clock::now();
auto later = now + hours(2);

// Durations
auto dur = minutes(5) + seconds(30);
auto ms = duration_cast<milliseconds>(dur);

// Literals (C++14)
using namespace std::literals;
auto dur = 5min + 30s + 100ms;
```

## Variant
```cpp
#include <variant>

std::variant<int, std::string> v;

v = 42;
v = "hello";

// Check type
if (std::holds_alternative<int>(v)) { }

// Access
int i = std::get<int>(v);         // Throws if wrong type
auto* p = std::get_if<int>(&v);   // nullptr if wrong

// Visit
std::visit([](auto&& arg) {
    std::cout << arg << '\n';
}, v);
```

## Attributes
```cpp
[[nodiscard]] int getValue();  // Warn if unused
[[maybe_unused]] int x = 0;    // Suppress unused warning
[[deprecated("Use newFunc")]] void oldFunc();
[[fallthrough]];               // In switch
[[likely]] if (condition) { }  // C++20
[[unlikely]] if (error) { }    // C++20
```

## Constexpr
```cpp
constexpr int square(int x) {
    return x * x;
}

constexpr auto val = square(5);  // Computed at compile time

// Constexpr if (C++17)
if constexpr (std::is_integral_v<T>) {
    // Code for integers
} else {
    // Code for others
}
```

## Concepts (C++20)
```cpp
#include <concepts>

template<std::integral T>
T add(T a, T b) { return a + b; }

// Custom concepts
template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

template<Numeric T>
T multiply(T a, T b) { return a * b; }
```

## Move Semantics
```cpp
// Move instead of copy
auto v2 = std::move(v1);  // v1 is now "moved-from"

// Perfect forwarding
template<typename T>
void wrapper(T&& arg) {
    func(std::forward<T>(arg));
}

// Return value optimization
Widget createWidget() {
    Widget w;
    return w;  // No move needed, RVO
}
```

## Initialization
```cpp
// Uniform initialization
Widget w{42};
std::vector<int> v{1, 2, 3};

// Direct initialization
Widget w(42);

// Copy initialization
Widget w = Widget(42);

// Aggregate initialization
struct Point { int x, y; };
Point p{1, 2};
Point p = {.x = 1, .y = 2};  // Designated (C++20)
```

## Common Patterns

### RAII
```cpp
{
    std::lock_guard lock(mutex);
    // Critical section
}  // Automatic unlock

{
    std::ifstream file("data.txt");
    // Use file
}  // Automatic close
```

### Factory Functions
```cpp
auto createWidget() {
    return std::make_unique<Widget>();  // Explicit
}

Widget createWidget() {
    return Widget{};  // RVO handles it
}
```

### Error Handling
```cpp
// Optional for single errors
std::optional<int> divide(int a, int b) {
    if (b == 0) return std::nullopt;
    return a / b;
}

// Exceptions for exceptional cases
int divide(int a, int b) {
    if (b == 0) throw std::invalid_argument("div by zero");
    return a / b;
}
```

## Compile Commands

```bash
# Basic compile
g++ -std=c++20 file.cpp

# With warnings
g++ -std=c++20 -Wall -Wextra -Wpedantic file.cpp

# Optimized
g++ -std=c++20 -O3 file.cpp

# Debug
g++ -std=c++20 -g -O0 file.cpp

# Sanitizers
g++ -std=c++20 -fsanitize=address,undefined file.cpp
```

## Resources
- cppreference.com - Best C++ reference
- C++ Core Guidelines - Best practices
- compiler explorer (godbolt.org) - See assembly
- quick-bench.com - Benchmark snippets
