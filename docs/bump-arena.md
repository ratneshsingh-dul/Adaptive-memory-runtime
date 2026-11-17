# Bump arena

`BumpArena` owns one `MemoryRegion` and serves allocations by advancing an offset. Allocation is constant time: it rounds the current address up to the requested power-of-two alignment, checks the remaining capacity, then advances the offset. A full arena throws `std::bad_alloc`; invalid alignment throws `std::invalid_argument`; a zero-byte request returns `nullptr`.

`used()` counts consumed bytes from the start of the region, including alignment padding. `remaining()` reports the unconsumed tail. There is no per-object deallocation. Call `reset()` only after every object allocated from the arena is no longer in use; it sets the offset back to zero in constant time.

Reset does not clear the bytes and does not call object destructors. Constructed objects with non-trivial destructors must be destroyed by their owner before reset. The arena is not thread-safe and does not grow: requests that do not fit fail instead of silently acquiring another region.

The demo allocates 73 bytes at 64-byte alignment, makes a second allocation, reports capacity and used space, then resets the arena and reports zero bytes used.
