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
    // The OS mapping size, rounded up to a full number of pages.
    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] explicit operator bool() const noexcept { return data_ != nullptr; }

    // Releases the mapping and clears ownership on success. If the OS reports
    // failure, the region retains its address so the caller can inspect/retry.
    [[nodiscard]] bool reset() noexcept;

private:
    friend class OSMemory;

    MemoryRegion(std::byte* data, std::size_t size) noexcept;

    std::byte* data_ = nullptr;
    std::size_t size_ = 0;
};

class OSMemory final {
public:
    [[nodiscard]] static std::size_t page_size();

    // Returns an empty region for zero bytes. Other sizes are rounded up to a
    // page boundary. Throws std::bad_alloc if the OS cannot provide the region.
    [[nodiscard]] static MemoryRegion allocate(std::size_t bytes);

private:
    friend class MemoryRegion;
    static bool release(std::byte* data, std::size_t bytes) noexcept;
};

} // namespace amr
