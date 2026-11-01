# Allocation API notes

The first concrete resource is `amr::MallocResource` in `include/amr/malloc_resource.hpp`. It gives the baseline and later resources a common shape without forcing virtual dispatch into every call.

## Contract

- `allocate(bytes, alignment)` accepts non-zero power-of-two alignments.
- A zero-byte request returns `nullptr`.
- Allocation failure throws `std::bad_alloc`; invalid alignment throws `std::invalid_argument`.
- Pass the same alignment to `deallocate` that was used for the allocation. Passing `nullptr` to `deallocate` is a no-op.
- Requests at or below `alignof(std::max_align_t)` use the regular C allocator. Larger alignments use the platform’s aligned allocation routine.

The runtime will keep this call shape for its later resource implementation. Size and alignment are passed back on free so pool-backed resources can validate and account for deallocations.
