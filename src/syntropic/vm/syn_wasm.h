/**
 * @file syn_wasm.h
 * @brief [EXPERIMENTAL] Zero-heap, 32-bit WebAssembly cooperative interpreter.
 *
 * @warning EXPERIMENTAL MODULE - API and bytecode execution semantics subject to change.
 *
 * Implements a 100% zero-malloc, non-blocking 32-bit WebAssembly interpreter
 * designed for embedded microcontrollers. Supports static linear memory arrays,
 * zero-copy parsing and load-time validation from flash, host function registration,
 * and instruction-sliced cooperative execution with protothread (`syn_pt`) integration.
 *
 * Supported WebAssembly feature profile (validated at module load):
 *   - WebAssembly 1.0 Core (MVP): i32/i64/f32/f64, structured control flow,
 *     single linear memory (index 0), single funcref table (index 0) with
 *     signature-checked `call_indirect`, and multi-result function types.
 *   - `mutable-globals`: imported and internal mutable globals.
 *   - `sign-ext`: `i32.extend8_s`, `i32.extend16_s`, `i64.extend8_s`,
 *     `i64.extend16_s`, `i64.extend32_s` (`0xC0`–`0xC4`).
 *   - `nontrapping-fptoint`: saturating float-to-int conversions (`0xFC 0x00`–`0x07`).
 *   - `bulk-memory` (memory 0 subset): `memory.copy`, `memory.fill`, `memory.init`, `data.drop`.
 *
 * Unsupported extensions (rejected at compile time via `-mcpu=mvp` and at load time):
 *   - `threads` / shared memory / atomics (`0xFE`), `simd` (`0xFD`),
 *     `reference-types`, `multivalue` block signatures, `tail-call`,
 *     `exception-handling`, `memory64`, `multi-memory`, and `gc`.
 *
 * Recommended C-to-WASM compilation command (pins Clang to the supported profile):
 *   clang --target=wasm32-unknown-unknown -mcpu=mvp -O2 -nostdlib \
 *     -mmutable-globals -msign-ext -mbulk-memory -mnontrapping-fptoint \
 *     -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined \
 *     -Wl,-z,stack-size=1024 app.c -o app.wasm
 */

#ifndef SYN_WASM_H
#define SYN_WASM_H

#include "../common/syn_defs.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_sched.h"
#include "../util/syn_qmath.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @name WebAssembly VM Execution Limit Constants */
/**@{*/
#ifndef SYN_WASM_MAX_STACK
#define SYN_WASM_MAX_STACK 64 /**< Maximum operand stack depth */
#endif

#ifndef SYN_WASM_MAX_LOCALS
#define SYN_WASM_MAX_LOCALS 256 /**< Maximum local variable count per frame */
#endif

#ifndef SYN_WASM_MAX_CALL_DEPTH
#define SYN_WASM_MAX_CALL_DEPTH 32 /**< Maximum call frame stack depth */
#endif

#ifndef SYN_WASM_MAX_FUNCTIONS
#define SYN_WASM_MAX_FUNCTIONS 32 /**< Maximum internal function defs */
#endif

#ifndef SYN_WASM_MAX_HOST_FUNCS
#define SYN_WASM_MAX_HOST_FUNCS 16 /**< Maximum registered host functions */
#endif

#ifndef SYN_WASM_MAX_HOST_ARGS
#define SYN_WASM_MAX_HOST_ARGS 16 /**< Maximum argument count for imported host functions */
#endif

#if (SYN_WASM_MAX_HOST_ARGS < 1) || (SYN_WASM_MAX_HOST_ARGS > 255)
#error "SYN_WASM_MAX_HOST_ARGS must be between 1 and 255"
#endif

#ifndef SYN_WASM_MAX_GLOBALS
#define SYN_WASM_MAX_GLOBALS 16 /**< Maximum global variables */
#endif

#ifndef SYN_WASM_MAX_LABELS
#define SYN_WASM_MAX_LABELS 16 /**< Maximum block/loop control labels */
#endif

#ifndef SYN_WASM_MAX_TYPES
#define SYN_WASM_MAX_TYPES 64 /**< Maximum function type signatures per module */
#endif

