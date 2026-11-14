# OS memory provider

`OSMemory::allocate(bytes)` obtains a read/write region directly from the operating system. The Windows implementation uses `VirtualAlloc` with reserve and commit flags. The POSIX implementation uses a private anonymous `mmap` mapping.

The returned `MemoryRegion` is move-only and releases its mapping when destroyed. It stores the requested byte count because POSIX `munmap` needs the mapped length. A zero-byte request returns an empty region; an OS allocation failure throws `std::bad_alloc`.

The demo asks for a 2 MiB region, writes to its first and last bytes, then lets RAII release it at scope exit. The provider does not yet expose page size or round requests itself; that is the next step, along with clearer diagnostics for release failures.
