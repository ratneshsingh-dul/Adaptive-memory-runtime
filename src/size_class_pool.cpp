#include "amr/size_class_pool.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>

namespace amr {

SizeClassPool::SizeClassPool(std::size_t chunk_bytes)
    : chunk_bytes_(chunk_bytes) {
    if (chunk_bytes_ == 0) {
        throw std::invalid_argument("chunk size must be greater than zero");
    }
}

void* SizeClassPool::allocate(std::size_t bytes, std::size_t alignment) {
    if (!is_power_of_two(alignment)) {
        throw std::invalid_argument("alignment must be a non-zero power of two");
    }
    if (bytes == 0) {
        return nullptr;
    }

    const std::size_t page = OSMemory::page_size();
    const bool page_compatible_alignment = alignment <= page && page % alignment == 0;
    if (page_compatible_alignment) {
        const std::size_t required = required_slot_bytes(bytes, alignment);
        for (std::size_t index = 0; index < class_sizes_.size(); ++index) {
            if (class_sizes_[index] >= required) {
                if (free_lists_[index] == nullptr) {
                    add_chunk(index);
                }

                FreeNode* slot = free_lists_[index];
                free_lists_[index] = slot->next;
                auto* header = ::new (static_cast<void*>(slot)) AllocationHeader{
                    AllocationMetadata{AllocationMetadata::expected_magic, this, bytes,
                                       alignment, AllocationSource::size_class_slot},
                    slot, 0, index};
                std::byte* payload = payload_address(slot, alignment);
                store_allocation_metadata(payload, &header->metadata);
                return payload;
            }
        }
    }

    return allocate_large(bytes, alignment);
}

void SizeClassPool::deallocate(void* address) noexcept {
    if (address == nullptr) {
        return;
    }

    AllocationMetadata* metadata = load_allocation_metadata(address);
    if (!is_valid_metadata(metadata) || metadata->owner != this) {
        return;
    }
    auto* header = reinterpret_cast<AllocationHeader*>(metadata);

    if (metadata->source == AllocationSource::system_fallback) {
        void* base = header->base;
        const std::size_t alignment = header->raw_alignment;
        std::destroy_at(header);
        large_resource_.deallocate(base, alignment);
        return;
    }
    if (metadata->source != AllocationSource::size_class_slot ||
        header->class_index >= free_lists_.size()) {
        return;
    }

    const std::size_t index = header->class_index;
    auto* slot = static_cast<FreeNode*>(header->base);
    std::destroy_at(header);
    auto* node = ::new (static_cast<void*>(slot)) FreeNode{free_lists_[index]};
    free_lists_[index] = node;
}

bool SizeClassPool::is_power_of_two(std::size_t value) noexcept {
    return value != 0 && (value & (value - 1)) == 0;
}

std::size_t SizeClassPool::required_slot_bytes(
    std::size_t bytes, std::size_t alignment) {
    constexpr std::size_t fixed_overhead =
        sizeof(AllocationHeader) + sizeof(AllocationMetadata*);
    const std::size_t padding = alignment - 1;
    const std::size_t maximum = std::numeric_limits<std::size_t>::max();
    if (padding > maximum - fixed_overhead ||
        bytes > maximum - fixed_overhead - padding) {
        throw std::bad_alloc{};
    }
    return fixed_overhead + padding + bytes;
}

std::byte* SizeClassPool::payload_address(void* base, std::size_t alignment) {
    const auto address = reinterpret_cast<std::uintptr_t>(base);
    constexpr auto maximum = std::numeric_limits<std::uintptr_t>::max();
    constexpr std::size_t fixed_overhead =
        sizeof(AllocationHeader) + sizeof(AllocationMetadata*);
    const auto mask = static_cast<std::uintptr_t>(alignment - 1);
    if (address > maximum - fixed_overhead ||
        address + fixed_overhead > maximum - mask) {
        throw std::bad_alloc{};
    }
    const auto start = address + fixed_overhead;
    const auto aligned = (start + mask) & ~mask;
    return reinterpret_cast<std::byte*>(aligned);
}

void SizeClassPool::add_chunk(std::size_t class_index) {
    const std::size_t slot_size = class_sizes_[class_index];
    const std::size_t requested_size = std::max(chunk_bytes_, slot_size);
    MemoryRegion chunk = OSMemory::allocate(requested_size);
    const std::size_t slot_count = chunk.size() / slot_size;
    if (slot_count == 0) {
        throw std::bad_alloc{};
    }

    chunks_.push_back(std::move(chunk));
    std::byte* base = chunks_.back().data();
    for (std::size_t slot_index = 0; slot_index < slot_count; ++slot_index) {
        auto* address = base + slot_index * slot_size;
        auto* node = ::new (static_cast<void*>(address)) FreeNode{free_lists_[class_index]};
        free_lists_[class_index] = node;
    }
}

void* SizeClassPool::allocate_large(std::size_t bytes, std::size_t alignment) {
    const std::size_t total_bytes = required_slot_bytes(bytes, alignment);
    const std::size_t raw_alignment =
        std::max(alignment, alignof(AllocationHeader));
    void* base = large_resource_.allocate(total_bytes, raw_alignment);
    auto* header = ::new (base) AllocationHeader{
        AllocationMetadata{AllocationMetadata::expected_magic, this, bytes,
                           alignment, AllocationSource::system_fallback},
        base, raw_alignment, 0};
    std::byte* payload = payload_address(base, alignment);
    store_allocation_metadata(payload, &header->metadata);
    return payload;
}

} // namespace amr
