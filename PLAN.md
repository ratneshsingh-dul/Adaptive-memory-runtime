# 20-Day Build and GitHub Delivery Plan

## Delivery objective

Deliver a clean, independently buildable C++20 repository for an adaptive memory runtime, with an explainable design, safe fallback behavior, Google Benchmark comparisons, reproducible result reporting, and a strong interview walkthrough.

**Schedule:** Day 1 is 2026-11-02; Day 20 is 2026-11-21 (Asia/Calcutta).  
**Daily Git target:** 3 meaningful commits/pushes, with a 4th when the work naturally separates into another reviewable slice. No empty or cosmetic-only commits to inflate the count. Push each completed slice to the dedicated GitHub remote once configured.

## Quality gates

- Keep the repository standalone; do not modify another project to house this code.
- Separate OS memory acquisition, allocation strategies, telemetry, prediction, and policy.
- Keep allocation metadata sufficient to route deallocation safely.
- Unknown or low-confidence contexts use a conservative fallback.
- Never reset an arena unless its lifetime boundary is explicit and safe.
- Record benchmark hardware, compiler, build mode, workload parameters, and measurement method.
- Define internal and external fragmentation metrics precisely before reporting them.
- Treat the 20 ns predictor goal as an experiment and report measured costs, not as a guarantee.
- Prioritize correctness, explainability, and reproducibility over unsupported performance claims.

## Day-by-day plan and push targets

### Day 1 — 2026-11-02: Repository and project contract
1. Commit project charter, scope, assumptions, and non-goals.
2. Commit architecture sketch, component boundaries, and data-flow description.
3. Commit initial CMake project skeleton and documented build commands.
4. Optional: commit a tiny executable smoke example if it is independently useful.

### Day 2 — 2026-11-03: API and measurement baseline
1. Commit public allocator/runtime API proposal and alignment/error contracts.
2. Commit benchmark methodology and explicit fragmentation definitions.
3. Commit baseline harness structure against `malloc`/`free`.
4. Optional: commit compiler/platform support matrix.

### Day 3 — 2026-11-04: OS memory provider
1. Commit OS memory provider interface and result/error types.
2. Commit Windows virtual-memory implementation.
3. Commit POSIX mapping implementation and platform selection.
4. Optional: commit provider usage example and platform notes.

### Day 4 — 2026-11-05: Region management
1. Commit reservation/commit/release behavior and page-size handling.
2. Commit alignment and overflow-safe size calculations.
3. Commit RAII region ownership and failure paths.
4. Optional: commit a diagnostic sample showing region state.

### Day 5 — 2026-11-06: Bump arena
1. Commit bump arena structure and allocation path.
2. Commit reset semantics and explicit lifetime contract.
3. Commit bounds, exhaustion, and alignment handling.
4. Optional: commit small usage example.

### Day 6 — 2026-11-07: Size-class pools
1. Commit size-class mapping and class configuration.
2. Commit block allocation and free-slot reuse.
3. Commit large-size bypass rules and allocator diagnostics.
4. Optional: commit workload example for common small sizes.

### Day 7 — 2026-11-08: Persistent free-list arena
1. Commit block headers and free-list representation.
2. Commit splitting and reuse behavior.
3. Commit coalescing and invariants documentation.
4. Optional: commit a visual allocator layout diagram.

### Day 8 — 2026-11-09: Large and aligned allocations
1. Commit aligned allocation handling.
2. Commit dedicated large-allocation path and release policy.
3. Commit pointer ownership/range lookup metadata.
4. Optional: commit edge-case examples in the API guide.

### Day 9 — 2026-11-10: Allocation records and routing metadata
1. Commit allocation metadata format and lifecycle.
2. Commit safe deallocation route selection.
3. Commit allocator counters for live bytes, reserved bytes, and fallbacks.
4. Optional: commit debug inspection output.

### Day 10 — 2026-11-11: Concurrency foundation
1. Commit threading and ownership model decision record.
2. Commit thread-local fast-path boundary or synchronization design.
3. Commit cross-thread free strategy and conservative fallback.
4. Optional: commit concurrency limitations document if full support is deferred.

### Day 11 — 2026-11-12: Allocation context capture
1. Commit context ID and hashing design.
2. Commit low-overhead call-site capture option and platform abstraction.
3. Commit explicit user allocation tags and size-class contribution.
4. Optional: commit capture overhead measurement mode.

### Day 12 — 2026-11-13: Lifetime telemetry
1. Commit timestamp source abstraction and allocation sample records.
2. Commit bounded per-context statistics storage.
3. Commit lifetime observation and update API.
4. Optional: commit telemetry export format.

### Day 13 — 2026-11-14: EMA predictor
1. Commit EMA implementation and configurable parameters.
2. Commit sample-count confidence and unknown-context behavior.
3. Commit prediction result type with reason/confidence information.
4. Optional: commit model state inspection example.

### Day 14 — 2026-11-15: Routing policy
1. Commit short-lived/long-lived routing policy.
2. Commit confidence thresholds, fallback, and policy overrides.
3. Commit safe learning update on deallocation.
4. Optional: commit policy decision trace output.

### Day 15 — 2026-11-16: Runtime integration
1. Commit runtime ownership and lifecycle integration.
2. Commit end-to-end allocate/predict/route/deallocate flow.
3. Commit configuration and policy documentation.
4. Optional: commit end-to-end usage example.

### Day 16 — 2026-11-17: STL adapter and sample workload
1. Commit `SmartAllocator<T>` adapter and allocator-traits compatibility surface.
2. Commit container integration example.
3. Commit mixed temporary/session workload generator.
4. Optional: commit portability notes for allocator propagation behavior.

### Day 17 — 2026-11-18: Benchmark suite
1. Commit throughput and latency benchmark cases.
2. Commit fragmentation and retained-memory accounting.
3. Commit predictor overhead and warmup/cold-start benchmark cases.
4. Optional: commit workload parameter presets for reproducibility.

### Day 18 — 2026-11-19: Performance evaluation
1. Commit baseline measurements with environment details.
2. Commit adaptive allocator measurements and raw result capture.
3. Commit analysis of wins, losses, and sensitivity to policy parameters.
4. Optional: commit profiler guidance and flamegraph instructions.

### Day 19 — 2026-11-20: Hardening and communication
1. Commit API and architecture documentation polish.
2. Commit limitations, safety model, and design tradeoffs.
3. Commit benchmark result tables and reproducibility instructions.
4. Optional: commit demo script and interview talking points.

### Day 20 — 2026-11-21: Release candidate
1. Commit final integration fixes and version metadata.
2. Commit release notes and v0.1 tag preparation.
3. Commit final README walkthrough and repository presentation polish.
4. Optional: commit follow-up roadmap based on measured results.

## Daily review loop

At the end of each day: inspect the diff, ensure the commits tell a coherent story, confirm no secrets or generated build artifacts are tracked, and push the completed commits. Record blockers and carry them into the next day's first slice rather than silently dropping scope.

## Interview-ready final walkthrough

1. State the problem and workload assumptions.
2. Draw the allocation path: context → prediction → policy → arena → metadata-backed free.
3. Explain why the predictor is deliberately lightweight and how confidence controls routing.
4. Show benchmark setup and measured comparison with the system allocator.
5. Discuss tradeoffs, failure cases, and one design decision changed by the measurements.
