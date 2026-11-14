

# File syn\_wasm.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**vm**](dir_10a4ed2785bdaf7cfd36ec9032e0af72.md) **>** [**syn\_wasm.h**](syn__wasm_8h.md)

[Go to the source code of this file](syn__wasm_8h_source.md)

_[EXPERIMENTAL] Zero-heap, 32-bit WebAssembly (Wasm MVP) cooperative interpreter._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../pt/syn_pt.h"`
* `#include "../sched/syn_sched.h"`
* `#include "../util/syn_qmath.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_WASM\_CallFrame**](structSYN__WASM__CallFrame.md) <br>_WebAssembly runtime call frame._  |
| struct | [**SYN\_WASM\_Context\_s**](structSYN__WASM__Context__s.md) <br>_WebAssembly virtual machine execution context._  |
| struct | [**SYN\_WASM\_FuncDef**](structSYN__WASM__FuncDef.md) <br>_WebAssembly function definition metadata._  |
| struct | [**SYN\_WASM\_Label**](structSYN__WASM__Label.md) <br>_WebAssembly control flow label stack entry._  |
| struct | [**SYN\_WASM\_Module**](structSYN__WASM__Module.md) <br>_Parsed WebAssembly module header and export registry._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef uint64\_t(\* | [**SYN\_WASM\_HostFunc**](#typedef-syn_wasm_hostfunc)  <br>_Host function signature callable from Wasm bytecode._  |
| enum  | [**SYN\_WASM\_Status**](#enum-syn_wasm_status)  <br>_WebAssembly VM execution status codes and traps._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_wasm\_call**](#function-syn_wasm_call) (SYN\_WASM\_Context \* ctx, uint16\_t func\_index) <br>_Prepare context to call a function index._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_wasm\_exec\_pt**](#function-syn_wasm_exec_pt) ([**SYN\_PT**](structSYN__PT.md) \* pt, SYN\_WASM\_Context \* ctx, uint16\_t quantum\_instructions) <br>_Child protothread that executes a Wasm function call in instruction quanta._  |
|  int32\_t | [**syn\_wasm\_find\_export**](#function-syn_wasm_find_export) (const [**SYN\_WASM\_Module**](structSYN__WASM__Module.md) \* mod, const char \* name) <br>_Find exported function index by name._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_wasm\_init**](#function-syn_wasm_init) (SYN\_WASM\_Context \* ctx, const [**SYN\_WASM\_Module**](structSYN__WASM__Module.md) \* mod, uint8\_t \* linear\_mem, uint32\_t mem\_size) <br>_Initialize execution context for loaded module._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_wasm\_module\_load**](#function-syn_wasm_module_load) ([**SYN\_WASM\_Module**](structSYN__WASM__Module.md) \* mod, const uint8\_t \* bytes, uint32\_t size) <br>_Parse Wasm binary module from flash buffer (zero memory allocation)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_wasm\_register\_host**](#function-syn_wasm_register_host) (SYN\_WASM\_Context \* ctx, uint16\_t import\_index, [**SYN\_WASM\_HostFunc**](syn__wasm_8h.md#typedef-syn_wasm_hostfunc) func) <br>_Register C host function for imported Wasm functions._  |
|  uint64\_t | [**syn\_wasm\_result**](#function-syn_wasm_result) (const SYN\_WASM\_Context \* ctx) <br>_Read top of evaluation stack result after HALTED._  |
|  [**SYN\_WASM\_Status**](syn__wasm_8h.md#enum-syn_wasm_status) | [**syn\_wasm\_step**](#function-syn_wasm_step) (SYN\_WASM\_Context \* ctx, uint16\_t max\_instructions) <br>_Execute instruction slice non-blockingly._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_wasm\_task**](#function-syn_wasm_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative scheduler task entry point for a WebAssembly VM context._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_WASM\_DEFAULT\_QUANTUM**](syn__wasm_8h.md#define-syn_wasm_default_quantum)  `64U`<br> |
| define  | [**SYN\_WASM\_MAX\_CALL\_DEPTH**](syn__wasm_8h.md#define-syn_wasm_max_call_depth)  `32`<br> |
| define  | [**SYN\_WASM\_MAX\_FUNCTIONS**](syn__wasm_8h.md#define-syn_wasm_max_functions)  `32`<br> |
| define  | [**SYN\_WASM\_MAX\_GLOBALS**](syn__wasm_8h.md#define-syn_wasm_max_globals)  `16`<br> |
| define  | [**SYN\_WASM\_MAX\_HOST\_FUNCS**](syn__wasm_8h.md#define-syn_wasm_max_host_funcs)  `16`<br> |
| define  | [**SYN\_WASM\_MAX\_LABELS**](syn__wasm_8h.md#define-syn_wasm_max_labels)  `16`<br> |
| define  | [**SYN\_WASM\_MAX\_LOCALS**](syn__wasm_8h.md#define-syn_wasm_max_locals)  `256`<br> |
| define  | [**SYN\_WASM\_MAX\_STACK**](syn__wasm_8h.md#define-syn_wasm_max_stack)  `64`<br> |

## Detailed Description




**Warning:**

EXPERIMENTAL MODULE - API and bytecode execution semantics subject to change.


Implements a 100% zero-malloc, non-blocking 32-bit WebAssembly MVP interpreter designed for embedded microcontrollers. Supports static linear memory arrays, zero-copy parsing from flash, host function registration, and instruction-sliced cooperative execution with protothread (`syn_pt`) integration.


Recommended C-to-WASM compilation command for user applications: clang target=wasm32-unknown-unknown -O2 -nostdlib \ -mbulk-memory -msign-ext -mmultivalue -mnontrapping-fptoint \ -Wl,no-entry -Wl,export-all -Wl,allow-undefined \ -Wl,-z,stack-size=1024 app.c -o app.wasm 


    
## Public Types Documentation




### typedef SYN\_WASM\_HostFunc 

_Host function signature callable from Wasm bytecode._ 
```C++
typedef uint64_t(* SYN_WASM_HostFunc) (SYN_WASM_Context *ctx, const uint64_t *args, uint8_t argc);
```





**Parameters:**


* `ctx` Pointer to Wasm execution context. 
* `args` Array of 64-bit operand stack slots passed as arguments (32-bit types in low bits). 
* `argc` Number of arguments. 



**Returns:**

64-bit return value pushed onto Wasm stack when result\_count &gt; 0 (ignored if void). 





        

<hr>



### enum SYN\_WASM\_Status 

_WebAssembly VM execution status codes and traps._ 
```C++
enum SYN_WASM_Status {
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
};
```




<hr>
## Public Functions Documentation




### function syn\_wasm\_call 

_Prepare context to call a function index._ 
```C++
bool syn_wasm_call (
    SYN_WASM_Context * ctx,
    uint16_t func_index
) 
```





**Parameters:**


* `ctx` Pointer to context. 
* `func_index` Function index to invoke. 



**Returns:**

true if call frame initialized successfully. 





        

<hr>



### function syn\_wasm\_exec\_pt 

_Child protothread that executes a Wasm function call in instruction quanta._ 
```C++
SYN_PT_Status syn_wasm_exec_pt (
    SYN_PT * pt,
    SYN_WASM_Context * ctx,
    uint16_t quantum_instructions
) 
```



Steps `ctx` by `quantum_instructions` opcodes per slice (defaults to SYN\_WASM\_DEFAULT\_QUANTUM when 0) and yields PT\_MORE via [**PT\_CONTINUE(pt)**](syn__pt_8h.md#define-pt_continue) whenever the slice expires with SYN\_WASM\_YIELDED. Composes directly with PT\_SPAWN and [**PT\_SPAWN\_IF\_NEEDED(pt, task, child, syn\_wasm\_exec\_pt(...))**](syn__sched_8h.md#define-pt_spawn_if_needed).




**Parameters:**


* `pt` Child protothread control block. 
* `ctx` Initialized Wasm context with an active [**syn\_wasm\_call()**](syn__wasm_8h.md#function-syn_wasm_call) frame. 
* `quantum_instructions` Opcodes per slice (0 uses SYN\_WASM\_DEFAULT\_QUANTUM). 



**Returns:**

PT\_MORE while executing, PT\_ENDED on SYN\_WASM\_HALTED, or PT\_EXITED on NULL/trap. 





        

<hr>



### function syn\_wasm\_find\_export 

_Find exported function index by name._ 
```C++
int32_t syn_wasm_find_export (
    const SYN_WASM_Module * mod,
    const char * name
) 
```





**Parameters:**


* `mod` Pointer to loaded module. 
* `name` Exported function name string. 



**Returns:**

Function index if found, or -1 if not found. 





        

<hr>



### function syn\_wasm\_init 

_Initialize execution context for loaded module._ 
```C++
bool syn_wasm_init (
    SYN_WASM_Context * ctx,
    const SYN_WASM_Module * mod,
    uint8_t * linear_mem,
    uint32_t mem_size
) 
```





**Parameters:**


* `ctx` Pointer to context structure. 
* `mod` Pointer to loaded module descriptor. 
* `linear_mem` Pointer to statically-allocated linear memory byte array. 
* `mem_size` Size of linear memory buffer in bytes. 



**Returns:**

true if initialized successfully. 





        

<hr>



### function syn\_wasm\_module\_load 

_Parse Wasm binary module from flash buffer (zero memory allocation)._ 
```C++
bool syn_wasm_module_load (
    SYN_WASM_Module * mod,
    const uint8_t * bytes,
    uint32_t size
) 
```





**Parameters:**


* `mod` Pointer to module descriptor to populate. 
* `bytes` Pointer to flash buffer containing .wasm binary. 
* `size` Length of wasm binary buffer. 



**Returns:**

true if valid Wasm MVP binary module parsed successfully. 





        

<hr>



### function syn\_wasm\_register\_host 

_Register C host function for imported Wasm functions._ 
```C++
bool syn_wasm_register_host (
    SYN_WASM_Context * ctx,
    uint16_t import_index,
    SYN_WASM_HostFunc func
) 
```





**Parameters:**


* `ctx` Pointer to context. 
* `import_index` Index of import function slot (0 .. import\_func\_count - 1). 
* `func` Function pointer to host C handler. 



**Returns:**

true if registered successfully. 





        

<hr>



### function syn\_wasm\_result 

_Read top of evaluation stack result after HALTED._ 
```C++
uint64_t syn_wasm_result (
    const SYN_WASM_Context * ctx
) 
```





**Parameters:**


* `ctx` Pointer to context. 



**Returns:**

64-bit top-of-stack return value (32-bit Wasm types occupy low 32 bits). 





        

<hr>



### function syn\_wasm\_step 

_Execute instruction slice non-blockingly._ 
```C++
SYN_WASM_Status syn_wasm_step (
    SYN_WASM_Context * ctx,
    uint16_t max_instructions
) 
```





**Parameters:**


* `ctx` Pointer to context. 
* `max_instructions` Maximum opcodes to process before yielding. 



**Returns:**

SYN\_WASM\_OK (completed), SYN\_WASM\_YIELDED (time slice expired), SYN\_WASM\_HALTED (finished), or TRAP. 





        

<hr>



### function syn\_wasm\_task 

_Cooperative scheduler task entry point for a WebAssembly VM context._ 
```C++
SYN_PT_Status syn_wasm_task (
    SYN_PT * pt,
    SYN_Task * task
) 
```



Expects `task->user_data` to point to an initialized `SYN_WASM_Context`. Executes active calls in SYN\_WASM\_DEFAULT\_QUANTUM slices and defers via [**PT\_CONTINUE\_IF\_NEEDED(pt, task)**](syn__sched_8h.md#define-pt_continue_if_needed) when a peer task requires CPU time, yields PT\_IDLE via [**PT\_PASS(pt)**](syn__pt_8h.md#define-pt_pass) when halted or waiting for a new call, and terminates with PT\_ENDED if the VM enters a fault trap state.




**Parameters:**


* `pt` Protothread state. 
* `task` Scheduler task (`task->user_data` is `SYN_WASM_Context *`). 



**Returns:**

Protothread status. 





        

<hr>
## Macro Definition Documentation





### define SYN\_WASM\_DEFAULT\_QUANTUM 

```C++
#define SYN_WASM_DEFAULT_QUANTUM `64U`
```



Default instruction slice per cooperative quantum 


        

<hr>



### define SYN\_WASM\_MAX\_CALL\_DEPTH 

```C++
#define SYN_WASM_MAX_CALL_DEPTH `32`
```



Maximum call frame stack depth 


        

<hr>



### define SYN\_WASM\_MAX\_FUNCTIONS 

```C++
#define SYN_WASM_MAX_FUNCTIONS `32`
```



Maximum internal function defs 


        

<hr>



### define SYN\_WASM\_MAX\_GLOBALS 

```C++
#define SYN_WASM_MAX_GLOBALS `16`
```



Maximum global variables 


        

<hr>



### define SYN\_WASM\_MAX\_HOST\_FUNCS 

```C++
#define SYN_WASM_MAX_HOST_FUNCS `16`
```



Maximum registered host functions 


        

<hr>



### define SYN\_WASM\_MAX\_LABELS 

```C++
#define SYN_WASM_MAX_LABELS `16`
```



Maximum block/loop control labels 


        

<hr>



### define SYN\_WASM\_MAX\_LOCALS 

```C++
#define SYN_WASM_MAX_LOCALS `256`
```



Maximum local variable count per frame 


        

<hr>



### define SYN\_WASM\_MAX\_STACK 

```C++
#define SYN_WASM_MAX_STACK `64`
```



Maximum operand stack depth 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/vm/syn_wasm.h`