#ifndef SYN_WASM_MAX_EXPORTS
#define SYN_WASM_MAX_EXPORTS SYN_WASM_MAX_FUNCTIONS /**< Maximum exported items per module */
#endif

#ifndef SYN_WASM_MAX_TABLE_ELEMS
#define SYN_WASM_MAX_TABLE_ELEMS 64 /**< Maximum indirect function table slots */
#endif

#ifndef SYN_WASM_MAX_DATA_SEGS
#define SYN_WASM_MAX_DATA_SEGS 16 /**< Maximum data segments per module */
#endif

#ifndef SYN_WASM_DEFAULT_QUANTUM
#define SYN_WASM_DEFAULT_QUANTUM 64U /**< Default instruction slice per cooperative quantum */
#endif
/**@}*/

/* ── Status Codes & Traps ───────────────────────────────────────────────── */

/** @brief WebAssembly VM execution status codes and traps */
typedef enum {
    SYN_WASM_OK = 0,                   /**< Execution completed normally */
    SYN_WASM_YIELDED,                  /**< Execution yielded after instruction quota or PT_MORE */
    SYN_WASM_WAITING,                  /**< Execution parked waiting on async host coroutine */
    SYN_WASM_HALTED,                   /**< VM halted */
    SYN_WASM_TRAP_STACK_OVERFLOW,      /**< Operand stack overflow trap */
    SYN_WASM_TRAP_STACK_UNDERFLOW,     /**< Operand stack underflow trap */
    SYN_WASM_TRAP_OUT_OF_BOUNDS,       /**< Memory access out of bounds trap */
    SYN_WASM_TRAP_BAD_OPCODE,          /**< Invalid or unsupported opcode trap */
    SYN_WASM_TRAP_DIV_ZERO,            /**< Division by zero trap */
    SYN_WASM_TRAP_UNREACHABLE,         /**< Unreachable instruction trap */
    SYN_WASM_TRAP_CALL_STACK_OVERFLOW, /**< Call stack overflow trap */
    SYN_WASM_TRAP_TYPE_MISMATCH,       /**< Type mismatch trap */
    SYN_WASM_TRAP_INVALID_MODULE,      /**< Invalid Wasm module header/binary */
    SYN_WASM_TRAP_UNREGISTERED_HOST,   /**< Unregistered host function trap */
    SYN_WASM_TRAP_HOST                 /**< Host coroutine aborted via PT_EXIT */
} SYN_WASM_Status;

/** @brief Sentinel value for an uninitialized indirect function table slot */
#define SYN_WASM_TABLE_NULL 0xFFFFU

/** @brief Load-time module validation result and rejection reason */
typedef enum {
    SYN_WASM_LOAD_OK = 0,             /**< Module is valid and within supported profile */
    SYN_WASM_LOAD_ERR_INVALID_ARG,    /**< NULL pointer or buffer smaller than 8-byte header */
    SYN_WASM_LOAD_ERR_BAD_HEADER,     /**< Invalid Wasm magic ("\0asm") or version (!= 1) */
    SYN_WASM_LOAD_ERR_MALFORMED,      /**< Truncated section, invalid LEB128, or invalid bytecode */
    SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED, /**< Module exceeds static VM resource limits */
    SYN_WASM_LOAD_ERR_UNSUPPORTED     /**< Module uses unsupported Wasm extension or opcode */
} SYN_WASM_LoadError;

/* Forward declarations */
typedef struct SYN_WASM_Context_s SYN_WASM_Context;

/**
 * @brief Protothread coroutine host function signature callable from Wasm bytecode.
 *
 * Polled with `ctx->host_pt` until returning `PT_ENDED` (normal completion) or
 * `PT_EXITED` (host trap). Yielding `PT_IDLE` suspends the guest call with
 * `SYN_WASM_WAITING`, while yielding `PT_MORE` yields a cooperative chunk with
 * `SYN_WASM_YIELDED`. Arguments in @p args are copied into `ctx->host_args` and
 * remain valid across yields. @p ret is always non-NULL and zero-initialized when
 * invoked by the VM.
 *
 * @param pt   Host coroutine protothread control block (`&ctx->host_pt`).
 * @param ctx  Pointer to Wasm execution context.
 * @param args Copied array of 64-bit argument slots (`NULL` when `argc == 0`).
 * @param argc Number of arguments (`<= SYN_WASM_MAX_HOST_ARGS`).
 * @param ret  Output slot for 64-bit return value when `result_count > 0`.
 * @return `PT_ENDED` on completion, `PT_IDLE` / `PT_MORE` while suspended, or `PT_EXITED` on trap.
 */
