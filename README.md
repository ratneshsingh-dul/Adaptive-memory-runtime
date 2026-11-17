# Adaptive Memory Runtime

A C++20 allocator project exploring specialized allocation paths and a small online model for learning object lifetimes.

The current code includes a system-allocator reference, a page-aware OS region provider, and a bounded bump arena for phase-scoped allocations. The size-class pool, persistent allocator, and lifetime predictor are still ahead. The model will guide future placement; it will not decide when memory is safe to reclaim.

## Build and run

```sh
cmake -S . -B build
cmake --build build
./build/amr_demo
./build/amr_baseline 1000000
```

The demo requests an aligned allocation from a bump arena, shows the consumed capacity, and resets the arena. See [bump arena notes](docs/bump-arena.md), [OS memory notes](docs/os-memory.md), and [benchmark notes](docs/benchmark-notes.md) for the current behavior and limits.
