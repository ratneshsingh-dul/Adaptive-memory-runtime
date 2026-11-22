#include "amr/free_list_allocator.hpp"

#include <bit>
#include <cstdint>
#include <limits>
#include <new>
#include <stdexcept>

namespace amr {

FreeListAllocator::FreeListAllocator(std::size_t capacity_bytes)
    : region_(OSMemory::allocate(capacity_bytes)) {
    if (!region_) {
        throw std::invalid_argument("free-list capacity must be greater than zero");
    }

    first_physical_ = ::new (static_cast<void*>(region_.data())) BlockHeader{};
    first_physical_->metadata = {
        AllocationMetadata::expected_magic, this, 0, 0, AllocationSource::free_list};
    first_physical_->block_size = region_.size();
    first_physical_->is_free = true;
    free_bytes_ = region_.size();
    insert_free(first_physical_);
}

void* FreeListAllocator::allocate(std::size_t bytes, std::size_t alignment) {
    if (!is_power_of_two(alignment)) {
        throw std::invalid_argument("alignment must be a non-zero power of two");
    }
    if (bytes == 0) {
        return nullptr;
    }

    const std::size_t minimum_span = sizeof(BlockHeader) + sizeof(AllocationMetadata*) + bytes;
    if (minimum_span < bytes) {
        throw std::bad_alloc{};
    }
    const std::size_t first_bin = bin_for(minimum_span);
    BlockHeader* selected = nullptr;
    std::size_t selected_span = 0;
    std::size_t selected_offset = 0;

    for (std::size_t bin = first_bin; bin < free_bins_.size() && selected == nullptr; ++bin) {
        for (BlockHeader* block = free_bins_[bin]; block != nullptr; block = block->next_free) {
            std::size_t span = 0;
            std::size_t payload_offset = 0;
            if (layout_for(block, bytes, alignment, span, payload_offset)) {
                selected = block;
                selected_span = span;
                selected_offset = payload_offset;
                break;
            }
        }
    }
    if (selected == nullptr) {
        throw std::bad_alloc{};
    }

    const std::size_t original_size = selected->block_size;
    remove_free(selected);
    const std::size_t remainder = original_size - selected_span;
    constexpr std::size_t minimum_free_span =
        sizeof(BlockHeader) + sizeof(AllocationMetadata*) + 1;

    if (remainder >= minimum_free_span) {
        auto* tail_address = reinterpret_cast<std::byte*>(selected) + selected_span;
        auto* tail = ::new (static_cast<void*>(tail_address)) BlockHeader{};
        tail->metadata = {
            AllocationMetadata::expected_magic, this, 0, 0, AllocationSource::free_list};
        tail->block_size = remainder;
        tail->previous_physical = selected;
        tail->next_physical = selected->next_physical;
        tail->is_free = true;
        if (tail->next_physical != nullptr) {
            tail->next_physical->previous_physical = tail;
        }
        selected->next_physical = tail;
        selected->block_size = selected_span;
        insert_free(tail);
        free_bytes_ -= selected_span;
    } else {
        selected->block_size = original_size;
        free_bytes_ -= original_size;
    }

    selected->metadata = {
        AllocationMetadata::expected_magic, this, bytes, alignment,
        AllocationSource::free_list};
    selected->payload_offset = selected_offset;
    selected->is_free = false;
    auto* payload = reinterpret_cast<std::byte*>(selected) + selected_offset;
    store_allocation_metadata(payload, &selected->metadata);
    return payload;
}

void FreeListAllocator::deallocate(void* address) noexcept {
    if (address == nullptr) {
        return;
    }

    AllocationMetadata* metadata = load_allocation_metadata(address);
    if (!is_valid_metadata(metadata) || metadata->owner != this ||
        metadata->source != AllocationSource::free_list) {
        return;
    }
    auto* block = reinterpret_cast<BlockHeader*>(metadata);
    if (block->is_free) {
        return;
    }

    block->is_free = true;
    block->metadata.requested_bytes = 0;
    block->metadata.requested_alignment = 0;
    free_bytes_ += block->block_size;

    if (block->previous_physical != nullptr && block->previous_physical->is_free) {
        BlockHeader* previous = block->previous_physical;
        remove_free(previous);
        previous->block_size += block->block_size;
        previous->next_physical = block->next_physical;
        if (block->next_physical != nullptr) {
            block->next_physical->previous_physical = previous;
        }
        block->metadata.magic = 0;
        block = previous;
    }

    if (block->next_physical != nullptr && block->next_physical->is_free) {
        BlockHeader* next = block->next_physical;
        remove_free(next);
        block->block_size += next->block_size;
        block->next_physical = next->next_physical;
        if (next->next_physical != nullptr) {
            next->next_physical->previous_physical = block;
        }
        next->metadata.magic = 0;
    }

    insert_free(block);
}

bool FreeListAllocator::is_power_of_two(std::size_t value) noexcept {
    return value != 0 && (value & (value - 1)) == 0;
}

std::size_t FreeListAllocator::bin_for(std::size_t block_size) noexcept {
    return static_cast<std::size_t>(std::bit_width(block_size) - 1);
}

bool FreeListAllocator::layout_for(
    const BlockHeader* block,
    std::size_t bytes,
    std::size_t alignment,
    std::size_t& span,
    std::size_t& payload_offset) noexcept {
    const auto base = reinterpret_cast<std::uintptr_t>(block);
    constexpr auto address_max = std::numeric_limits<std::uintptr_t>::max();
    constexpr std::size_t fixed_overhead =
        sizeof(BlockHeader) + sizeof(AllocationMetadata*);
    const auto mask = static_cast<std::uintptr_t>(alignment - 1);
    if (base > address_max - fixed_overhead) {
        return false;
    }
    const auto unaligned = base + fixed_overhead;
    if (unaligned > address_max - mask) {
        return false;
    }
    const auto payload = (unaligned + mask) & ~mask;
    if (payload > address_max - bytes) {
        return false;
    }
    const auto end = payload + bytes;
    const std::size_t raw_span = static_cast<std::size_t>(end - base);
    constexpr std::size_t header_alignment = alignof(BlockHeader);
    const std::size_t remainder = raw_span % header_alignment;
    const std::size_t padding = remainder == 0 ? 0 : header_alignment - remainder;
    if (raw_span > std::numeric_limits<std::size_t>::max() - padding) {
        return false;
    }
    span = raw_span + padding;
    payload_offset = static_cast<std::size_t>(payload - base);
    return span <= block->block_size;
}

void FreeListAllocator::insert_free(BlockHeader* block) noexcept {
    const std::size_t bin = bin_for(block->block_size);
    block->previous_free = nullptr;
    block->next_free = free_bins_[bin];
    if (block->next_free != nullptr) {
        block->next_free->previous_free = block;
    }
    free_bins_[bin] = block;
}

void FreeListAllocator::remove_free(BlockHeader* block) noexcept {
    const std::size_t bin = bin_for(block->block_size);
    if (block->previous_free != nullptr) {
        block->previous_free->next_free = block->next_free;
    } else {
        free_bins_[bin] = block->next_free;
    }
    if (block->next_free != nullptr) {
        block->next_free->previous_free = block->previous_free;
    }
    block->previous_free = nullptr;
    block->next_free = nullptr;
}

} // namespace amr