typedef SYN_PT_Status (*SYN_WASM_HostFunc)(SYN_PT *pt, SYN_WASM_Context *ctx, const uint64_t *args,
                                           uint8_t argc, uint64_t *ret);

/**
 * @brief Optional cleanup callback invoked by syn_wasm_abort() when a pending host call is
 * cancelled.
 *
 * @param ctx  Pointer to Wasm execution context.
 * @param user Per-import user pointer registered via syn_wasm_register_host_ex().
 */
typedef void (*SYN_WASM_HostCancelFunc)(SYN_WASM_Context *ctx, void *user);

/**
 * @brief Complete a synchronous Wasm host coroutine with a 64-bit return value.
 *
 * Resets @p pt, writes @p val_expr to `*(ret_ptr)` when non-NULL, and returns `PT_ENDED`.
 *
 * @param pt       Host protothread control block (`SYN_PT *`).
 * @param ret_ptr  Output return slot pointer (`uint64_t *`).
 * @param val_expr Return value expression convertible to `uint64_t`.
 */
#define SYN_WASM_HOST_RETURN(pt, ret_ptr, val_expr)    \
    do {                                               \
        uint64_t syn_wasm_ret_ = (uint64_t)(val_expr); \
        if ((pt) != NULL) {                            \
            PT_INIT(pt);                               \
        }                                              \
        if ((ret_ptr) != NULL) {                       \
            *(ret_ptr) = syn_wasm_ret_;                \
        }                                              \
        return PT_ENDED;                               \
    } while (0)

/**
 * @brief Define a `static` non-yielding Wasm host function returning `uint64_t`.
 *
 * Generates a `SYN_WASM_HostFunc`-compatible `static` wrapper named @p name around a
 * synchronous C function body so one-shot host callbacks remain concise.
 *
 * @param name     Identifier of the generated `static` `SYN_WASM_HostFunc`.
 * @param ctx_var  Parameter name for `SYN_WASM_Context *`.
 * @param args_var Parameter name for `const uint64_t *`.
 * @param argc_var Parameter name for `uint8_t`.
 */
#define SYN_WASM_HOST_SYNC(name, ctx_var, args_var, argc_var)                                   \
    static inline uint64_t name##_sync_impl_(SYN_WASM_Context *(ctx_var),                       \
                                             const uint64_t *(args_var), uint8_t argc_var);     \
    static SYN_PT_Status name(SYN_PT *syn_wasm_pt_, SYN_WASM_Context *syn_wasm_ctx_,            \
                              const uint64_t *syn_wasm_args_, uint8_t syn_wasm_argc_,           \
                              uint64_t *syn_wasm_ret_ptr_)                                      \
    {                                                                                           \
        SYN_WASM_HOST_RETURN(syn_wasm_pt_, syn_wasm_ret_ptr_,                                   \
                             name##_sync_impl_(syn_wasm_ctx_, syn_wasm_args_, syn_wasm_argc_)); \
    }                                                                                           \
    static inline uint64_t name##_sync_impl_(SYN_WASM_Context *(ctx_var),                       \
                                             const uint64_t *(args_var), uint8_t argc_var)

/** @brief Per-call scratch storage preserved across host coroutine yields */
typedef union {
    uint64_t u64[2]; /**< 64-bit scratch slots */
    uint32_t u32[4]; /**< 32-bit scratch slots (e.g., standalone PT_DELAY_MS target) */
    uint8_t u8[16];  /**< Byte scratch buffer */
    struct {
        SYN_PT child_pt; /**< Child protothread control block for nested PT_SPAWN */
        uint16_t u16[7]; /**< Auxiliary 16-bit counters alongside child_pt */
    } pt;                /**< Nested protothread + counter layout */
    SYN_PT child_pt;     /**< Direct child protothread control block */
} SYN_WASM_HostScratch;

