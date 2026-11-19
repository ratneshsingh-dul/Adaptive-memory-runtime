# Size class pool

`SizeClassPool` keeps free lists for slot sizes from 64 bytes through 8 KiB. The slot includes a small private header and enough alignment padding for the user request. When a class runs out of free slots, the pool maps another OS region and splits it into slots. Freed small allocations return to their class free list for reuse.

The class uses a separate `MallocResource` path when the request plus metadata and alignment cannot fit a slot, or when its alignment is not compatible with the system page size. A pointer-sized header immediately before the returned address records how to route `deallocate(void*)`; callers do not pass the size back.

The pool is not thread-safe. Its chunks stay mapped until the pool is destroyed, even when every slot is free. The caller must deallocate large fallback allocations before destroying the pool; all pointers become invalid when the pool itself is destroyed. Invalid pointers and double frees are programming errors.

The demo allocates and frees a small 64-byte-aligned slot, allocates and frees a 20 KiB request through the fallback path, then reports the number of OS chunks the pool owns.
