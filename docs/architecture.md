# Architecture notes

This is the first cut at the design, not a promise to build every allocator trick at once. The aim is to keep the parts replaceable so benchmark results can change the implementation without changing the public interface.

## Allocation path

The runtime will take a request (size, alignment, and optional tag), pick a destination, and return a pointer with enough side metadata to free it later. The predictor can influence where the *next* allocation from a context goes. It cannot reclaim an existing object or reset an arena; only an explicit owner or scope can do that.

```text
request
  -> context key and size class
  -> policy (prediction + confidence + safety rules)
  -> arena or system fallback
  -> pointer plus ownership metadata
  -> deallocation updates observed lifetime
```

## Pieces and responsibilities

- **OS memory provider:** reserve and release page-backed regions. No allocation policy lives here.
- **Arenas and pools:** serve requests from already acquired regions. Each strategy owns its reuse rules.
- **Allocation metadata:** records the source and information needed for a correct free, even if the prediction was wrong.
- **Telemetry:** collects sampled allocation and deallocation observations by context.
- **Predictor:** maintains a lightweight lifetime estimate and confidence per context.
- **Policy:** decides whether a prediction is trustworthy enough to influence a new allocation; otherwise use the general path.
- **Public adapter:** exposes the runtime and, later, a standard C++ allocator wrapper.

## Decisions for the first version

### Learn from context, but keep capture optional

Capturing a full stack trace for every allocation is likely to cost more than the prediction itself. Start with a compact call-site key where the compiler/platform supports it, plus size class and an optional user tag. Keep a mode that disables capture so its cost can be compared directly.

### Predictions affect placement, not object lifetime

An incorrect short-lived prediction must not free memory early. The first implementation will only use the prediction to choose a pool that supports the required deallocation contract. Arena reset remains explicit.

### Unknown contexts use the conservative path

An EMA based on one object is not strong evidence. Track sample count with the estimate and require enough observations before the policy acts on it. Keep the threshold and confidence rule configurable for experiments.

### Keep benchmark dependencies out of the runtime

Google Benchmark belongs to the benchmark target, not the allocator library. This keeps the core dependency-free and makes it possible to build the example without fetching third-party code.

## Questions to settle with measurements

- Does call-site capture provide enough signal to justify its cost?
- Which workload mix benefits from separate temporary and persistent pools?
- How much memory does each strategy retain after the workload ends?
- Does a per-thread cache help enough to justify cross-thread-free handling?
- What confidence threshold makes routing useful without amplifying bad early observations?

Answers belong in the benchmark notes, with the workload and machine recorded. They should not be decided by intuition alone.
