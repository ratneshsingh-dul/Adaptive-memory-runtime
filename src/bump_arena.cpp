#include "amr/bump_arena.hpp"

#include <cstdint>
#include <limits>
#include <new>
#include <stdexcept>
#include <utility>

namespace amr {

BumpArena::BumpArena(std::size_t capacity_bytes)
    : region_(OSMemory::allocate(capacity_bytes)) {}

BumpArena::BumpArena(MemoryRegion region) noexcept
    : region_(std::move(region)) {}

void* BumpArena::allocate(std::size_t bytes, std::size_t alignment) {
    if (!is_power_of_two(alignment)) {
        throw std::invalid_argument("alignment must be a non-zero power of two");
    }
    if (bytes == 0) {
        return nullptr;
    }

    const std::size_t available = capacity() - offset_;
    const auto base = reinterpret_cast<std::uintptr_t>(region_.data());
    constexpr auto address_max = std::numeric_limits<std::uintptr_t>::max();
    if (offset_ > address_max - base) {
        throw std::bad_alloc{};
    }
    const auto current = base + offset_;
    const auto mask = static_cast<std::uintptr_t>(alignment - 1);
    if (current > address_max - mask) {
        throw std::bad_alloc{};
    }
    const auto aligned = (current + mask) & ~mask;
    const std::size_t padding = static_cast<std::size_t>(aligned - current);

    if (padding > available || bytes > available - padding) {
        throw std::bad_alloc{};
    }

    offset_ += padding + bytes;
    return reinterpret_cast<void*>(aligned);
}

bool BumpArena::is_power_of_two(std::size_t value) noexcept {
    return value != 0 && (value & (value - 1)) == 0;
}

} // namespace amr
