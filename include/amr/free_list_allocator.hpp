#pragma once

#include "amr/allocation_metadata.hpp"
#include "amr/os_memory.hpp"

#include <array>
#include <cstddef>

namespace amr {

class FreeListAllocator final {
public:
    explicit FreeListAllocator(std::size_t capacity_bytes);
    ~FreeListAllocator() = default;

    FreeListAllocator(const FreeListAllocator&) = delete;
    FreeListAllocator& operator=(const FreeListAllocator&) = delete;
    FreeListAllocator(FreeListAllocator&&) = delete;
    FreeListAllocator& operator=(FreeListAllocator&&) = delete;

    [[nodiscard]] void* allocate(
        std::size_t bytes,
        std::size_t alignment = alignof(std::max_align_t));
    void deallocate(void* address) noexcept;

    [[nodiscard]] std::size_t capacity() const noexcept { return region_.size(); }
    // Includes block headers and free-list space.
    [[nodiscard]] std::size_t free_bytes() const noexcept { return free_bytes_; }

private:
    struct BlockHeader {
        AllocationMetadata metadata;
        std::size_t block_size;
        std::size_t payload_offset;
        BlockHeader* previous_physical;
        BlockHeader* next_physical;
        BlockHeader* previous_free;
        BlockHeader* next_free;
        bool is_free;
    };

    static constexpr std::size_t bin_count = sizeof(std::size_t) * 8;

    [[nodiscard]] static bool is_power_of_two(std::size_t value) noexcept;
    [[nodiscard]] static std::size_t bin_for(std::size_t block_size) noexcept;
    [[nodiscard]] static bool layout_for(
        const BlockHeader* block,
        std::size_t bytes,
        std::size_t alignment,
        std::size_t& span,
        std::size_t& payload_offset) noexcept;

    void insert_free(BlockHeader* block) noexcept;
    void remove_free(BlockHeader* block) noexcept;

    MemoryRegion region_;
    std::array<BlockHeader*, bin_count> free_bins_{};
    BlockHeader* first_physical_ = nullptr;
    std::size_t free_bytes_ = 0;
};

} // namespace amr