/** @brief WebAssembly function signature descriptor (points into module bytes) */
typedef struct {
    uint32_t params_offset;  /**< Byte offset of parameter valtype vector in module */
    uint32_t results_offset; /**< Byte offset of result valtype vector in module */
    uint8_t param_count;     /**< Number of input parameters */
    uint8_t result_count;    /**< Number of return values */
} SYN_WASM_FuncType;

/** @brief WebAssembly function definition metadata */
typedef struct {
    uint32_t type_idx;    /**< Type section index */
    uint32_t code_offset; /**< Code offset in module bytes */
    uint32_t code_size;   /**< Size of bytecode instructions */
    uint8_t param_count;  /**< Number of input parameters */
    uint8_t result_count; /**< Number of return values */
} SYN_WASM_FuncDef;

/** @brief Parsed WebAssembly module header and export registry */
typedef struct {
    const uint8_t *bytes; /**< Pointer to raw WebAssembly binary */
    uint32_t size;        /**< Size of binary in bytes */

    SYN_WASM_FuncType types[SYN_WASM_MAX_TYPES]; /**< Function signature registry */
    uint16_t type_count;                         /**< Parsed function signature count */

    SYN_WASM_FuncDef funcs[SYN_WASM_MAX_FUNCTIONS]; /**< Function registry */
    uint16_t func_count;                            /**< Total function count */
    uint16_t import_func_count;                     /**< Imported host function count */
    uint16_t import_global_count;                   /**< Imported global variable count */

    struct {
        uint32_t name_offset;        /**< Export name offset */
        uint16_t name_len;           /**< Export name length */
        uint16_t func_idx;           /**< Export target index (function/table/memory/global) */
        uint8_t kind;                /**< Export kind: 0=func, 1=table, 2=memory, 3=global */
    } exports[SYN_WASM_MAX_EXPORTS]; /**< Export registry */
    uint16_t export_count;           /**< Export count */

    uint16_t table_elements[SYN_WASM_MAX_TABLE_ELEMS]; /**< Element table for indirect calls */
    uint16_t table_element_count;                      /**< Element count */
    uint32_t table_min_elems;                          /**< Declared minimum table size */
    uint32_t table_max_elems;                          /**< Declared maximum table size */
    bool has_table;                                    /**< True if module declares/imports table */
    bool table_has_max;                                /**< True if table has an upper limit */

    uint32_t mem_min_pages; /**< Declared minimum linear memory size in 64 KiB pages */
    uint32_t mem_max_pages; /**< Declared maximum linear memory size in 64 KiB pages */
    bool has_memory;        /**< True if module declares/imports linear memory */
    bool mem_has_max;       /**< True if linear memory has an upper page limit */

    struct {
        uint32_t offset;                 /**< Byte offset of passive payload in module bytes */
        uint32_t size;                   /**< Byte length of passive payload (0 if active) */
        bool passive;                    /**< True if passive segment usable by memory.init */
    } data_segs[SYN_WASM_MAX_DATA_SEGS]; /**< Data segment descriptors */
    uint16_t data_seg_count;             /**< Parsed data segment count */

    uint32_t start_func_idx;       /**< Start function index */
    bool has_start_func;           /**< True if module has start function */
    SYN_WASM_LoadError load_error; /**< Most recent load-time validation status */
} SYN_WASM_Module;

/* ── Runtime Call Frame & Label Stacks ───────────────────────────────────── */

/** @brief WebAssembly runtime call frame */
typedef struct {
    uint16_t func_idx;   /**< Called function index */
    uint32_t return_pc;  /**< Return program counter */
    uint32_t frame_sp;   /**< Frame operand stack pointer */
    uint16_t local_base; /**< Base index in locals array */
    uint16_t label_base; /**< Caller label stack depth on entry */
} SYN_WASM_CallFrame;

