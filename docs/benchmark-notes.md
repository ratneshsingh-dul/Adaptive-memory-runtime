# Benchmark notes

The first harness is intentionally small and has no third-party dependency. It times repeated `MallocResource::allocate`/`deallocate` pairs using `steady_clock`. Google Benchmark is planned for the fuller workload suite; these early numbers are a reference point, not a performance claim.

## Running it

```sh
cmake -S . -B build
cmake --build build
./build/amr_baseline 1000000
```

On Windows, run `build\amr_baseline.exe 1000000` from PowerShell. The argument is the number of allocation/free pairs in each sample. The harness performs a warmup of up to 10,000 pairs, then reports the median of five samples for 32, 128, 512, and 4096-byte requests.

The timed operation is one allocation followed immediately by one free; it does not touch the returned memory. Results therefore describe allocator-call throughput for this narrow pattern. They do not represent a server workload, tail latency, memory pressure, multithreaded behavior, or fragmentation.

## First local reference run

Collected on Nov 2, 2026, on Windows 11 Home Single Language with a 12th Gen Intel Core i5-12450H. Compiler: MSYS2 GCC 16.2.0. CMake: 4.4.2 with Ninja. Single-config CMake defaults to Release; select `-DCMAKE_BUILD_TYPE=Debug` when debugging.

One run with 1,000,000 pairs per sample produced:

| Request size | Median ns per allocation/free pair | Pairs per second |
|---:|---:|---:|
| 32 bytes | 35.21 | 28,403,200 |
| 128 bytes | 42.13 | 23,738,591 |
| 512 bytes | 39.81 | 25,122,093 |
| 4096 bytes | 45.64 | 21,910,173 |

These values came from one run on one machine and showed noticeable variation by size. Keep them as a smoke reference only; rerun the benchmark suite on the same machine and build before drawing comparisons. The C++ API wrapper is thin and calls `malloc`/`free` for these default-alignment requests.

## Metrics for later allocator comparisons

- **Throughput and latency:** report allocation/free pairs per second and per-pair latency for each named workload. The final Google Benchmark suite should also report distributions where practical.
- **Live and reserved memory:** report requested live bytes separately from memory reserved from the OS.
- **Internal waste:** for a pool, `(capacity of live slots - requested live bytes) / capacity of live slots`.
- **External fragmentation:** for an arena with visible free spans, `1 - largest free span / total free bytes`; report zero when there are no free bytes. This metric does not apply to an opaque system allocator, so do not invent a `malloc` fragmentation number.
- **Predictor overhead:** compare the same workload with prediction disabled and enabled, separating context-capture cost when possible.

Always include the machine, compiler, build mode, workload parameters, and run count next to reported results.
