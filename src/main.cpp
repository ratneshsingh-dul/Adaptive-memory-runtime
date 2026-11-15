#include "amr/malloc_resource.hpp"
#include "amr/os_memory.hpp"

#include <cstddef>
#include <iostream>

int main() {
    const std::size_t page = amr::OSMemory::page_size();
    const std::size_t requested_bytes = page + 1;
    auto region = amr::OSMemory::allocate(requested_bytes);
    if (!region || region.size() < requested_bytes || region.size() % page != 0) {
        std::cerr << "OSMemory returned a region with an invalid size.\n";
        return 1;
    }

    region.data()[0] = std::byte{0x2A};
    region.data()[region.size() - 1] = std::byte{0x2B};
    std::cout << "Page size: " << page << " bytes; requested "
              << requested_bytes << " bytes; mapped " << region.size() << " bytes.\n";
    if (!region.reset()) {
        std::cerr << "The OS could not release the memory region.\n";
        return 1;
    }

    amr::MallocResource system_memory;
    auto* buffer = static_cast<char*>(system_memory.allocate(64));
    buffer[0] = 'A';
    std::cout << "MallocResource: allocated 64 bytes; first byte is "
              << buffer[0] << ".\n";
    system_memory.deallocate(buffer);
    return 0;
}
