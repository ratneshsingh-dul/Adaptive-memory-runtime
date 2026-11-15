# OS memory provider

`OSMemory::allocate(bytes)` obtains a read/write region directly from the operating system. Windows uses `VirtualAlloc` with reserve and commit flags; POSIX systems use a private anonymous `mmap` mapping.

`OSMemory::page_size()` reports the system page size. Requests are rounded up to a whole page, with an overflow check before rounding. `MemoryRegion::size()` reports the mapped size, so it can be larger than the requested size. A zero-byte request returns an empty region. A mapping failure throws `std::bad_alloc`.

`MemoryRegion` is move-only. Its destructor releases the mapping, and `reset()` lets callers check the result explicitly: it returns `true` and clears the region on success; on failure it returns `false` and keeps the address and size so release can be retried. A destructor cannot report a release error, so code that needs to handle that case should call `reset()` directly before the object leaves scope. Move assignment leaves both objects unchanged if it cannot release the destination's current mapping.

The demo requests one byte more than a page, checks that the mapping rounds up, touches the first and last bytes, and explicitly releases the region. The Windows implementation uses `VirtualFree` with `MEM_RELEASE`; POSIX uses `munmap` with the rounded mapping size.
