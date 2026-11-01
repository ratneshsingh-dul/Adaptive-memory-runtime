#pragma once

#include <cstddef>
#include <cstdlib>
#include <limits>
#include <new>
#include <stdexcept>

#ifdef _WIN32
#include <malloc.h>
#endif

namespace amr {

// The system-allocator reference resource. AdaptiveResource will use the same
// allocate/deallocate shape so workloads can be measured without changing them.
class MallocResource final {
public:
    [[nodiscard]] void* allocate(
        std::size_t bytes,
        std::size_t alignment = alignof(std::max_align_t)) const {
        if (!is_power_of_two(alignment)) {
            throw std::invalid_argument("alignment must be a non-zero power of two");
        }
        if (bytes == 0) {
            return nullptr;
        }

        void* result = nullptr;
        if (alignment <= alignof(std::max_align_t)) {
            result = std::malloc(bytes);
        } else {
#ifdef _WIN32
            result = _aligned_malloc(bytes, alignment);
#else
            const std::size_t remainder = bytes % alignment;
            const std::size_t padding = remainder == 0 ? 0 : alignment - remainder;
            if (bytes > std::numeric_limits<std::size_t>::max() - padding) {
                throw std::bad_alloc{};
            }
            result = std::aligned_alloc(alignment, bytes + padding);
#endif
        }

        if (result == nullptr) {
            throw std::bad_alloc{};
        }
        return result;
    }

    void deallocate(
        void* address,
        std::size_t alignment = alignof(std::max_align_t)) const noexcept {
        if (address == nullptr) {
            return;
        }
#ifdef _WIN32
        if (alignment > alignof(std::max_align_t)) {
            _aligned_free(address);
            return;
        }
#else
        (void)alignment;
#endif
        std::free(address);
    }

private:
    static constexpr bool is_power_of_two(std::size_t value) noexcept {
        return value != 0 && (value & (value - 1)) == 0;
    }
};

} // namespace amr
