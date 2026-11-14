# WebAssembly (WASM) Virtual Machine Module

The `syn_wasm` module provides a 100% zero-heap, non-blocking 32-bit WebAssembly cooperative interpreter designed for embedded microcontrollers. It supports static linear memory arenas (including zero-heap `memory.grow`), zero-copy module and passive data segment loading from Flash memory, full load-time W3C Appendix A.2 bytecode validation, protothread (`SYN_PT`) coroutine host functions, and instruction-sliced execution with cooperative scheduler integration.

---

## Supported WebAssembly Feature Profile

`syn_wasm` implements a strict, spec-tested embedded WebAssembly profile. Every module is validated at load time by `syn_wasm_module_load_ex()` (and `syn_wasm_module_load()`) before a single instruction executes:

| Category | Feature / Proposal | Status | Notes |
|---|---|---|---|
| **Baseline** | WebAssembly 1.0 Core (MVP) | **Supported** | Single return value (`0` or `1` result), `i32`/`i64`/`f32`/`f64`, structured control flow, `call_indirect`, IEEE-754 exact `nearest`/`min`/`max`/`abs`/`neg`/`copysign` |
| **Extension** | `mutable-globals` | **Supported** | Imported and exported mutable/immutable globals; `i32`/`i64`/`f32`/`f64.const` and immutable imported `global.get` initializers via `syn_wasm_init_ex()` |
| **Extension** | `sign-ext` (`0xC0`–`0xC4`) | **Supported** | `i32.extend8_s`, `i32.extend16_s`, `i64.extend8_s`, `i64.extend16_s`, `i64.extend32_s` |
| **Extension** | `nontrapping-fptoint` (`0xFC 0x00`–`0x07`) | **Supported** | Saturating `i32`/`i64.trunc_sat_f32`/`f64_s`/`u` (requires hardware/soft FPU mode, `SYN_WASM_USE_FIXED=0`) |
| **Extension** | `bulk-memory` (single-memory subset) | **Supported** | `memory.copy`, `memory.fill`, `memory.init`, `data.drop`, `DataCount` section, active and zero-copy Flash passive data segments |
| **Excluded** | `threads` / shared memory / atomics (`0xFE`) | **Rejected** | Rejected with `SYN_WASM_LOAD_ERR_UNSUPPORTED` |
| **Excluded** | `simd` (`v128`, `0xFD`) | **Rejected** | Rejected with `SYN_WASM_LOAD_ERR_UNSUPPORTED` |
| **Excluded** | `reference-types` (`ref.*`, `table.*`, multi-table) | **Rejected** | Only single `funcref` table (`0x70`, index `0`) for MVP `call_indirect` is permitted |
| **Excluded** | `multivalue` (`>1` result, `s33` block sigs) | **Rejected** | Rejected with `SYN_WASM_LOAD_ERR_UNSUPPORTED` |
| **Excluded** | `tail-call`, `exception-handling`, `memory64`, `multi-memory`, `gc` (`0xFB`) | **Rejected** | Rejected with `SYN_WASM_LOAD_ERR_UNSUPPORTED` |

When `SYN_WASM_USE_FIXED=1` is enabled for FPU-less microcontrollers, `f32` operations map to `syn_qmath.h` Q16.16 fixed-point arithmetic and `f64` / `trunc_sat` opcodes are rejected at load time with `SYN_WASM_LOAD_ERR_UNSUPPORTED`.

---

## Recommended C-to-WASM Compilation Command

Modern LLVM/Clang (Clang 19+) defaults to `-mcpu=generic`, which implicitly enables WebAssembly 2.0+ proposals (`+reference-types`, `+multivalue`) outside the embedded profile. Always pin `-mcpu=mvp` and enable only the supported extensions explicitly:

```bash
clang --target=wasm32-unknown-unknown -mcpu=mvp -O2 -nostdlib \
  -mmutable-globals -msign-ext -mbulk-memory -mnontrapping-fptoint \
  -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined \
  -Wl,-z,stack-size=1024 app.c -o app.wasm
```

> **Note (`SYN_WASM_USE_FIXED=1` targets):** Omit `-mnontrapping-fptoint` and avoid `double` (`f64`) types when compiling for fixed-point VM builds.

