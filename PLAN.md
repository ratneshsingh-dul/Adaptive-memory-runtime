# Build plan

The aim is to finish a small but serious C++20 allocator project in 20 days, from Nov 1 through Nov 20, 2026. The result should be something I can explain at a whiteboard and defend with measurements: what the allocator does, why its policy works for a workload, and where it loses to the system allocator.

I’m keeping the first version deliberately bounded. It will have a few distinct allocation paths, lightweight lifetime learning, safe fallbacks, and benchmarks. It will not claim that one allocator wins on every workload. Call-site capture, concurrency, and the predictor’s cost all need to earn their place with measurements.

## How I’ll work

Aim for three useful GitHub pushes on a workday, and make a fourth only when there is another clean piece to review. Each push should contain a real change: a design decision, a working slice, or a result that improves the project. No empty commits, padding, or made-up performance numbers. If a day slips, carry the unfinished work forward and update this plan.

Keep the code split by responsibility: OS memory, allocation strategies, metadata, telemetry, policy, and the public allocator adapter. Never use a lifetime prediction as permission to reclaim memory; reclamation must follow an explicit ownership or scope rule. Unknown contexts start on the general path. Benchmark reports include the machine, compiler, build mode, workload, and metric definitions.

## The 20 days

| Date | Focus | What should exist by the end of the day |
|---|---|---|
| Nov 1 | Project footing | This plan, initial architecture decisions, and a buildable CMake/C++ skeleton. |
| Nov 2 | API and baseline | A small runtime API and a baseline harness that can time the system allocator. |
| Nov 3 | OS memory | One region interface with Windows and POSIX implementations behind it. |
| Nov 4 | Region correctness | Page-size handling, alignment checks, overflow handling, and clear ownership of mapped regions. |
| Nov 5 | Bump arena | A bounded bump allocator with an explicit reset contract. |
| Nov 6 | Small object pools | Size classes, slot reuse, and a separate path for requests that are too large. |
| Nov 7 | Persistent allocations | A free-list arena with splitting, reuse, and coalescing. |
| Nov 8 | Allocation metadata | Enough information to free through the right path, including aligned and large allocations. |
| Nov 9 | Accounting | Track live and reserved bytes, fallbacks, and allocator ownership in a form the demo can show. |
| Nov 10 | Threading decision | Settle the ownership model. Add a thread-local path only if cross-thread frees can be handled clearly. |
| Nov 11 | Context IDs | Build a compact context key from call-site information, size, and optional caller tags. Measure capture cost. |
| Nov 12 | Lifetime observations | Record sampled allocation/free times and keep bounded per-context statistics. |
| Nov 13 | EMA predictor | Estimate lifetime with an EMA; expose sample count and confidence so a handful of samples cannot dictate policy. |
| Nov 14 | Routing policy | Route future allocations by prediction, with a conservative default and a way to inspect each decision. |
| Nov 15 | Runtime integration | Connect allocation, metadata, learning, and deallocation into one usable runtime. |
| Nov 16 | Standard allocator adapter | Add `SmartAllocator<T>` and a small container example. |
| Nov 17 | Workloads | Add benchmark cases for temporary objects, mixed lifetimes, size churn, and long-lived sessions. |
| Nov 18 | Measurements | Compare against `malloc`/`free`; report throughput, latency, retained memory, and predictor overhead. |
| Nov 19 | Explain the results | Write down where it helped, where it did not, and what policy or design changed because of the data. |
| Nov 20 | Release candidate | Clean build instructions, usage example, design notes, limitations, and a tagged v0.1 release. |

## What counts as a result

For performance, report throughput and latency on named workloads rather than one blended score. For memory use, distinguish live bytes, reserved bytes, internal waste from size classes, and external fragmentation under a stated request pattern. For learning, show how estimates change as observations arrive and how often the policy falls back while confidence is low.

The 20-nanosecond predictor target is a hypothesis to measure on a named machine and compiler. Stack walking, locks, cache misses, and timer reads can easily dominate that budget. If the number is missed, report the cause and keep expensive context capture off the common path.

## Interview walkthrough

The final explanation should fit on a whiteboard: allocation context goes into a small model; the model suggests a destination; policy checks confidence and safety; metadata remembers the actual source so deallocation is correct. Then show benchmark evidence and talk through one surprising result or tradeoff. The measured story is the point; the project should not depend on impressive-sounding claims.
