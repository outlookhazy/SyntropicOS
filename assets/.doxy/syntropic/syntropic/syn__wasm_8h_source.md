

# File syn\_wasm.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**vm**](dir_10a4ed2785bdaf7cfd36ec9032e0af72.md) **>** [**syn\_wasm.h**](syn__wasm_8h.md)

[Go to the documentation of this file](syn__wasm_8h.md)


```C++

#ifndef SYN_WASM_H
#define SYN_WASM_H

#include "../common/syn_defs.h"
#include "../util/syn_qmath.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SYN_WASM_MAX_STACK
#define SYN_WASM_MAX_STACK 64 
#endif

#ifndef SYN_WASM_MAX_LOCALS
#define SYN_WASM_MAX_LOCALS 256 
#endif

#ifndef SYN_WASM_MAX_CALL_DEPTH
#define SYN_WASM_MAX_CALL_DEPTH 32 
#endif

#ifndef SYN_WASM_MAX_FUNCTIONS
#define SYN_WASM_MAX_FUNCTIONS 32 
#endif

#ifndef SYN_WASM_MAX_HOST_FUNCS
#define SYN_WASM_MAX_HOST_FUNCS 16 
#endif

#ifndef SYN_WASM_MAX_GLOBALS
#define SYN_WASM_MAX_GLOBALS 16 
#endif

#ifndef SYN_WASM_MAX_LABELS
#define SYN_WASM_MAX_LABELS 16 
#endif
/* ── Status Codes & Traps ───────────────────────────────────────────────── */

typedef enum {
    SYN_WASM_OK = 0,                   
    SYN_WASM_YIELDED,                  
    SYN_WASM_HALTED,                   
    SYN_WASM_TRAP_STACK_OVERFLOW,      
    SYN_WASM_TRAP_STACK_UNDERFLOW,     
    SYN_WASM_TRAP_OUT_OF_BOUNDS,       
    SYN_WASM_TRAP_BAD_OPCODE,          
    SYN_WASM_TRAP_DIV_ZERO,            
    SYN_WASM_TRAP_UNREACHABLE,         
    SYN_WASM_TRAP_CALL_STACK_OVERFLOW, 
    SYN_WASM_TRAP_TYPE_MISMATCH,       
    SYN_WASM_TRAP_INVALID_MODULE,      
    SYN_WASM_TRAP_UNREGISTERED_HOST    
} SYN_WASM_Status;

/* Forward declarations */
typedef struct SYN_WASM_Context_s SYN_WASM_Context;

typedef uint64_t (*SYN_WASM_HostFunc)(SYN_WASM_Context *ctx, const uint64_t *args, uint8_t argc);

typedef struct {
    uint32_t type_idx;    
    uint32_t code_offset; 
    uint32_t code_size;   
    uint8_t param_count;  
    uint8_t result_count; 
} SYN_WASM_FuncDef;

typedef struct {
    const uint8_t *bytes; 
    uint32_t size;        
    SYN_WASM_FuncDef funcs[SYN_WASM_MAX_FUNCTIONS]; 
    uint16_t func_count;                            
    uint16_t import_func_count;                     
    struct {
        uint32_t name_offset;          
        uint16_t name_len;             
        uint16_t func_idx;             
    } exports[SYN_WASM_MAX_FUNCTIONS]; 
    uint16_t export_count;             
    uint16_t table_elements[64];  
    uint16_t table_element_count; 
    uint32_t start_func_idx; 
    bool has_start_func;     
} SYN_WASM_Module;

/* ── Runtime Call Frame & Label Stacks ───────────────────────────────────── */

typedef struct {
    uint16_t func_idx;   
    uint32_t return_pc;  
    uint32_t frame_sp;   
    uint16_t local_base; 
} SYN_WASM_CallFrame;

typedef struct {
    uint8_t opcode;     
    uint32_t target_pc; 
    uint32_t stack_sp;  
} SYN_WASM_Label;

/* ── Runtime Execution Context ────────────────────────────────────────────── */

struct SYN_WASM_Context_s {
    const SYN_WASM_Module *module; 
    uint32_t pc;                        
    uint32_t sp;                        
    uint64_t stack[SYN_WASM_MAX_STACK]; 
    uint64_t locals[SYN_WASM_MAX_LOCALS]; 
    uint16_t local_count;                 
    SYN_WASM_CallFrame call_stack[SYN_WASM_MAX_CALL_DEPTH]; 
    uint8_t call_depth;                                     
    SYN_WASM_Label label_stack[SYN_WASM_MAX_LABELS]; 
    uint8_t label_depth;                             
    uint64_t globals[SYN_WASM_MAX_GLOBALS]; 
    uint16_t global_count;                  
    uint8_t *linear_mem;      
    uint32_t linear_mem_size; 
    SYN_WASM_HostFunc host_funcs[SYN_WASM_MAX_HOST_FUNCS]; 
    uint16_t host_func_count;                              
    void *user_ctx;         
    SYN_WASM_Status status; 
};

/* ── Public API ─────────────────────────────────────────────────────────── */

bool syn_wasm_module_load(SYN_WASM_Module *mod, const uint8_t *bytes, uint32_t size);

bool syn_wasm_init(SYN_WASM_Context *ctx, const SYN_WASM_Module *mod, uint8_t *linear_mem,
                   uint32_t mem_size);

bool syn_wasm_register_host(SYN_WASM_Context *ctx, uint16_t import_index, SYN_WASM_HostFunc func);

int32_t syn_wasm_find_export(const SYN_WASM_Module *mod, const char *name);

bool syn_wasm_call(SYN_WASM_Context *ctx, uint16_t func_index);

SYN_WASM_Status syn_wasm_step(SYN_WASM_Context *ctx, uint16_t max_instructions);

uint64_t syn_wasm_result(const SYN_WASM_Context *ctx);

#ifdef __cplusplus
}
#endif

#endif /* SYN_WASM_H */
```


