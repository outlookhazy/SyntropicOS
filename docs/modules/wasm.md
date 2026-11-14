# WebAssembly (WASM) Virtual Machine Module

The `syn_wasm` module provides a 100% zero-heap, non-blocking 32-bit WebAssembly cooperative interpreter designed for embedded microcontrollers. It supports static linear memory arenas (including zero-heap `memory.grow`), zero-copy module and passive data segment loading from Flash memory, full load-time W3C Appendix A.2 bytecode validation, host function registration, and instruction-sliced execution with protothread (`syn_pt`) scheduling.

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

## Code Example

```c
#include <syntropic/vm/syn_wasm.h>
#include <stdio.h>

static uint8_t wasm_ram[65536]; /* 1 WebAssembly page (64 KiB) static arena */
static SYN_WASM_Module mod;
static SYN_WASM_Context ctx;

/* Host function callback signature */
static uint64_t host_log_cb(SYN_WASM_Context *c, const uint64_t *args, uint8_t argc) {
    (void)c;
    (void)argc;
    printf("[WASM LOG] Code=%u, Val=%u\n", (uint32_t)args[0], (uint32_t)args[1]);
    return args[0] + args[1];
}

void run_wasm_app(const uint8_t *wasm_bytes, size_t wasm_size) {
    /* 1. Parse and validate module header, sections, feature profile, and bytecode */
    SYN_WASM_LoadError load_err = SYN_WASM_LOAD_OK;
    if (!syn_wasm_module_load_ex(&mod, wasm_bytes, (uint32_t)wasm_size, &load_err)) {
        printf("WASM load rejected: %s\n", syn_wasm_load_error_str(load_err));
        return;
    }

    /* 2. Instantiate execution context with static RAM arena */
    if (!syn_wasm_init(&ctx, &mod, wasm_ram, sizeof(wasm_ram))) {
        printf("Error: WASM instantiation failed (trap=%d)\n", ctx.status);
        return;
    }

    /* 3. Register native host callback at import index 0 */
    syn_wasm_register_host(&ctx, 0, host_log_cb);

    /* 4. Find exported function "run_tests" */
    int32_t fn_idx = syn_wasm_find_export(&mod, "run_tests");
    if (fn_idx < 0) {
        printf("Error: Exported function 'run_tests' not found\n");
        return;
    }

    /* 5. Set up call frame and execute with instruction slicing
     *    (Inside a SyntropicOS task, prefer PT_SPAWN_IF_NEEDED with syn_wasm_exec_pt()
     *    or run syn_wasm_task directly on the cooperative scheduler.) */
    if (!syn_wasm_call(&ctx, (uint16_t)fn_idx)) {
        printf("Error: WASM call setup failed (trap=%d)\n", ctx.status);
        return;
    }

    SYN_WASM_Status st;
    do {
        st = syn_wasm_step(&ctx, 100);
    } while (st == SYN_WASM_YIELDED);

    if (st == SYN_WASM_HALTED) {
        uint32_t result = (uint32_t)syn_wasm_result(&ctx);
        printf("WASM execution completed. Result: %u\n", result);
    } else {
        printf("WASM execution trapped! Trap code: %d\n", st);
    }
}
```

---

## Official W3C WebAssembly Spec Conformance Suite

`syn_wasm` is verified against the official W3C `WebAssembly/spec` test suite (`wg-1.0` core + `wg-2.0` float, `sign-ext`, `nontrapping-fptoint`, `mutable-globals`, single-memory `bulk-memory`, and negative excluded-extension vectors) under Clang ASan + UBSan:

```bash
make -C tools/containers container-wasm-spec
```

