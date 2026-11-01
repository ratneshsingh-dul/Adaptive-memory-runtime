# Adaptive Memory Runtime

A C++20 allocator project. The idea is to combine a few specialized allocation paths with a small online model that learns which allocation contexts tend to produce short-lived objects.

The model will only guide future placement. It will never decide when memory is safe to reclaim. That remains the job of the allocator’s ownership and scope rules.

The project is at the build-skeleton stage; the allocator is not implemented yet. The schedule is in [PLAN.md](PLAN.md), and the current design notes are in [docs/architecture.md](docs/architecture.md).

## Build

```sh
cmake -S . -B build
cmake --build build
```

The first target is only a small C++20 executable to establish the project layout. The runtime itself will be added in stages.
