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
    SYN_WASM_YIELDED,                  /**< Execution yielded after instruction quota */
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
    SYN_WASM_TRAP_UNREGISTERED_HOST    /**< Unregistered host function trap */
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
 * @brief Host function signature callable from Wasm bytecode.
 * @param ctx Pointer to Wasm execution context.
 * @param args Array of 64-bit operand stack slots passed as arguments (32-bit types in low bits).
 * @param argc Number of arguments.
 * @return 64-bit return value pushed onto Wasm stack when result_count > 0 (ignored if void).
 */
typedef uint64_t (*SYN_WASM_HostFunc)(SYN_WASM_Context *ctx, const uint64_t *args, uint8_t argc);

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

    SYN_WASM_HostFunc host_funcs[SYN_WASM_MAX_HOST_FUNCS]; /**< Host function table */
    uint16_t host_func_count;                              /**< Registered host function count */

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
 * @brief Register C host function for imported Wasm functions.
 * @param ctx Pointer to context.
 * @param import_index Index of import function slot (0 .. import_func_count - 1).
 * @param func Function pointer to host C handler.
 * @return true if registered successfully.
 */
bool syn_wasm_register_host(SYN_WASM_Context *ctx, uint16_t import_index, SYN_WASM_HostFunc func);

/**
 * @brief Find exported function index by name.
 * @param mod Pointer to loaded module.
 * @param name Exported function name string.
 * @return Function index if found, or -1 if not found.
 */
int32_t syn_wasm_find_export(const SYN_WASM_Module *mod, const char *name);

/**
 * @brief Prepare context to call a function index.
 * @param ctx Pointer to context.
 * @param func_index Function index to invoke.
 * @return true if call frame initialized successfully.
 */
bool syn_wasm_call(SYN_WASM_Context *ctx, uint16_t func_index);

/**
 * @brief Execute instruction slice non-blockingly.
 * @param ctx Pointer to context.
 * @param max_instructions Maximum opcodes to process before yielding.
 * @return SYN_WASM_OK (completed), SYN_WASM_YIELDED (time slice expired), SYN_WASM_HALTED
 * (finished), or TRAP.
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
 * Steps @p ctx by @p quantum_instructions opcodes per slice (defaults to
 * SYN_WASM_DEFAULT_QUANTUM when 0) and yields PT_MORE via PT_CONTINUE(pt) whenever
 * the slice expires with SYN_WASM_YIELDED. Composes directly with PT_SPAWN and
 * PT_SPAWN_IF_NEEDED(pt, task, child, syn_wasm_exec_pt(...)).
 *
 * @param pt                   Child protothread control block.
 * @param ctx                  Initialized Wasm context with an active syn_wasm_call() frame.
 * @param quantum_instructions Opcodes per slice (0 uses SYN_WASM_DEFAULT_QUANTUM).
 * @return PT_MORE while executing, PT_ENDED on SYN_WASM_HALTED, or PT_EXITED on NULL/trap.
 */
SYN_PT_Status syn_wasm_exec_pt(SYN_PT *pt, SYN_WASM_Context *ctx, uint16_t quantum_instructions);

/**
 * @brief Cooperative scheduler task entry point for a WebAssembly VM context.
 *
 * Expects `task->user_data` to point to an initialized `SYN_WASM_Context`.
 * Executes active calls in SYN_WASM_DEFAULT_QUANTUM slices and defers via
 * PT_CONTINUE_IF_NEEDED(pt, task) when a peer task requires CPU time, yields
 * PT_IDLE via PT_PASS(pt) when halted or waiting for a new call, and terminates
 * with PT_ENDED if the VM enters a fault trap state.
 *
 * @param pt   Protothread state.
 * @param task Scheduler task (`task->user_data` is `SYN_WASM_Context *`).
 * @return Protothread status.
 */
SYN_PT_Status syn_wasm_task(SYN_PT *pt, SYN_Task *task);

#ifdef __cplusplus
}
#endif

#endif /* SYN_WASM_H */
