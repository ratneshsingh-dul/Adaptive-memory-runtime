# Adaptive Memory Runtime

A C++20 systems project exploring an adaptive user-space allocator that combines specialized allocation strategies with lightweight online lifetime learning.

## Project goal

Build a measurable, safe, and explainable allocator runtime. It will classify allocation contexts from observed object lifetimes and use those observations to guide future allocations, while preserving conservative fallbacks when confidence is low.

The project will evaluate its claims against the system allocator on documented workloads and hardware. It will not promise a universal speedup or assume a fixed latency target before measurement.

## Status

Planning phase. See [PLAN.md](PLAN.md) for the 20-day implementation and Git cadence.
