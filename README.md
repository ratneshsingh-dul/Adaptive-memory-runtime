# Adaptive Memory Runtime

A C++20 allocator project. The idea is to combine specialized allocation paths with a small online model that learns which allocation contexts tend to produce short-lived objects.

The model will only guide future placement. It will never decide when memory is safe to reclaim; that follows explicit ownership and scope rules.

The project is at the reference-resource stage. `MallocResource` currently wraps the system allocator and gives the later implementation a call shape to follow. The design is in [docs/architecture.md](docs/architecture.md), the API contract is in [docs/api.md](docs/api.md), and the 20-day schedule is in [PLAN.md](PLAN.md).

## Build and run

```sh
cmake -S . -B build
cmake --build build
./build/amr_demo
./build/amr_baseline 1000000
```

The baseline is a small dependency-free timing harness for repeated allocation/free pairs. It is useful for early checks, not a substitute for the planned Google Benchmark workload suite. See [docs/benchmark-notes.md](docs/benchmark-notes.md) for the measurement details and first local reference run.
