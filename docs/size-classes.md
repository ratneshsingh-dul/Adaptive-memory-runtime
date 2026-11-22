# Size class pool

`SizeClassPool` keeps free lists for slot sizes from 64 bytes through 8 KiB. The slot includes private metadata and enough alignment padding for the user request. When a class runs out of free slots, the pool maps another OS region and splits it into slots. Freed small allocations return to their class free list for reuse.

The pool uses the shared `AllocationMetadata` record to identify its owner and whether a pointer came from a size-class slot or the `MallocResource` fallback. Its private header also stores the slot base and class index. The metadata pointer sits immediately before the returned address, so `deallocate(void*)` can select the correct path without a size argument.

The class uses a separate `MallocResource` path when the request plus metadata and alignment cannot fit a slot, or when its alignment is not compatible with the system page size. The pool is not thread-safe. Its chunks stay mapped until the pool is destroyed, even when every slot is free. The caller must deallocate large fallback allocations before destroying the pool; all pointers become invalid when the pool itself is destroyed. Invalid pointers and double frees are programming errors.

The demo allocates and frees a small 64-byte-aligned slot, allocates and frees a 20 KiB request through the fallback path, then reports the number of OS chunks the pool owns.
