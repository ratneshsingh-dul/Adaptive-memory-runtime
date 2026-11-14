# Adaptive Memory Runtime

A C++20 allocator project exploring specialized allocation paths and a small online model for learning object lifetimes.

The project currently has two pieces: `MallocResource` as the system-allocator reference, and `OSMemory` for owning raw OS-backed regions. The arenas and lifetime predictor are still ahead. The model will guide future placement only; reclamation follows explicit ownership and scope rules.

## Build and run

```sh
cmake -S . -B build
cmake --build build
./build/amr_demo
./build/amr_baseline 1000000
```

The demo requests a 2 MiB OS region, touches its first and last bytes, and releases it through RAII. The baseline harness times repeated system allocation/free pairs. See [OS memory notes](docs/os-memory.md) and [benchmark notes](docs/benchmark-notes.md) for current behavior and measurement limits.