/** @brief WebAssembly control flow label stack entry */
typedef struct {
    uint8_t opcode;     /**< Opcode: 0x02 block, 0x03 loop, 0x04 if */
    uint8_t arity;      /**< Branch operand arity (0 for void/loop, 1 for typed block/if) */
    uint32_t target_pc; /**< Target program counter on break */
    uint32_t stack_sp;  /**< Operand stack pointer on entry */
} SYN_WASM_Label;

/* ── Runtime Execution Context ────────────────────────────────────────────── */

/** @brief WebAssembly virtual machine execution context */
struct SYN_WASM_Context_s {
    const SYN_WASM_Module *module; /**< Pointer to target Wasm module */

    uint32_t pc;                        /**< Program counter opcode offset */
    uint32_t sp;                        /**< Operand stack pointer */
    uint64_t stack[SYN_WASM_MAX_STACK]; /**< Operand stack array */

    uint64_t locals[SYN_WASM_MAX_LOCALS]; /**< Local variables array */
    uint16_t local_count;                 /**< Active local variable count */

    SYN_WASM_CallFrame call_stack[SYN_WASM_MAX_CALL_DEPTH]; /**< Call frame stack array */
    uint16_t call_depth;                                    /**< Active call depth */

    SYN_WASM_Label label_stack[SYN_WASM_MAX_LABELS]; /**< Control block label stack array */
    uint16_t label_depth;                            /**< Active label depth */

    uint64_t globals[SYN_WASM_MAX_GLOBALS]; /**< Global variables array */
    uint16_t global_count;                  /**< Global variable count */

    uint16_t table_elements[SYN_WASM_MAX_TABLE_ELEMS]; /**< Instance indirect call table */
    uint16_t table_element_count;                      /**< Populated indirect call table bound */

    uint64_t data_dropped[(SYN_WASM_MAX_DATA_SEGS + 63) / 64]; /**< Dropped data segment bitmask */

    uint8_t *linear_mem;      /**< Pointer to linear RAM memory buffer */
    uint32_t linear_mem_size; /**< Active linear RAM memory size in bytes */
    uint32_t linear_mem_cap;  /**< Total backing capacity of linear_mem buffer in bytes */

    SYN_WASM_HostFunc host_funcs[SYN_WASM_MAX_HOST_FUNCS];         /**< Host coroutine table */
    SYN_WASM_HostCancelFunc host_cancels[SYN_WASM_MAX_HOST_FUNCS]; /**< Host cancel callbacks */
    void *host_users[SYN_WASM_MAX_HOST_FUNCS];                     /**< Per-import user pointers */
    uint16_t host_func_count; /**< Registered host function bound */

    uint64_t host_args[SYN_WASM_MAX_HOST_ARGS]; /**< Copied arguments for active host call */
    SYN_WASM_HostScratch host_scratch;          /**< Per-call cross-yield scratch union */
    SYN_Task *active_task;                      /**< Owning scheduler task (or NULL standalone) */
    SYN_PT host_pt;                             /**< Active host coroutine control block */
    uint16_t host_import_idx;                   /**< Active/pending imported function index */
    uint8_t host_argc;                          /**< Active host call argument count */
    bool host_pending;                          /**< True while a host coroutine is in flight */
    bool host_waiting;                          /**< True when pending host yielded PT_IDLE */

    void *user_ctx;         /**< Custom user context pointer */
    SYN_WASM_Status status; /**< VM status and trap code */
};

/* ── Public API ─────────────────────────────────────────────────────────── */

/**
 * @brief Parse and validate a Wasm binary module from flash buffer (zero memory allocation).
 * @param mod Pointer to module descriptor to populate.
 * @param bytes Pointer to flash buffer containing .wasm binary.
 * @param size Length of wasm binary buffer.
 * @return true if valid and within the supported WebAssembly profile and limits.
 */
bool syn_wasm_module_load(SYN_WASM_Module *mod, const uint8_t *bytes, uint32_t size);

/**
 * @brief Parse and validate a Wasm binary module with explicit rejection diagnostics.
 * @param mod Pointer to module descriptor to populate.
 * @param bytes Pointer to flash buffer containing .wasm binary.
 * @param size Length of wasm binary buffer.
 * @param out_error Optional output receiving SYN_WASM_LOAD_OK or the specific rejection reason.
 * @return true if valid and within the supported WebAssembly profile and limits.
 */
