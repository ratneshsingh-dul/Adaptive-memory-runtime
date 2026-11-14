#pragma once

#include <cstddef>

namespace amr {

class OSMemory;

class MemoryRegion final {
public:
    MemoryRegion() noexcept = default;
    ~MemoryRegion();

    MemoryRegion(const MemoryRegion&) = delete;
    MemoryRegion& operator=(const MemoryRegion&) = delete;
    MemoryRegion(MemoryRegion&& other) noexcept;
    MemoryRegion& operator=(MemoryRegion&& other) noexcept;

    [[nodiscard]] std::byte* data() noexcept { return data_; }
    [[nodiscard]] const std::byte* data() const noexcept { return data_; }
    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] explicit operator bool() const noexcept { return data_ != nullptr; }

private:
    friend class OSMemory;

    MemoryRegion(std::byte* data, std::size_t size) noexcept;

    std::byte* data_ = nullptr;
    std::size_t size_ = 0;
};

class OSMemory final {
public:
    // Returns an empty region for zero bytes. Throws std::bad_alloc if the OS
    // cannot provide the requested region.
    [[nodiscard]] static MemoryRegion allocate(std::size_t bytes);

private:
    friend class MemoryRegion;
    static void release(std::byte* data, std::size_t bytes) noexcept;
};

} // namespace amr
