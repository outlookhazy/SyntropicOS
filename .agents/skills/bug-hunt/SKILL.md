---
name: bug-hunt
description: >-
  Conduct structured, low-waste C99 bug hunting and security/concurrency audits across SyntropicOS
  modules. Activate when auditing subsystems for defects, race conditions, bounds bugs, or regressions.
---

# SyntropicOS Bug-Hunting Protocol

Systematic audit workflow designed to maximize defect accuracy while preventing redundant file reads, context eviction, and false-positive reports.

## Step 1: Load Ancestor & Subsystem Invariant Maps First

Before reading `.c` implementations, read the applicable `AGENTS.md` maps (`src/syntropic/AGENTS.md`, `src/port/AGENTS.md`, `tests/unit/AGENTS.md`, plus any target subsystem map in `sched/`, `crypto/`, `net/`, `proto/`, or `storage/`) to establish cross-directory wiring and non-obvious invariants.

## Step 2: Disjoint Subagent Partitioning & Context Discipline

- **Single-Module Scope**: Audit directly in the main agent without spawning subagents.
- **Multi-Module / Subsystem Scope**: Partition `research` subagents along **disjoint `.c` ownership boundaries** (never split tightly coupled state machines such as `syn_sched.c` + `syn_mutex.c` across separate subagents). While subagents are running, do not duplicate their full cold-sweep audit in the parent — finish the parent's own assigned scope and **end your turn** (never poll `manage_subagents`, `schedule` timers, or read child transcripts) until their `send_message` reports arrive.
- **Mandatory `research` Subagent Prompt Header**: Built-in `research` subagents run in a read-only sandbox and do **not** inherit workspace `AGENTS.md`. Prepend this header to every `invoke_subagent(TypeName="research", ...)` prompt:

```text
Context & Read Rules:
1. First read_file: <list applicable src/.../AGENTS.md paths>.
2. Assigned scope ONLY: <exact non-overlapping .h/.c list>. Do not audit files outside scope.
3. For any file >800 lines (especially tests/unit/test_*.c and src/port/*), run grep -n first and read_file targeted StartLine..EndLine slices — never page >800-line test or port files end-to-end unless every line is in scope.
4. Read .h API contracts before .c bodies; do not re-read ranges already in context.
```

- **Warm Continuation**: Keep spawned subagents warm and follow up via `send_message` for second-pass trace checks or diff verification instead of launching fresh cold-start subagents.

## Step 3: High-Yield SyntropicOS Defect Focus

Prioritize invariants documented in the target `AGENTS.md` maps and `.h` headers:
1. **ISR, Critical-Section & Callback Reentrancy**: Nested IRQ mask save/restore vs. blind enable; unprotected RMWs; SPSC `SYN_STORE_RELEASE` / `SYN_LOAD_ACQUIRE` pairing; hardware counter rollover races; user callbacks mutating queues/timers/tasks mid-dispatch; non-`static` locals across protothread yields.
2. **32-Bit Tick & Integer Arithmetic**: Modular `(int32_t)(now - deadline) >= 0` comparisons; stale anchors across long `SYN_TASK_SUSPENDED` intervals; `count * elem_size` overflow; signed left-shift UB and `NaN`/`Inf`/`dt <= 0` boundaries.
3. **Untrusted Wire, Media & VM Bounds**: Frame/header/option offset validation prior to pointer math; varint/TLV bounds; compression-pointer and cluster-chain loop limits; byte-stream resync after corrupt headers/CRCs; Classic CAN (`8B`) vs. CAN FD (`64B`) buffer bounds.
4. **Fail-Closed Crypto, Auth & Standalone Gates**: Constant-time secret comparisons; `volatile` zeroization on all error paths; AEAD tag verification before plaintext use; diagnostic/fieldbus session & SecurityAccess checks before state mutation; `#if SYN_USE_* == 0` standalone compilation.

## Step 4: Mandatory Adversarial Falsification Gate

Never report speculative guesses. When subagents return candidate findings, read targeted `StartLine..EndLine` slices around the cited lines in the parent to independently verify them. Every candidate bug **must** pass falsification before reporting:
- Cite exact `file:line` for root cause and interacting callers/callees.
- Provide a concrete step-by-step input, packet, tick, or ISR interleaving trace reaching the faulty state through public APIs.
- Explicitly verify existing preconditions, `.h` documented caller contracts, compile-time guards, and runtime checks do **not** already block the trace. Drop any candidate that fails falsification.
- **Triage Disposition Tag**: Classify every surviving finding as one of:
  1. `[CORE DEFECT]` — Bug inside a supported, documented workflow.
  2. `[EPICYCLE REGRESSION]` — Bug inside complex state/flag machinery added by a prior fix (recommend state reduction or simplification).
  3. `[BOUNDARY / ARCHITECTURAL CHOICE]` — Ambiguous cross-subsystem or out-of-contract interaction (recommend rejecting with `SYN_INVALID_PARAM` + `@pre` or escalating to user rather than inventing new cross-module machinery).
