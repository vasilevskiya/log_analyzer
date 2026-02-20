#pragma once

#include <cstddef>
#include <memory>
#include <new>
#include <vector>

namespace LogAnalyzer {

namespace detail {

struct MemoryPool {
    std::vector<std::byte> storage;
    size_t offset = 0;

    explicit MemoryPool(size_t bytes) : storage(bytes) {}
};

} // namespace detail

template <typename T>
class PoolAllocator {
public:
    using value_type = T;

    explicit PoolAllocator(size_t poolSize = 1024)
        : pool_(std::make_shared<detail::MemoryPool>(poolSize * sizeof(T))) {}

    // Rebind/converting constructor: share the same pool across types
    template <typename U>
    PoolAllocator(const PoolAllocator<U>& other) noexcept
        : pool_(other.pool_) {}

    [[nodiscard]] T* allocate(size_t n) {
        size_t bytes = n * sizeof(T);
        size_t aligned_offset = align_up(pool_->offset, alignof(T));
        if (aligned_offset + bytes > pool_->storage.size()) {
            throw std::bad_alloc();
        }
        T* result = reinterpret_cast<T*>(pool_->storage.data() + aligned_offset);
        pool_->offset = aligned_offset + bytes;
        return result;
    }

    void deallocate(T* /*p*/, size_t /*n*/) noexcept {
        // No-op: pool freed on destruction (arena style)
    }

    template <typename U>
    bool operator==(const PoolAllocator<U>& other) const noexcept {
        return pool_ == other.pool_;
    }

    template <typename U>
    bool operator!=(const PoolAllocator<U>& other) const noexcept {
        return pool_ != other.pool_;
    }

private:
    std::shared_ptr<detail::MemoryPool> pool_;

    static size_t align_up(size_t value, size_t alignment) noexcept {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    template <typename U>
    friend class PoolAllocator;
};

} // namespace LogAnalyzer