bool syn_wasm_module_load_ex(SYN_WASM_Module *mod, const uint8_t *bytes, uint32_t size,
                             SYN_WASM_LoadError *out_error);

/**
 * @brief Return a human-readable description for a load-time validation status code.
 * @param err Load error code from syn_wasm_module_load_ex() or mod->load_error.
 * @return Constant NUL-terminated string describing @p err.
 */
const char *syn_wasm_load_error_str(SYN_WASM_LoadError err);

/**
 * @brief Initialize execution context for loaded module.
 * @param ctx Pointer to context structure.
 * @param mod Pointer to loaded module descriptor.
 * @param linear_mem Pointer to statically-allocated linear memory byte array.
 * @param mem_size Size of linear memory buffer in bytes.
 * @return true if initialized successfully.
 */
bool syn_wasm_init(SYN_WASM_Context *ctx, const SYN_WASM_Module *mod, uint8_t *linear_mem,
                   uint32_t mem_size);

/**
 * @brief Initialize execution context with imported global variable bindings.
 * @param ctx Pointer to context structure.
 * @param mod Pointer to loaded module descriptor.
 * @param linear_mem Pointer to statically-allocated linear memory byte array.
 * @param mem_size Capacity of linear memory buffer in bytes.
 * @param import_globals Optional array of initial 64-bit values for imported globals.
 * @param import_global_count Number of entries in @p import_globals.
 * @return true if initialized successfully.
 */
bool syn_wasm_init_ex(SYN_WASM_Context *ctx, const SYN_WASM_Module *mod, uint8_t *linear_mem,
                      uint32_t mem_size, const uint64_t *import_globals,
                      uint16_t import_global_count);

/**
 * @brief Register a protothread host coroutine for an imported Wasm function slot.
 * @param ctx Pointer to context.
 * @param import_index Index of import function slot (0 .. import_func_count - 1).
 * @param func Host coroutine callback.
 * @return true if registered successfully.
 */
bool syn_wasm_register_host(SYN_WASM_Context *ctx, uint16_t import_index, SYN_WASM_HostFunc func);

/**
 * @brief Register a protothread host coroutine with optional cancellation hook and user state.
 * @param ctx Pointer to context.
 * @param import_index Index of import function slot (0 .. import_func_count - 1).
 * @param func Host coroutine callback.
 * @param cancel Optional cleanup callback invoked by syn_wasm_abort() while pending (or NULL).
 * @param user Optional per-import user pointer retrievable via syn_wasm_host_user() (or NULL).
 * @return true if registered successfully (fails if @p import_index is currently pending).
 */
bool syn_wasm_register_host_ex(SYN_WASM_Context *ctx, uint16_t import_index, SYN_WASM_HostFunc func,
                               SYN_WASM_HostCancelFunc cancel, void *user);

/**
 * @brief Return the per-import user pointer for the currently executing or pending host call.
 * @param ctx Pointer to context.
 * @return Registered `user` pointer for `ctx->host_import_idx`, or NULL when no host call is
 * active.
 */
void *syn_wasm_host_user(const SYN_WASM_Context *ctx);

/**
 * @brief Obtain a mutable bounds-checked pointer into guest linear memory.
 *
 * Guest linear memory is backed by a fixed static arena (`memory.grow` only increases
 * `linear_mem_size` in place without reallocation), so a non-NULL pointer returned here
 * remains valid across cooperative coroutine yields.
 *
 * @param ctx    Pointer to Wasm execution context.
 * @param offset Byte offset into guest linear memory.
 * @param size   Required accessible byte span (0 still validates `offset <= linear_mem_size`).
 * @return Mutable host pointer to `linear_mem + offset`, or NULL on out-of-bounds/overflow/NULL.
 */
void *syn_wasm_mem_ptr(SYN_WASM_Context *ctx, uint32_t offset, uint32_t size);

