# Debug

| Module | Header | Config | Description |
|---|---|---|---|
| Trace | `debug/syn_trace.h` | `SYN_USE_TRACE` | Circular event trace buffer with timestamps. Record events in production, dump post-mortem or via CLI. |
| Profiler | `debug/syn_profiler.h` | `SYN_USE_PROFILER` | Per-task CPU% measurement and peak tracking. Uses `syn_fmt` for output formatting. |
