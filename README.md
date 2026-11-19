# Adaptive Memory Runtime

A C++20 allocator project exploring specialized allocation paths and a small online model for learning object lifetimes.

The current code includes a system-allocator reference, a page-aware OS region provider, a bounded bump arena, and a size-class pool for reusable small allocations. Larger or page-incompatible requests use the system-allocator fallback. The persistent free-list allocator and lifetime predictor are still ahead.

## Build and run

```sh
cmake -S . -B build
cmake --build build
./build/amr_demo
./build/amr_baseline 1000000
```

The demo shows bump allocation and reset, small-slot reuse, and the large-request fallback. See [size-class notes](docs/size-classes.md), [bump arena notes](docs/bump-arena.md), [OS memory notes](docs/os-memory.md), and [benchmark notes](docs/benchmark-notes.md).