/**
 * @brief Obtain a read-only bounds-checked pointer into guest linear memory.
 *
 * @param ctx    Pointer to Wasm execution context.
 * @param offset Byte offset into guest linear memory.
 * @param size   Required accessible byte span (0 still validates `offset <= linear_mem_size`).
 * @return Const host pointer to `linear_mem + offset`, or NULL on out-of-bounds/overflow/NULL.
 */
const void *syn_wasm_mem_cptr(const SYN_WASM_Context *ctx, uint32_t offset, uint32_t size);

/**
 * @brief Find exported function index by name.
 * @param mod Pointer to loaded module.
 * @param name Exported function name string.
 * @return Function index if found, or -1 if not found.
 */
int32_t syn_wasm_find_export(const SYN_WASM_Module *mod, const char *name);

/**
 * @brief Prepare context to call a function index.
 *
 * Rejects the call and returns `false` if a previous host coroutine is still in flight
 * (`ctx->host_pending == true`); let the pending call finish or call syn_wasm_abort() first.
 *
 * @param ctx Pointer to context.
 * @param func_index Function index to invoke.
 * @return true if call frame or pending host coroutine initialized (or completed synchronously).
 */
bool syn_wasm_call(SYN_WASM_Context *ctx, uint16_t func_index);

/**
 * @brief Abort any active Wasm execution and cancel any in-flight host coroutine.
 *
 * Resets VM call/operand stacks and sets `ctx->status` to `SYN_WASM_HALTED`. When
 * `ctx->host_pending` was true, clears `active_task->delay_until` (if owned) and invokes
 * any registered `cancel(ctx, user)` hook while `ctx->active_task` is still readable,
 * then clears `ctx->active_task`.
 *
 * @param ctx Pointer to context.
 */
void syn_wasm_abort(SYN_WASM_Context *ctx);

/**
 * @brief Execute instruction slice non-blockingly.
 * @param ctx Pointer to context.
 * @param max_instructions Maximum opcodes to process before yielding.
 * @return SYN_WASM_YIELDED (instruction quota or PT_MORE), SYN_WASM_WAITING (host PT_IDLE),
 *         SYN_WASM_HALTED (finished), SYN_WASM_OK, or a SYN_WASM_TRAP_* code.
 */
SYN_WASM_Status syn_wasm_step(SYN_WASM_Context *ctx, uint16_t max_instructions);

/**
 * @brief Read top of evaluation stack result after HALTED.
 * @param ctx Pointer to context.
 * @return 64-bit top-of-stack return value (32-bit Wasm types occupy low 32 bits).
 */
uint64_t syn_wasm_result(const SYN_WASM_Context *ctx);

/**
 * @brief Child protothread that executes a Wasm function call in instruction quanta.
 *
 * Propagates @p task to `ctx->active_task` (may be NULL outside a scheduler), steps @p ctx
 * by @p quantum_instructions opcodes per slice (defaults to SYN_WASM_DEFAULT_QUANTUM when 0),
 * yields PT_IDLE via PT_PASS(pt) on SYN_WASM_WAITING, and yields PT_MORE via PT_CONTINUE(pt)
 * on SYN_WASM_YIELDED. Composes directly with PT_SPAWN and
 * PT_SPAWN_IF_NEEDED(pt, task, child, syn_wasm_exec_pt(child, task, ctx, quantum)).
 *
 * @param pt                   Child protothread control block.
 * @param task                 Owning scheduler task (or NULL for standalone polling).
 * @param ctx                  Initialized Wasm context with an active syn_wasm_call() frame.
 * @param quantum_instructions Opcodes per slice (0 uses SYN_WASM_DEFAULT_QUANTUM).
 * @return PT_IDLE while waiting on async host I/O/timer, PT_MORE while executing quanta,
 *         PT_ENDED on SYN_WASM_HALTED, or PT_EXITED on NULL/trap.
 */
SYN_PT_Status syn_wasm_exec_pt(SYN_PT *pt, SYN_Task *task, SYN_WASM_Context *ctx,
                               uint16_t quantum_instructions);

