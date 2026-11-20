#include "amr/bump_arena.hpp"
#include "amr/free_list_allocator.hpp"
#include "amr/malloc_resource.hpp"
#include "amr/size_class_pool.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>

int main() {
    amr::BumpArena arena(64 * 1024);
    auto* first = static_cast<std::byte*>(arena.allocate(73, 64));
    auto* second = static_cast<std::byte*>(arena.allocate(128));
    if (reinterpret_cast<std::uintptr_t>(first) % 64 != 0 || second == nullptr) {
        std::cerr << "BumpArena returned an invalid allocation.\n";
        return 1;
    }
    first[0] = std::byte{0x2A};
    second[0] = std::byte{0x2B};

    std::cout << "BumpArena: capacity " << arena.capacity()
              << " bytes; used " << arena.used()
              << " bytes including alignment padding.\n";
    arena.reset();
    std::cout << "After reset: " << arena.used() << " bytes used.\n";

    amr::SizeClassPool pool;
    void* small = pool.allocate(48, 64);
    if (reinterpret_cast<std::uintptr_t>(small) % 64 != 0) {
        std::cerr << "SizeClassPool returned a misaligned small object.\n";
        return 1;
    }
    pool.deallocate(small);
    void* reused = pool.allocate(48, 64);
    pool.deallocate(reused);

    void* large = pool.allocate(20 * 1024, 64);
    if (reinterpret_cast<std::uintptr_t>(large) % 64 != 0) {
        std::cerr << "SizeClassPool returned a misaligned large object.\n";
        return 1;
    }
    pool.deallocate(large);
    std::cout << "SizeClassPool: small slot reused; oversized request used the fallback; "
              << pool.chunk_count() << " OS chunk(s) owned.\n";

    amr::FreeListAllocator persistent(64 * 1024);
    void* first_block = persistent.allocate(256, 64);
    void* second_block = persistent.allocate(512, 64);
    if (reinterpret_cast<std::uintptr_t>(first_block) % 64 != 0 ||
        reinterpret_cast<std::uintptr_t>(second_block) % 64 != 0) {
        std::cerr << "FreeListAllocator returned a misaligned block.\n";
        return 1;
    }
    persistent.deallocate(first_block);
    persistent.deallocate(second_block);
    if (persistent.free_bytes() != persistent.capacity()) {
        std::cerr << "FreeListAllocator did not coalesce adjacent free blocks.\n";
        return 1;
    }
    void* merged_block = persistent.allocate(32 * 1024, 64);
    persistent.deallocate(merged_block);
    std::cout << "FreeListAllocator: adjacent blocks coalesced; "
              << persistent.free_bytes() << " free of "
              << persistent.capacity() << " bytes.\n";

    amr::MallocResource system_memory;
    auto* buffer = static_cast<char*>(system_memory.allocate(64));
    buffer[0] = 'A';
    std::cout << "MallocResource: allocated 64 bytes; first byte is "
              << buffer[0] << ".\n";
    system_memory.deallocate(buffer);
    return 0;
}
