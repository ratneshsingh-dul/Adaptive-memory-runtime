#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace amr {

enum class AllocationSource : std::uint8_t {
    size_class_slot,
    system_fallback,
    free_list
};

// Common prefix stored in each allocator's private block header. The pointer
// stored immediately before a user address points to this prefix.
struct AllocationMetadata final {
    static constexpr std::uint64_t expected_magic = 0xA110CA7E5EED1234ULL;

    std::uint64_t magic = 0;
    const void* owner = nullptr;
    std::size_t requested_bytes = 0;
    std::size_t requested_alignment = 0;
    AllocationSource source = AllocationSource::size_class_slot;
};

[[nodiscard]] inline bool is_valid_metadata(
    const AllocationMetadata* metadata) noexcept {
    return metadata != nullptr && metadata->magic == AllocationMetadata::expected_magic;
}

inline void store_allocation_metadata(
    void* payload, AllocationMetadata* metadata) noexcept {
    auto* slot = static_cast<std::byte*>(payload) - sizeof(metadata);
    std::memcpy(slot, &metadata, sizeof(metadata));
}

[[nodiscard]] inline AllocationMetadata* load_allocation_metadata(
    void* payload) noexcept {
    AllocationMetadata* metadata = nullptr;
    const auto* slot = static_cast<const std::byte*>(payload) - sizeof(metadata);
    std::memcpy(&metadata, slot, sizeof(metadata));
    return metadata;
}

} // namespace amr
