# Allocation metadata

Allocations from `SizeClassPool` and `FreeListAllocator` now share a small metadata prefix. It records a magic value, the owning allocator, the requested byte count and alignment, and a source tag. A pointer to that record is stored immediately before the user address.

The pool uses the source tag to distinguish a reusable size-class slot from the `MallocResource` fallback. Its private header also keeps the slot base and class index. The free-list allocator uses the same prefix to validate the owner and source, then treats the record as the start of its larger block header, which carries the physical-neighbor and free-bin links.

Both allocators recover this record during `deallocate(void*)`; callers do not have to repeat size or alignment. The owner pointer prevents one allocator from accepting a pointer owned by another allocator. The magic value and source tag catch several routing mistakes. As with `free`, the pointer still must be a live pointer returned by the matching allocator; arbitrary pointers and use-after-free are invalid.

The bump arena does not use per-object metadata because it has no individual deallocation operation. Its memory is reclaimed by an explicit reset after the owning phase ends.
