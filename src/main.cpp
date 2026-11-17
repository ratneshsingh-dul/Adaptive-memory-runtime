#include "amr/bump_arena.hpp"
#include "amr/malloc_resource.hpp"

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

    amr::MallocResource system_memory;
    auto* buffer = static_cast<char*>(system_memory.allocate(64));
    buffer[0] = 'A';
    std::cout << "MallocResource: allocated 64 bytes; first byte is "
              << buffer[0] << ".\n";
    system_memory.deallocate(buffer);
    return 0;
}
