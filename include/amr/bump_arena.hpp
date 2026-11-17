#pragma once

#include "amr/os_memory.hpp"

#include <cstddef>

namespace amr {

class BumpArena final {
public:
    explicit BumpArena(std::size_t capacity_bytes);
    explicit BumpArena(MemoryRegion region) noexcept;

    BumpArena(const BumpArena&) = delete;
    BumpArena& operator=(const BumpArena&) = delete;
    BumpArena(BumpArena&&) noexcept = default;
    BumpArena& operator=(BumpArena&&) noexcept = delete;

    [[nodiscard]] void* allocate(
        std::size_t bytes,
        std::size_t alignment = alignof(std::max_align_t));

    // Reclaims every allocation in constant time. Object destructors are not run.
    void reset() noexcept { offset_ = 0; }

    [[nodiscard]] std::size_t capacity() const noexcept { return region_.size(); }
    [[nodiscard]] std::size_t used() const noexcept { return offset_; }
    [[nodiscard]] std::size_t remaining() const noexcept {
        return capacity() - offset_;
    }

private:
    static bool is_power_of_two(std::size_t value) noexcept;

    MemoryRegion region_;
    std::size_t offset_ = 0;
};

} // namespace amr

