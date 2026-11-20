# Free-list allocator

`FreeListAllocator` owns one fixed OS region and tracks its physical blocks with a doubly linked chain. Free blocks are also placed in logarithmic size bins. Allocation searches from the smallest plausible bin, chooses the first block that can fit the requested size and alignment, and splits off a tail block when the remainder can hold another allocation.

Each returned pointer has a small private header immediately before it. That header identifies its owning allocator and physical block, so `deallocate(void*)` can return the block without a size argument. When a block is freed, the allocator merges it with free physical neighbors and reinserts the merged span into the appropriate bin.

The allocator is fixed-capacity and single-threaded. It does not fall back to another region when full; allocation throws `std::bad_alloc`. `free_bytes()` includes block headers and reports all free spans, so it is not the same as the total payload size available. All pointers become invalid when the allocator is destroyed.

The demo allocates two adjacent blocks, frees them, and checks that coalescing restores the arena's full free span. It then allocates a larger block from that merged span.
