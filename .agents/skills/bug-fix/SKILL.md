---
name: bug-fix
description: >-
  Diagnose, reproduce, fix, and verify bugs in SyntropicOS using fast incremental Unity tests,
  sibling-module sweeps, and containerized sanitizers. Activate when debugging or fixing defects.
---

# SyntropicOS Bug-Fixing Protocol

Root-cause remediation workflow enforcing fast Red -> Green reproduction, sibling defect sweeps, and targeted dynamic verification.

## Step 1: Architectural Triage, Anti-Epicycle Check & Blast Radius

1. Read the target directory's `AGENTS.md` (`src/syntropic/`, `src/port/`, `tests/unit/`, and any subsystem `AGENTS.md`) plus the module's `.h` header contracts.
2. **Boundary vs. Scope Creep Check**: Before writing code, ask whether the reported scenario belongs inside the module's contract. If fixing a corner case requires inventing new cross-subsystem state or violating AMP/layer isolation, **reject invalid combinations at API entry (`SYN_INVALID_PARAM` + `.h` `@note`/`@pre`) or ask the user** — never silently invent complex cross-module machinery.
3. **Anti-Epicycle Rule**: Prefer **deleting or simplifying state** over adding new booleans, bitflags, or intrusive lists ($2^N \to 2^{N+1}$ state explosion). Whenever adding or modifying a state/flag field (e.g., `sched_flags`, `state`), run `grep -n` for **every read and write** of that field across the module to verify no transition clobbers or leaks it.
4. On files `>800` lines (`tests/unit/test_*.c`, `src/port/*`, large protocol/VM files), use `grep -n` and targeted `StartLine..EndLine` `read_file` slices.
5. Check callers (`grep -rn`) and **sibling modules** sharing the same pattern (e.g., `syn_i2c_queue` <-> `syn_spi_queue`, `syn_tls` <-> `syn_dtls`, `port_stm32f4` <-> `port_stm32_hal` / `port_ch32v307` / `port_rp2040`, Classic CAN <-> CAN FD peers) so identical bugs across siblings are fixed together.

## Step 2: Strict Red -> Green Reproducer & State-Invariant Tests (`make container-unit`)

Execute all edits and container commands in the main agent (never inside read-only `research` sandboxes, and **never run two `make container-*` commands concurrently**):

1. **Red**: Add a minimal, deterministic regression test in `tests/unit/test_<module>.c` (respecting `tests/unit/AGENTS.md` 32-bit ARM QEMU rules). For stateful engines (schedulers, mutexes, timers, queues, protocol state machines), pair point reproducers with a **seeded randomized transition + invariant check** verifying structural invariants hold across arbitrary API/ISR sequences. Run:
   `make container-unit FILTER=<test_function_name>`
   Confirm the new test compiles and **fails** on the unpatched bug (and does not exit with `matched 0 tests`).
2. **Fix**: Implement the minimal C99 root-cause fix in `src/` (zero dynamic allocation, `<= 1024B` stack frame, `V(G) <= 15`). Never weaken assertions or patch symptoms. If clarifying an API precondition or contract, update the `.h` Doxygen `@note` (and subsystem `AGENTS.md` if an architectural contract changed).
3. **Green**: Re-run `make container-unit FILTER=<module>` and confirm `0 Failures`.

## Step 3: Targeted Dynamic Oracle Routing

Run the verification target(s) matching the defect class (strictly serialized, waiting for exit code `0`):
- **Wire/Media/VM Parsers, Bounds, Pointer Math**: `make container-san` (ASan + UBSan) and `make container-fuzz` (when a fuzzer exists in `tests/fuzz/`).
- **Scheduler, Mutex PI, SPSC, ISR/Async Queues**: `make container-tsan` (ThreadSanitizer) and `make container-san`.
- **MCU Ports, Alignment, Endianness, 32-Bit Wraparound**: `make container-qemu`.
- **`#if SYN_USE_*` Feature Gates / Header Dependencies**: `make container-examples`.

## Step 4: Mandatory Completion Gate

Before declaring the fix complete, run and report exact pass/zero-warning summaries for:
1. `make container-format FIX=1` (followed by `make container-format`)
2. `make container-lint` and `make container-static`
3. `make container-dox` (if any `.h`/`.c` comments or signatures changed)
4. `make container-test` (full clean LCOV suite — report exact `X Tests 0 Failures 0 Ignored`)
5. `make container-qemu` (bare-metal Cortex-M4 — report exact `X Tests 0 Failures 0 Ignored`)
