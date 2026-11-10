# Portable Module Architecture & Invariants (`src/syntropic/`)

Applies to all portable modules under `src/syntropic/`.

## Cross-Directory Wiring

- **Foundation (`common/`)**: `syn_defs.h`, `syn_compiler.h`, and `syn_barrier.h` (`SYN_LOAD_ACQUIRE`, `SYN_STORE_RELEASE`).
- **Port Contracts (`port/`)**: Hardware HAL headers (`syn_port_*.h`); weak asserting stubs live in `port_stubs/syn_port_stubs.c`; MCU implementations live in `src/port/<platform>/` (plus `src/port/syn_port_eth.h`, `syn_port_usb.h`, `syn_port_usb_host.h`).
- **Coroutines & Kernel (`pt/`, `sched/`, `system/`)**: 2-byte stackless protothreads (`pt/syn_pt.h`, `pt/syn_pt_sem.h`), cooperative scheduler (`sched/`), tickless sleep coordinator (`system/syn_sleep.h`), and event signaling (`util/syn_event.h`).
- **Feature Gates**: Optional subsystems compile conditionally via `SYN_USE_*` flags templated in `src/syntropic/syn_config_template.h` and aggregated in `src/syntropic/syntropic.h`.

## Portable C99 Module Laws

- **Zero Dynamic Allocation & Bounded Stack**: Never call `malloc`/`calloc`/`realloc`/`free` or declare VLAs. Keep per-function static stack frames `<= 1024B` (`<= 1920B` in ECC bignum paths) and cyclomatic complexity `V(G) <= 15`.
- **Reentrant User Callback Safety**: User completion, timer, and event callbacks may re-enter public APIs (`init`, `enqueue`, `cancel`, `stop`, `restart`) on the same instance. Copy active descriptor/callback pointers to stack locals and advance internal state *before* invoking callbacks; never dereference mutable slot pointers afterward without re-validating.
- **Protothread State Lifetime (`pt/`)**: Non-`static` local variables do not survive `PT_WAIT_*`, `PT_YIELD`, `PT_PASS`, or `PT_CONTINUE*` yields. Persist state in caller-owned context structs (`task->user_data`), never place bare `switch` statements inside `PT_BEGIN`..`PT_END`, and preserve `SYN_PT_STATE_BARRIER()` ordering.
- **ISR & Multicore Memory Ordering**: Shared SPSC indices and lock-free flags across ISR/task or AMP cores (`SYN_USE_MULTICORE`) must pair `SYN_STORE_RELEASE` after payload writes with `SYN_LOAD_ACQUIRE` before payload reads. Protect multi-field RMWs with `syn_port_enter_critical()` / `syn_port_exit_critical()`.
- **Fail-Closed Firmware & Untrusted Inputs (`system/`, `vm/`, `ble/`, `drivers/`)**: Verify cryptographic tags/signatures completely before trusting, committing, or booting decrypted payloads (`syn_fwupdate`, `syn_ota`, `syn_fwboot`). Validate untrusted wire/varint/descriptor lengths and `count * elem_size` multiplication overflow before pointer arithmetic or indexing.
- **Numerics, DSP & Control (`control/`, `dsp/`, `motor/`, `audio/`, `util/`)**: Guard against `NaN`/`Inf`, `dt <= 0`, zero sample rates, divide-by-zero, and signed left-shift UB in Q7/Q15/Q31 fixed-point paths; clamp/saturate actuator outputs on fault or failsafe.
- **Standalone Modularity**: Modules must compile cleanly both with all features enabled and in lean profiles where peer `SYN_USE_*` flags are `0` (verified via `make container-examples`).
