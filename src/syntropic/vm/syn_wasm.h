/**
 * @file syn_wasm.h
 * @brief [EXPERIMENTAL] Zero-heap, 32-bit WebAssembly (Wasm MVP) cooperative interpreter.
 *
 * @warning EXPERIMENTAL MODULE - API and bytecode execution semantics subject to change.
 *
 * Implements a 100% zero-malloc, non-blocking 32-bit WebAssembly MVP interpreter
 * designed for embedded microcontrollers. Supports static linear memory arrays,
 * zero-copy parsing from flash, host function registration, and instruction-sliced
 * cooperative execution with protothread (`syn_pt`) integration.
 *
 * Recommended C-to-WASM compilation command for user applications:
 *   clang --target=wasm32-unknown-unknown -O2 -nostdlib \
 *     -mbulk-memory -msign-ext -mmultivalue -mnontrapping-fptoint \
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

    SYN_WASM_FuncDef funcs[SYN_WASM_MAX_FUNCTIONS]; /**< Function registry */
    uint16_t func_count;                            /**< Total function count */
    uint16_t import_func_count;                     /**< Imported host function count */

    struct {
        uint32_t name_offset;          /**< Export name offset */
        uint16_t name_len;             /**< Export name length */
        uint16_t func_idx;             /**< Function index */
    } exports[SYN_WASM_MAX_FUNCTIONS]; /**< Export registry */
    uint16_t export_count;             /**< Export count */

    uint16_t table_elements[64];  /**< Element table for indirect calls */
    uint16_t table_element_count; /**< Element count */

    uint32_t start_func_idx; /**< Start function index */
    bool has_start_func;     /**< True if module has start function */
} SYN_WASM_Module;

/* ── Runtime Call Frame & Label Stacks ───────────────────────────────────── */

/** @brief WebAssembly runtime call frame */
typedef struct {
    uint16_t func_idx;   /**< Called function index */
    uint32_t return_pc;  /**< Return program counter */
    uint32_t frame_sp;   /**< Frame operand stack pointer */
    uint16_t local_base; /**< Base index in locals array */
    uint8_t label_base;  /**< Caller label stack depth on entry */
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
    uint8_t call_depth;                                     /**< Active call depth */

    SYN_WASM_Label label_stack[SYN_WASM_MAX_LABELS]; /**< Control block label stack array */
    uint8_t label_depth;                             /**< Active label depth */

    uint64_t globals[SYN_WASM_MAX_GLOBALS]; /**< Global variables array */
    uint16_t global_count;                  /**< Global variable count */

    uint8_t *linear_mem;      /**< Pointer to linear RAM memory buffer */
    uint32_t linear_mem_size; /**< Size of linear RAM memory buffer in bytes */

    SYN_WASM_HostFunc host_funcs[SYN_WASM_MAX_HOST_FUNCS]; /**< Host function table */
    uint16_t host_func_count;                              /**< Registered host function count */

    void *user_ctx;         /**< Custom user context pointer */
    SYN_WASM_Status status; /**< VM status and trap code */
};

/* ── Public API ─────────────────────────────────────────────────────────── */

/**
 * @brief Parse Wasm binary module from flash buffer (zero memory allocation).
 * @param mod Pointer to module descriptor to populate.
 * @param bytes Pointer to flash buffer containing .wasm binary.
 * @param size Length of wasm binary buffer.
 * @return true if valid Wasm MVP binary module parsed successfully.
 */
bool syn_wasm_module_load(SYN_WASM_Module *mod, const uint8_t *bytes, uint32_t size);

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
