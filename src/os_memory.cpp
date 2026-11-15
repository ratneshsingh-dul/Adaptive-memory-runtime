#include "amr/os_memory.hpp"

#include <limits>
#include <new>
#include <stdexcept>
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
#include <unistd.h>

#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif
#if !defined(MAP_ANONYMOUS)
#error "This platform does not provide anonymous mmap mappings"
#endif
#endif

namespace amr {

MemoryRegion::MemoryRegion(std::byte* data, std::size_t size) noexcept
    : data_(data), size_(size) {}

MemoryRegion::~MemoryRegion() {
    (void)reset();
}

MemoryRegion::MemoryRegion(MemoryRegion&& other) noexcept
    : data_(std::exchange(other.data_, nullptr)),
      size_(std::exchange(other.size_, 0)) {}

MemoryRegion& MemoryRegion::operator=(MemoryRegion&& other) noexcept {
    if (this != &other && reset()) {
        data_ = std::exchange(other.data_, nullptr);
        size_ = std::exchange(other.size_, 0);
    }
    return *this;
}

bool MemoryRegion::reset() noexcept {
    if (data_ == nullptr) {
        size_ = 0;
        return true;
    }
    if (!OSMemory::release(data_, size_)) {
        return false;
    }
    data_ = nullptr;
    size_ = 0;
    return true;
}

std::size_t OSMemory::page_size() {
    static const std::size_t value = [] {
#ifdef _WIN32
        SYSTEM_INFO info{};
        GetSystemInfo(&info);
        if (info.dwPageSize == 0) {
            throw std::runtime_error("GetSystemInfo returned a zero page size");
        }
        return static_cast<std::size_t>(info.dwPageSize);
#else
        const long result = sysconf(_SC_PAGESIZE);
        if (result <= 0) {
            throw std::runtime_error("sysconf could not determine the page size");
        }
        return static_cast<std::size_t>(result);
#endif
    }();
    return value;
}

MemoryRegion OSMemory::allocate(std::size_t bytes) {
    if (bytes == 0) {
        return {};
    }

    const std::size_t page = page_size();
    const std::size_t remainder = bytes % page;
    const std::size_t padding = remainder == 0 ? 0 : page - remainder;
    if (bytes > std::numeric_limits<std::size_t>::max() - padding) {
        throw std::bad_alloc{};
    }
    const std::size_t mapped_size = bytes + padding;

    void* address = nullptr;
#ifdef _WIN32
    address = VirtualAlloc(nullptr, mapped_size,
                           MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
    address = mmap(nullptr, mapped_size, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (address == MAP_FAILED) {
        address = nullptr;
    }
#endif

    if (address == nullptr) {
        throw std::bad_alloc{};
    }
    return MemoryRegion(static_cast<std::byte*>(address), mapped_size);
}

bool OSMemory::release(std::byte* data, std::size_t bytes) noexcept {
    if (data == nullptr) {
        return true;
    }
#ifdef _WIN32
    (void)bytes;
    return VirtualFree(data, 0, MEM_RELEASE) != 0;
#else
    return munmap(data, bytes) == 0;
#endif
}

} // namespace amr
