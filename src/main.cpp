#include "amr/malloc_resource.hpp"
#include "amr/os_memory.hpp"

#include <cstddef>
#include <iostream>

int main() {
    auto region = amr::OSMemory::allocate(2 * 1024 * 1024);
    if (!region) {
        std::cerr << "The OS did not provide a memory region.\n";
        return 1;
    }

    region.data()[0] = std::byte{0x2A};
    region.data()[region.size() - 1] = std::byte{0x2B};
    std::cout << "OS region: " << region.size() / (1024 * 1024)
              << " MiB; first and last bytes are accessible.\n";

    amr::MallocResource system_memory;
    auto* buffer = static_cast<char*>(system_memory.allocate(64));
    buffer[0] = 'A';
    std::cout << "MallocResource: allocated 64 bytes; first byte is "
              << buffer[0] << ".\n";
    system_memory.deallocate(buffer);

    // The OS region is released automatically when it leaves scope.
    return 0;
}
