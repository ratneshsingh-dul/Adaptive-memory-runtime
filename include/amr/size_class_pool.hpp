#pragma once

#include "amr/allocation_metadata.hpp"
#include "amr/malloc_resource.hpp"
#include "amr/os_memory.hpp"

#include <array>
#include <cstddef>
#include <vector>

namespace amr {

class SizeClassPool final {
public:
    explicit SizeClassPool(std::size_t chunk_bytes = 64 * 1024);
    ~SizeClassPool() = default;

    SizeClassPool(const SizeClassPool&) = delete;
    SizeClassPool& operator=(const SizeClassPool&) = delete;
    SizeClassPool(SizeClassPool&&) = delete;
    SizeClassPool& operator=(SizeClassPool&&) = delete;

    [[nodiscard]] void* allocate(
        std::size_t bytes,
        std::size_t alignment = alignof(std::max_align_t));
    void deallocate(void* address) noexcept;

    [[nodiscard]] std::size_t chunk_count() const noexcept { return chunks_.size(); }

private:
    struct FreeNode {
        FreeNode* next;
    };

    struct AllocationHeader {
        AllocationMetadata metadata;
        void* base;
        std::size_t raw_alignment;
        std::size_t class_index;
    };

    static constexpr std::array<std::size_t, 8> class_sizes_{
        64, 128, 256, 512, 1024, 2048, 4096, 8192};

    [[nodiscard]] static bool is_power_of_two(std::size_t value) noexcept;
    [[nodiscard]] static std::size_t required_slot_bytes(
        std::size_t bytes, std::size_t alignment);
    [[nodiscard]] static std::byte* payload_address(
        void* base, std::size_t alignment);

    void add_chunk(std::size_t class_index);
    [[nodiscard]] void* allocate_large(
        std::size_t bytes, std::size_t alignment);

    std::size_t chunk_bytes_;
    std::array<FreeNode*, class_sizes_.size()> free_lists_{};
    std::vector<MemoryRegion> chunks_;
    MallocResource large_resource_;
};

} // namespace amr