### Compiler Flags Summary

| Flag | Purpose |
|---|---|
| `--target=wasm32-unknown-unknown` | Targets freestanding 32-bit WebAssembly |
| `-mcpu=mvp` | Pins LLVM baseline to WebAssembly 1.0 MVP so excluded 2.0+ extensions (`reference-types`, `multivalue`, `simd`) are never emitted implicitly |
| `-O2 -nostdlib` | Optimization level 2 with zero C standard library bloat |
| `-mmutable-globals` | Enables mutable global imports/exports (used by LLVM's shadow stack pointer) |
| `-msign-ext` | Enables `0xC0`–`0xC4` in-register sign-extension instructions |
| `-mbulk-memory` | Enables `0xFC` bulk memory instructions (`memory.copy`, `memory.fill`, `memory.init`, `data.drop`) |
| `-mnontrapping-fptoint` | Enables `0xFC 0x00`–`0x07` saturating float-to-int conversions |
| `-Wl,--no-entry` | Omits standard `_start` entry point requirement |
| `-Wl,--export-all` | Exports C functions for lookup via `syn_wasm_find_export()` |
| `-Wl,--allow-undefined` | Permits `extern` host function declarations resolved via `syn_wasm_register_host()` |
| `-Wl,-z,stack-size=1024` | Bounds LLVM's linear-memory shadow stack to 1 KB for embedded SRAM budgets |

---

## Configuration Options

All VM resource bounds are `#ifndef`-guarded in `syn_wasm.h` and can be overridden in `syn_config.h` or via compiler `-D` flags:

```c
#define SYN_WASM_MAX_STACK        64   /* Operand stack depth */
#define SYN_WASM_MAX_LOCALS      256   /* Total local variables across active call frames */
#define SYN_WASM_MAX_CALL_DEPTH   32   /* Maximum function call frame depth */
#define SYN_WASM_MAX_FUNCTIONS    32   /* Max imported + internal functions per module */
#define SYN_WASM_MAX_HOST_FUNCS   16   /* Max registered C host callbacks */
#define SYN_WASM_MAX_HOST_ARGS    16   /* Max parameters on an imported host function */
#define SYN_WASM_MAX_GLOBALS      16   /* Max imported + module-defined globals */
#define SYN_WASM_MAX_LABELS       16   /* Max nested structured control-flow blocks */
#define SYN_WASM_MAX_TYPES        64   /* Max function type signatures per module */
#define SYN_WASM_MAX_EXPORTS      32   /* Max exported items per module */
#define SYN_WASM_MAX_TABLE_ELEMS  64   /* Max indirect function table slots */
#define SYN_WASM_MAX_DATA_SEGS    16   /* Max active/passive data segments tracked */
#define SYN_WASM_DEFAULT_QUANTUM  64U  /* Default instructions per cooperative slice */
```

Modules exceeding any configured table capacity fail closed during `syn_wasm_module_load_ex()` with `SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED`.

### Static-Arena Linear Memory & `memory.grow`

`syn_wasm_init(&ctx, &mod, linear_mem, mem_size)` (and `syn_wasm_init_ex()`) treats `linear_mem` of `mem_size` bytes as the fixed static backing arena:
- Initial accessible linear memory (`ctx.linear_mem_size`) is set to `mod.mem_min_pages * 65536` bytes (when `mod.has_memory` and `mem_size` covers the declared minimum) and zero-initialized.
- `memory.grow` zero-fills and expands `ctx.linear_mem_size` in 64 KiB pages within `mem_size` (and `mod.mem_max_pages`, if declared) with **zero heap allocation**, returning `-1` (`0xFFFFFFFF`) when growth would exceed either bound.

---

## Coroutine Host Functions & Scheduler Integration

Imported host functions are `SYN_PT` protothread coroutines rather than blocking C callbacks. Guest WebAssembly code invokes blocking-style imports (`sleep_ms(100)`, `uart_read(buf, len)`, mutex locks) with normal `call` / `call_indirect` instructions while cooperatively yielding to `SYN_Sched`.

### Signatures & Registration

```c
typedef SYN_PT_Status (*SYN_WASM_HostFunc)(SYN_PT *pt, SYN_WASM_Context *ctx,
                                           const uint64_t *args, uint8_t argc,
                                           uint64_t *ret);
typedef void (*SYN_WASM_HostCancelFunc)(SYN_WASM_Context *ctx, void *user);

bool syn_wasm_register_host(SYN_WASM_Context *ctx, uint16_t import_index,
                            SYN_WASM_HostFunc func);
bool syn_wasm_register_host_ex(SYN_WASM_Context *ctx, uint16_t import_index,
                               SYN_WASM_HostFunc func,
                               SYN_WASM_HostCancelFunc cancel, void *user);
void *syn_wasm_host_user(const SYN_WASM_Context *ctx);
```

- **Argument Lifetime & Scratch State**: Before the first poll of an imported call, `syn_wasm` pops `argc` operands into `ctx->host_args[]` (`argc <= SYN_WASM_MAX_HOST_ARGS`, enforced at module load time with `SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED`), zeroes `ctx->host_scratch` (`u32[4]` / `u64[2]` / `ptr` / `child_pt`), and initializes `ctx->host_pt`. Never keep non-`static` C stack locals live across `PT_*` yield points; store cross-yield state in `ctx->host_scratch` or per-import `user` state (`syn_wasm_host_user(ctx)`).
- **Synchronous Convenience Macros**: Non-yielding host callbacks use `SYN_WASM_HOST_SYNC(name, ctx_var, args_var, argc_var) { ... }` or `SYN_WASM_HOST_RETURN(pt, ret_ptr, val_expr)`.
- **Poll Status Mapping**:
  - `PT_ENDED`: Clears `ctx->host_pending`, pushes `*ret` if the import declares a return value, and **continues executing guest WebAssembly instructions in the same `syn_wasm_step()` slice** (or transitions `ctx->status = SYN_WASM_HALTED` when invoked directly at `call_depth == 0`).
  - `PT_IDLE`: Keeps `ctx->host_pending = true` and `ctx->status = SYN_WASM_OK`; `syn_wasm_step()` returns `SYN_WASM_WAITING`, and `syn_wasm_task()` / `syn_wasm_exec_pt()` yield `PT_PASS(pt)` (`PT_IDLE`).
  - `PT_MORE`: Keeps `ctx->host_pending = true` and `ctx->status = SYN_WASM_OK`; `syn_wasm_step()` returns `SYN_WASM_YIELDED`, `syn_wasm_task()` yields via `PT_CONTINUE_IF_NEEDED(pt, task)` (bursting uncontended or sharing when peers are ready), and `syn_wasm_exec_pt()` yields `PT_CONTINUE(pt)`.
  - `PT_EXITED`: Clears `ctx->host_pending` and sets `ctx->status = SYN_WASM_TRAP_HOST` if no specific `SYN_WASM_TRAP_*` code was already assigned by the callback.

### Owning `SYN_Task *` Propagation (`ctx->active_task`)

`syn_wasm_task()` and `syn_wasm_exec_pt(pt, task, ctx, quantum_instructions)` record the owning `SYN_Task *` in `ctx->active_task` (`NULL` when stepped standalone):
- **True Tickless Sleep**: Use `PT_TASK_DELAY_MS(pt, ctx->active_task, ms)` when `ctx->active_task != NULL` (falling back to `PT_DELAY_MS(pt, &ctx->host_scratch.u32[0], ms)` when `NULL`) so `task->delay_until` is armed and the scheduler skips polling the task until wakeup.
- **Priority-Inheriting Mutexes**: Use `PT_MUTEX_LOCK(pt, ctx->active_task, &mutex)` and `PT_MUTEX_UNLOCK(&mutex, ctx->active_task)` so contended locks boost lower-priority owners.

### Bounds-Checked Guest Memory, Cancellation & Built-In Host Coroutines

- **Linear Memory Accessors**: `syn_wasm_mem_ptr(ctx, offset, size)` and `syn_wasm_mem_cptr(ctx, offset, size)` perform `uint64_t` overflow-safe bounds checks against `ctx->linear_mem_size` and return `NULL` on invalid ranges. Because `ctx->linear_mem` is a fixed static arena (`memory.grow` expands `linear_mem_size` in place without reallocation), validated pointers remain stable across yields.
- **Pending Call Cancellation**: `syn_wasm_call()` returns `false` while `ctx->host_pending` is `true`. Call `syn_wasm_abort(ctx)` to invoke any registered `SYN_WASM_HostCancelFunc`, clear pending host state and `active_task->delay_until`, unwind the VM stack, and set `ctx->status = SYN_WASM_HALTED`.
- **Built-In Standard Host Coroutines**: `syn_wasm_host_millis`, `syn_wasm_host_micros`, `syn_wasm_host_sleep_ms`, and `syn_wasm_host_yield` are provided out of the box for direct registration via `syn_wasm_register_host()`.

---

## Code Example

```c
#include <syntropic/sched/syn_sched.h>
#include <syntropic/vm/syn_wasm.h>
#include <stdio.h>
#include <string.h>

static uint8_t wasm_ram[65536]; /* 1 WebAssembly page (64 KiB) static arena */
static SYN_WASM_Module mod;
static SYN_WASM_Context ctx;

/* Synchronous host function using SYN_WASM_HOST_SYNC */
SYN_WASM_HOST_SYNC(host_log_cb, c, args, argc) {
    (void)c;
    (void)argc;
    printf("[WASM LOG] Code=%u, Val=%u\n", (uint32_t)args[0], (uint32_t)args[1]);
    return args[0] + args[1];
}

/* Yielding host coroutine writing into bounds-checked guest linear memory */
static SYN_PT_Status host_read_sensor_cb(SYN_PT *pt, SYN_WASM_Context *c,
                                         const uint64_t *args, uint8_t argc, uint64_t *ret) {
    if ((argc < 2U) || (syn_wasm_mem_ptr(c, (uint32_t)args[0], (uint32_t)args[1]) == NULL)) {
        c->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
        return PT_EXITED;
    }

    PT_BEGIN(pt);
    /* Non-spinning scheduler delay while sensor conversion completes */
    if (c->active_task != NULL) {
        PT_TASK_DELAY_MS(pt, c->active_task, 10U);
    } else {
        PT_DELAY_MS(pt, &c->host_scratch.u32[0], 10U);
    }
    {
        uint8_t *dst = (uint8_t *)syn_wasm_mem_ptr(c, (uint32_t)args[0], (uint32_t)args[1]);
        memset(dst, 0x42, (size_t)args[1]);
    }
    *ret = args[1];
    PT_END(pt);
}

bool start_wasm_task(SYN_Sched *sched, SYN_Task *wasm_task,
                     const uint8_t *wasm_bytes, size_t wasm_size) {
    SYN_WASM_LoadError load_err = SYN_WASM_LOAD_OK;
    if (!syn_wasm_module_load_ex(&mod, wasm_bytes, (uint32_t)wasm_size, &load_err)) {
        printf("WASM load rejected: %s\n", syn_wasm_load_error_str(load_err));
        return false;
    }

    if (!syn_wasm_init(&ctx, &mod, wasm_ram, sizeof(wasm_ram))) {
        return false;
    }

    syn_wasm_register_host(&ctx, 0U, host_log_cb);
    syn_wasm_register_host(&ctx, 1U, syn_wasm_host_sleep_ms);
    syn_wasm_register_host(&ctx, 2U, host_read_sensor_cb);

    int32_t fn_idx = syn_wasm_find_export(&mod, "run_app");
    if ((fn_idx < 0) || !syn_wasm_call(&ctx, (uint16_t)fn_idx)) {
        return false;
    }

    syn_task_create(wasm_task, "wasm_app", syn_wasm_task, SYN_PRIO_NORMAL, &ctx);
    syn_sched_init(sched, wasm_task, 1U);
    return true;
}
```

---

## Official W3C WebAssembly Spec Conformance Suite

`syn_wasm` is verified against the official W3C `WebAssembly/spec` test suite (`wg-1.0` core + `wg-2.0` float, `sign-ext`, `nontrapping-fptoint`, `mutable-globals`, single-memory `bulk-memory`, and negative excluded-extension vectors) under Clang ASan + UBSan:

```bash
make -C tools/containers container-wasm-spec
```