/**
 * @brief Cooperative scheduler task entry point for a WebAssembly VM context.
 *
 * Expects `task->user_data` to point to an initialized `SYN_WASM_Context`. Binds
 * `ctx->active_task = task`, executes active calls in SYN_WASM_DEFAULT_QUANTUM slices,
 * yields PT_IDLE via PT_PASS(pt) when halted or parked on an async host call
 * (`SYN_WASM_WAITING`), defers via PT_CONTINUE_IF_NEEDED(pt, task) on `SYN_WASM_YIELDED`
 * when a peer task requires CPU time, and terminates with PT_ENDED on a VM fault trap.
 *
 * @param pt   Protothread state.
 * @param task Scheduler task (`task->user_data` is `SYN_WASM_Context *`).
 * @return Protothread status.
 */
SYN_PT_Status syn_wasm_task(SYN_PT *pt, SYN_Task *task);

/** @name Built-In Standard Host Coroutines */
/**@{*/

/**
 * @brief Standard host coroutine returning `syn_port_get_tick_ms()` (`() -> i32` or `() -> i64`).
 * @param pt   Host protothread control block.
 * @param ctx  Wasm execution context.
 * @param args Argument array (unused).
 * @param argc Argument count (unused).
 * @param ret  Receives current millisecond system tick.
 * @return `PT_ENDED` on success, or `PT_EXITED` if @p pt or @p ret is NULL.
 */
SYN_PT_Status syn_wasm_host_millis(SYN_PT *pt, SYN_WASM_Context *ctx, const uint64_t *args,
                                   uint8_t argc, uint64_t *ret);

/**
 * @brief Standard host coroutine returning `syn_port_get_tick_us()` (`() -> i32` or `() -> i64`).
 * @param pt   Host protothread control block.
 * @param ctx  Wasm execution context.
 * @param args Argument array (unused).
 * @param argc Argument count (unused).
 * @param ret  Receives current microsecond system tick.
 * @return `PT_ENDED` on success, or `PT_EXITED` if @p pt or @p ret is NULL.
 */
SYN_PT_Status syn_wasm_host_micros(SYN_PT *pt, SYN_WASM_Context *ctx, const uint64_t *args,
                                   uint8_t argc, uint64_t *ret);

/**
 * @brief Standard non-spinning sleep host coroutine (`(i32 ms) -> ()` or `(i32 ms) -> i32`).
 *
 * Non-positive signed 32-bit durations (`<= 0`) complete immediately with `PT_ENDED`.
 * Positive durations use `PT_TASK_DELAY_MS(pt, ctx->active_task, ms)` when bound to a
 * scheduler task (enabling true zero-polling sleep and tickless idle), and fall back to
 * standalone `PT_DELAY_MS(pt, &ctx->host_scratch.u32[0], ms)` when `ctx->active_task == NULL`.
 *
 * @param pt   Host protothread control block.
 * @param ctx  Wasm execution context.
 * @param args Argument array (`args[0]` is signed 32-bit delay duration in milliseconds).
 * @param argc Argument count (must be `>= 1`).
 * @param ret  Receives `0` on completion.
 * @return `PT_IDLE` while sleeping, `PT_ENDED` when elapsed, or `PT_EXITED` on invalid args.
 */
SYN_PT_Status syn_wasm_host_sleep_ms(SYN_PT *pt, SYN_WASM_Context *ctx, const uint64_t *args,
                                     uint8_t argc, uint64_t *ret);

/**
 * @brief Standard cooperative round-pass host coroutine (`() -> ()` or `() -> i32`).
 *
 * Yields `PT_IDLE` once via `PT_PASS(pt)` so equal- and lower-priority tasks can run
 * during the current scheduler round, then completes with `PT_ENDED` on the next poll.
 *
 * @param pt   Host protothread control block.
 * @param ctx  Wasm execution context (unused).
 * @param args Argument array (unused).
 * @param argc Argument count (unused).
 * @param ret  Receives `0` on completion.
 * @return `PT_IDLE` on first poll, `PT_ENDED` on resume, or `PT_EXITED` if @p pt or @p ret is NULL.
 */
SYN_PT_Status syn_wasm_host_yield(SYN_PT *pt, SYN_WASM_Context *ctx, const uint64_t *args,
                                  uint8_t argc, uint64_t *ret);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif /* SYN_WASM_H */
