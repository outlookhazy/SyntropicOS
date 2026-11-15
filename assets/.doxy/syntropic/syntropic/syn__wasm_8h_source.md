

# File syn\_wasm.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**vm**](dir_10a4ed2785bdaf7cfd36ec9032e0af72.md) **>** [**syn\_wasm.h**](syn__wasm_8h.md)

[Go to the documentation of this file](syn__wasm_8h.md)


```C++


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

#ifndef SYN_WASM_MAX_HOST_ARGS
#define SYN_WASM_MAX_HOST_ARGS 16 
#endif

#if (SYN_WASM_MAX_HOST_ARGS < 1) || (SYN_WASM_MAX_HOST_ARGS > 255)
#error "SYN_WASM_MAX_HOST_ARGS must be between 1 and 255"
#endif

#ifndef SYN_WASM_MAX_GLOBALS
#define SYN_WASM_MAX_GLOBALS 16 
#endif

#ifndef SYN_WASM_MAX_LABELS
#define SYN_WASM_MAX_LABELS 16 
#endif

#ifndef SYN_WASM_MAX_TYPES
#define SYN_WASM_MAX_TYPES 64 
#endif

#ifndef SYN_WASM_MAX_EXPORTS
#define SYN_WASM_MAX_EXPORTS SYN_WASM_MAX_FUNCTIONS 
#endif

#ifndef SYN_WASM_MAX_TABLE_ELEMS
#define SYN_WASM_MAX_TABLE_ELEMS 64 
#endif

#ifndef SYN_WASM_MAX_DATA_SEGS
#define SYN_WASM_MAX_DATA_SEGS 16 
#endif

#ifndef SYN_WASM_DEFAULT_QUANTUM
#define SYN_WASM_DEFAULT_QUANTUM 64U 
#endif

/* ── Status Codes & Traps ───────────────────────────────────────────────── */

typedef enum {
    SYN_WASM_OK = 0,                   
    SYN_WASM_YIELDED,                  
    SYN_WASM_WAITING,                  
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
    SYN_WASM_TRAP_UNREGISTERED_HOST,   
    SYN_WASM_TRAP_HOST                 
} SYN_WASM_Status;

#define SYN_WASM_TABLE_NULL 0xFFFFU

typedef enum {
    SYN_WASM_LOAD_OK = 0,             
    SYN_WASM_LOAD_ERR_INVALID_ARG,    
    SYN_WASM_LOAD_ERR_BAD_HEADER,     
    SYN_WASM_LOAD_ERR_MALFORMED,      
    SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED, 
    SYN_WASM_LOAD_ERR_UNSUPPORTED     
} SYN_WASM_LoadError;

/* Forward declarations */
typedef struct SYN_WASM_Context_s SYN_WASM_Context;

typedef SYN_PT_Status (*SYN_WASM_HostFunc)(SYN_PT *pt, SYN_WASM_Context *ctx, const uint64_t *args,
                                           uint8_t argc, uint64_t *ret);

typedef void (*SYN_WASM_HostCancelFunc)(SYN_WASM_Context *ctx, void *user);

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

typedef union {
    uint64_t u64[2]; 
    uint32_t u32[4]; 
    uint8_t u8[16];  
    struct {
        SYN_PT child_pt; 
        uint16_t u16[7]; 
    } pt;                
    SYN_PT child_pt;     
} SYN_WASM_HostScratch;

typedef struct {
    uint32_t params_offset;  
    uint32_t results_offset; 
    uint8_t param_count;     
    uint8_t result_count;    
} SYN_WASM_FuncType;

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

    SYN_WASM_FuncType types[SYN_WASM_MAX_TYPES]; 
    uint16_t type_count;                         

    SYN_WASM_FuncDef funcs[SYN_WASM_MAX_FUNCTIONS]; 
    uint16_t func_count;                            
    uint16_t import_func_count;                     
    uint16_t import_global_count;                   

    struct {
        uint32_t name_offset;        
        uint16_t name_len;           
        uint16_t func_idx;           
        uint8_t kind;                
    } exports[SYN_WASM_MAX_EXPORTS]; 
    uint16_t export_count;           

    uint16_t table_elements[SYN_WASM_MAX_TABLE_ELEMS]; 
    uint16_t table_element_count;                      
    uint32_t table_min_elems;                          
    uint32_t table_max_elems;                          
    bool has_table;                                    
    bool table_has_max;                                

    uint32_t mem_min_pages; 
    uint32_t mem_max_pages; 
    bool has_memory;        
    bool mem_has_max;       

    struct {
        uint32_t offset;                 
        uint32_t size;                   
        bool passive;                    
    } data_segs[SYN_WASM_MAX_DATA_SEGS]; 
    uint16_t data_seg_count;             

    uint32_t start_func_idx;       
    bool has_start_func;           
    SYN_WASM_LoadError load_error; 
} SYN_WASM_Module;

/* ── Runtime Call Frame & Label Stacks ───────────────────────────────────── */

typedef struct {
    uint16_t func_idx;   
    uint32_t return_pc;  
    uint32_t frame_sp;   
    uint16_t local_base; 
    uint16_t label_base; 
} SYN_WASM_CallFrame;

typedef struct {
    uint8_t opcode;     
    uint8_t arity;      
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
    uint16_t call_depth;                                    

    SYN_WASM_Label label_stack[SYN_WASM_MAX_LABELS]; 
    uint16_t label_depth;                            

    uint64_t globals[SYN_WASM_MAX_GLOBALS]; 
    uint16_t global_count;                  

    uint16_t table_elements[SYN_WASM_MAX_TABLE_ELEMS]; 
    uint16_t table_element_count;                      

    uint64_t data_dropped[(SYN_WASM_MAX_DATA_SEGS + 63) / 64]; 

    uint8_t *linear_mem;      
    uint32_t linear_mem_size; 
    uint32_t linear_mem_cap;  

    SYN_WASM_HostFunc host_funcs[SYN_WASM_MAX_HOST_FUNCS];         
    SYN_WASM_HostCancelFunc host_cancels[SYN_WASM_MAX_HOST_FUNCS]; 
    void *host_users[SYN_WASM_MAX_HOST_FUNCS];                     
    uint16_t host_func_count; 

    uint64_t host_args[SYN_WASM_MAX_HOST_ARGS]; 
    SYN_WASM_HostScratch host_scratch;          
    SYN_Task *active_task;                      
    SYN_PT host_pt;                             
    uint16_t host_import_idx;                   
    uint8_t host_argc;                          
    bool host_pending;                          
    bool host_waiting;                          

    void *user_ctx;         
    SYN_WASM_Status status; 
};

/* ── Public API ─────────────────────────────────────────────────────────── */

bool syn_wasm_module_load(SYN_WASM_Module *mod, const uint8_t *bytes, uint32_t size);

bool syn_wasm_module_load_ex(SYN_WASM_Module *mod, const uint8_t *bytes, uint32_t size,
                             SYN_WASM_LoadError *out_error);

const char *syn_wasm_load_error_str(SYN_WASM_LoadError err);

bool syn_wasm_init(SYN_WASM_Context *ctx, const SYN_WASM_Module *mod, uint8_t *linear_mem,
                   uint32_t mem_size);

bool syn_wasm_init_ex(SYN_WASM_Context *ctx, const SYN_WASM_Module *mod, uint8_t *linear_mem,
                      uint32_t mem_size, const uint64_t *import_globals,
                      uint16_t import_global_count);

bool syn_wasm_register_host(SYN_WASM_Context *ctx, uint16_t import_index, SYN_WASM_HostFunc func);

bool syn_wasm_register_host_ex(SYN_WASM_Context *ctx, uint16_t import_index, SYN_WASM_HostFunc func,
                               SYN_WASM_HostCancelFunc cancel, void *user);

void *syn_wasm_host_user(const SYN_WASM_Context *ctx);

void *syn_wasm_mem_ptr(SYN_WASM_Context *ctx, uint32_t offset, uint32_t size);

const void *syn_wasm_mem_cptr(const SYN_WASM_Context *ctx, uint32_t offset, uint32_t size);

int32_t syn_wasm_find_export(const SYN_WASM_Module *mod, const char *name);

bool syn_wasm_call(SYN_WASM_Context *ctx, uint16_t func_index);

void syn_wasm_abort(SYN_WASM_Context *ctx);

SYN_WASM_Status syn_wasm_step(SYN_WASM_Context *ctx, uint16_t max_instructions);

uint64_t syn_wasm_result(const SYN_WASM_Context *ctx);

SYN_PT_Status syn_wasm_exec_pt(SYN_PT *pt, SYN_Task *task, SYN_WASM_Context *ctx,
                               uint16_t quantum_instructions);

SYN_PT_Status syn_wasm_task(SYN_PT *pt, SYN_Task *task);


SYN_PT_Status syn_wasm_host_millis(SYN_PT *pt, SYN_WASM_Context *ctx, const uint64_t *args,
                                   uint8_t argc, uint64_t *ret);

SYN_PT_Status syn_wasm_host_micros(SYN_PT *pt, SYN_WASM_Context *ctx, const uint64_t *args,
                                   uint8_t argc, uint64_t *ret);

SYN_PT_Status syn_wasm_host_sleep_ms(SYN_PT *pt, SYN_WASM_Context *ctx, const uint64_t *args,
                                     uint8_t argc, uint64_t *ret);

SYN_PT_Status syn_wasm_host_yield(SYN_PT *pt, SYN_WASM_Context *ctx, const uint64_t *args,
                                  uint8_t argc, uint64_t *ret);


#ifdef __cplusplus
}
#endif

#endif /* SYN_WASM_H */
```


