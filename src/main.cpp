#include "amr/malloc_resource.hpp"

#include <iostream>

int main() {
    amr::MallocResource memory;
    auto* buffer = static_cast<char*>(memory.allocate(64));
    buffer[0] = 'A';

    std::cout << "Allocated 64 bytes through MallocResource; first byte is "
              << buffer[0] << ".\n";

    memory.deallocate(buffer);
    return 0;
}
