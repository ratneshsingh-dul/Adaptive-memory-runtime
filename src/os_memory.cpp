#include "amr/os_memory.hpp"

#include <new>
#include <utility>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/mman.h>

#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif
#endif

namespace amr {

MemoryRegion::MemoryRegion(std::byte* data, std::size_t size) noexcept
    : data_(data), size_(size) {}

MemoryRegion::~MemoryRegion() {
    OSMemory::release(data_, size_);
}

MemoryRegion::MemoryRegion(MemoryRegion&& other) noexcept
    : data_(std::exchange(other.data_, nullptr)),
      size_(std::exchange(other.size_, 0)) {}

MemoryRegion& MemoryRegion::operator=(MemoryRegion&& other) noexcept {
    if (this != &other) {
        OSMemory::release(data_, size_);
        data_ = std::exchange(other.data_, nullptr);
        size_ = std::exchange(other.size_, 0);
    }
    return *this;
}

MemoryRegion OSMemory::allocate(std::size_t bytes) {
    if (bytes == 0) {
        return {};
    }

    void* address = nullptr;
#ifdef _WIN32
    address = VirtualAlloc(nullptr, bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
    address = mmap(nullptr, bytes, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (address == MAP_FAILED) {
        address = nullptr;
    }
#endif

    if (address == nullptr) {
        throw std::bad_alloc{};
    }
    return MemoryRegion(static_cast<std::byte*>(address), bytes);
}

void OSMemory::release(std::byte* data, std::size_t bytes) noexcept {
    if (data == nullptr) {
        return;
    }
#ifdef _WIN32
    (void)bytes;
    VirtualFree(data, 0, MEM_RELEASE);
#else
    (void)munmap(data, bytes);
#endif
}

} // namespace amr
