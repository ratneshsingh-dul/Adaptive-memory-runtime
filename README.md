# Adaptive Memory Runtime

A C++20 allocator project exploring specialized allocation paths and a small online model for learning object lifetimes.

The current code includes a system-allocator reference, OS-backed regions, a bounded bump arena, reusable size-class pools, and a fixed-capacity free-list allocator with block splitting and coalescing. The lifetime predictor and routing policy are still ahead.

## Build and run

```sh
cmake -S . -B build
cmake --build build
./build/amr_demo
./build/amr_baseline 1000000
```

The demo shows arena reset, small-slot reuse, large-request fallback, and free-list coalescing. See [free-list notes](docs/free-list.md), [size-class notes](docs/size-classes.md), and [benchmark notes](docs/benchmark-notes.md).
