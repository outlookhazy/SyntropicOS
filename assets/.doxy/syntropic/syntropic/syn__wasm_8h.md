

# File syn\_wasm.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**vm**](dir_10a4ed2785bdaf7cfd36ec9032e0af72.md) **>** [**syn\_wasm.h**](syn__wasm_8h.md)

[Go to the source code of this file](syn__wasm_8h_source.md)

_[EXPERIMENTAL] Zero-heap, 32-bit WebAssembly cooperative interpreter._ [More...](#detailed-description)

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
| struct | [**SYN\_WASM\_FuncType**](structSYN__WASM__FuncType.md) <br>_WebAssembly function signature descriptor (points into module bytes)._  |
| struct | [**SYN\_WASM\_Label**](structSYN__WASM__Label.md) <br>_WebAssembly control flow label stack entry._  |
| struct | [**SYN\_WASM\_Module**](structSYN__WASM__Module.md) <br>_Parsed WebAssembly module header and export registry._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_WASM\_HostCancelFunc**](#typedef-syn_wasm_hostcancelfunc)  <br>_Optional cleanup callback invoked by_ [_**syn\_wasm\_abort()**_](syn__wasm_8h.md#function-syn_wasm_abort) _when a pending host call is cancelled._ |
| typedef [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status)(\* | [**SYN\_WASM\_HostFunc**](#typedef-syn_wasm_hostfunc)  <br>_Protothread coroutine host function signature callable from Wasm bytecode._  |
| union  | [**SYN\_WASM\_HostScratch**](#union-syn_wasm_hostscratch)  <br>_Per-call scratch storage preserved across host coroutine yields._  |
| enum  | [**SYN\_WASM\_LoadError**](#enum-syn_wasm_loaderror)  <br>_Load-time module validation result and rejection reason._  |
| enum  | [**SYN\_WASM\_Status**](#enum-syn_wasm_status)  <br>_WebAssembly VM execution status codes and traps._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_wasm\_abort**](#function-syn_wasm_abort) (SYN\_WASM\_Context \* ctx) <br>_Abort any active Wasm execution and cancel any in-flight host coroutine._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_wasm\_call**](#function-syn_wasm_call) (SYN\_WASM\_Context \* ctx, uint16\_t func\_index) <br>_Prepare context to call a function index._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_wasm\_exec\_pt**](#function-syn_wasm_exec_pt) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task, SYN\_WASM\_Context \* ctx, uint16\_t quantum\_instructions) <br>_Child protothread that executes a Wasm function call in instruction quanta._  |
|  int32\_t | [**syn\_wasm\_find\_export**](#function-syn_wasm_find_export) (const [**SYN\_WASM\_Module**](structSYN__WASM__Module.md) \* mod, const char \* name) <br>_Find exported function index by name._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_wasm\_host\_micros**](#function-syn_wasm_host_micros) ([**SYN\_PT**](structSYN__PT.md) \* pt, SYN\_WASM\_Context \* ctx, const uint64\_t \* args, uint8\_t argc, uint64\_t \* ret) <br>_Standard host coroutine returning_ `syn_port_get_tick_us()` _(_`() -> i32` _or_`() -> i64` _)._ |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_wasm\_host\_millis**](#function-syn_wasm_host_millis) ([**SYN\_PT**](structSYN__PT.md) \* pt, SYN\_WASM\_Context \* ctx, const uint64\_t \* args, uint8\_t argc, uint64\_t \* ret) <br>_Standard host coroutine returning_ `syn_port_get_tick_ms()` _(_`() -> i32` _or_`() -> i64` _)._ |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_wasm\_host\_sleep\_ms**](#function-syn_wasm_host_sleep_ms) ([**SYN\_PT**](structSYN__PT.md) \* pt, SYN\_WASM\_Context \* ctx, const uint64\_t \* args, uint8\_t argc, uint64\_t \* ret) <br>_Standard non-spinning sleep host coroutine (_ `(i32 ms) -> ()` _or_`(i32 ms) -> i32` _)._ |
|  void \* | [**syn\_wasm\_host\_user**](#function-syn_wasm_host_user) (const SYN\_WASM\_Context \* ctx) <br>_Return the per-import user pointer for the currently executing or pending host call._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_wasm\_host\_yield**](#function-syn_wasm_host_yield) ([**SYN\_PT**](structSYN__PT.md) \* pt, SYN\_WASM\_Context \* ctx, const uint64\_t \* args, uint8\_t argc, uint64\_t \* ret) <br>_Standard cooperative round-pass host coroutine (_ `() -> ()` _or_`() -> i32` _)._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_wasm\_init**](#function-syn_wasm_init) (SYN\_WASM\_Context \* ctx, const [**SYN\_WASM\_Module**](structSYN__WASM__Module.md) \* mod, uint8\_t \* linear\_mem, uint32\_t mem\_size) <br>_Initialize execution context for loaded module._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_wasm\_init\_ex**](#function-syn_wasm_init_ex) (SYN\_WASM\_Context \* ctx, const [**SYN\_WASM\_Module**](structSYN__WASM__Module.md) \* mod, uint8\_t \* linear\_mem, uint32\_t mem\_size, const uint64\_t \* import\_globals, uint16\_t import\_global\_count) <br>_Initialize execution context with imported global variable bindings._  |
|  const char \* | [**syn\_wasm\_load\_error\_str**](#function-syn_wasm_load_error_str) ([**SYN\_WASM\_LoadError**](syn__wasm_8h.md#enum-syn_wasm_loaderror) err) <br>_Return a human-readable description for a load-time validation status code._  |
|  const void \* | [**syn\_wasm\_mem\_cptr**](#function-syn_wasm_mem_cptr) (const SYN\_WASM\_Context \* ctx, uint32\_t offset, uint32\_t size) <br>_Obtain a read-only bounds-checked pointer into guest linear memory._  |
|  void \* | [**syn\_wasm\_mem\_ptr**](#function-syn_wasm_mem_ptr) (SYN\_WASM\_Context \* ctx, uint32\_t offset, uint32\_t size) <br>_Obtain a mutable bounds-checked pointer into guest linear memory._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_wasm\_module\_load**](#function-syn_wasm_module_load) ([**SYN\_WASM\_Module**](structSYN__WASM__Module.md) \* mod, const uint8\_t \* bytes, uint32\_t size) <br>_Parse and validate a Wasm binary module from flash buffer (zero memory allocation)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_wasm\_module\_load\_ex**](#function-syn_wasm_module_load_ex) ([**SYN\_WASM\_Module**](structSYN__WASM__Module.md) \* mod, const uint8\_t \* bytes, uint32\_t size, [**SYN\_WASM\_LoadError**](syn__wasm_8h.md#enum-syn_wasm_loaderror) \* out\_error) <br>_Parse and validate a Wasm binary module with explicit rejection diagnostics._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_wasm\_register\_host**](#function-syn_wasm_register_host) (SYN\_WASM\_Context \* ctx, uint16\_t import\_index, [**SYN\_WASM\_HostFunc**](syn__wasm_8h.md#typedef-syn_wasm_hostfunc) func) <br>_Register a protothread host coroutine for an imported Wasm function slot._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_wasm\_register\_host\_ex**](#function-syn_wasm_register_host_ex) (SYN\_WASM\_Context \* ctx, uint16\_t import\_index, [**SYN\_WASM\_HostFunc**](syn__wasm_8h.md#typedef-syn_wasm_hostfunc) func, [**SYN\_WASM\_HostCancelFunc**](syn__wasm_8h.md#typedef-syn_wasm_hostcancelfunc) cancel, void \* user) <br>_Register a protothread host coroutine with optional cancellation hook and user state._  |
|  uint64\_t | [**syn\_wasm\_result**](#function-syn_wasm_result) (const SYN\_WASM\_Context \* ctx) <br>_Read top of evaluation stack result after HALTED._  |
|  [**SYN\_WASM\_Status**](syn__wasm_8h.md#enum-syn_wasm_status) | [**syn\_wasm\_step**](#function-syn_wasm_step) (SYN\_WASM\_Context \* ctx, uint16\_t max\_instructions) <br>_Execute instruction slice non-blockingly._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_wasm\_task**](#function-syn_wasm_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative scheduler task entry point for a WebAssembly VM context._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_WASM\_DEFAULT\_QUANTUM**](syn__wasm_8h.md#define-syn_wasm_default_quantum)  `64U`<br> |
| define  | [**SYN\_WASM\_HOST\_RETURN**](syn__wasm_8h.md#define-syn_wasm_host_return) (pt, ret\_ptr, val\_expr) `/* multi line expression */`<br>_Complete a synchronous Wasm host coroutine with a 64-bit return value._  |
| define  | [**SYN\_WASM\_HOST\_SYNC**](syn__wasm_8h.md#define-syn_wasm_host_sync) (name, ctx\_var, args\_var, argc\_var) `/* multi line expression */`<br>_Define a_ `static` _non-yielding Wasm host function returning_`uint64_t` _._ |
| define  | [**SYN\_WASM\_MAX\_CALL\_DEPTH**](syn__wasm_8h.md#define-syn_wasm_max_call_depth)  `32`<br> |
| define  | [**SYN\_WASM\_MAX\_DATA\_SEGS**](syn__wasm_8h.md#define-syn_wasm_max_data_segs)  `16`<br> |
| define  | [**SYN\_WASM\_MAX\_EXPORTS**](syn__wasm_8h.md#define-syn_wasm_max_exports)  `[**SYN\_WASM\_MAX\_FUNCTIONS**](syn__wasm_8h.md#define-syn_wasm_max_functions)`<br> |
| define  | [**SYN\_WASM\_MAX\_FUNCTIONS**](syn__wasm_8h.md#define-syn_wasm_max_functions)  `32`<br> |
| define  | [**SYN\_WASM\_MAX\_GLOBALS**](syn__wasm_8h.md#define-syn_wasm_max_globals)  `16`<br> |
| define  | [**SYN\_WASM\_MAX\_HOST\_ARGS**](syn__wasm_8h.md#define-syn_wasm_max_host_args)  `16`<br> |
| define  | [**SYN\_WASM\_MAX\_HOST\_FUNCS**](syn__wasm_8h.md#define-syn_wasm_max_host_funcs)  `16`<br> |
| define  | [**SYN\_WASM\_MAX\_LABELS**](syn__wasm_8h.md#define-syn_wasm_max_labels)  `16`<br> |
| define  | [**SYN\_WASM\_MAX\_LOCALS**](syn__wasm_8h.md#define-syn_wasm_max_locals)  `256`<br> |
| define  | [**SYN\_WASM\_MAX\_STACK**](syn__wasm_8h.md#define-syn_wasm_max_stack)  `64`<br> |
| define  | [**SYN\_WASM\_MAX\_TABLE\_ELEMS**](syn__wasm_8h.md#define-syn_wasm_max_table_elems)  `64`<br> |
| define  | [**SYN\_WASM\_MAX\_TYPES**](syn__wasm_8h.md#define-syn_wasm_max_types)  `64`<br> |
| define  | [**SYN\_WASM\_TABLE\_NULL**](syn__wasm_8h.md#define-syn_wasm_table_null)  `0xFFFFU`<br>_Sentinel value for an uninitialized indirect function table slot._  |

## Detailed Description




**Warning:**

EXPERIMENTAL MODULE - API and bytecode execution semantics subject to change.


Implements a 100% zero-malloc, non-blocking 32-bit WebAssembly interpreter designed for embedded microcontrollers. Supports static linear memory arrays, zero-copy parsing and load-time validation from flash, host function registration, and instruction-sliced cooperative execution with protothread (`syn_pt`) integration.


Supported WebAssembly feature profile (validated at module load):
* WebAssembly 1.0 Core (MVP): i32/i64/f32/f64, structured control flow, single linear memory (index 0), single funcref table (index 0) with signature-checked `call_indirect`, and multi-result function types.
* `mutable-globals`: imported and internal mutable globals.
* `sign-ext`: `i32.extend8_s`, `i32.extend16_s`, `i64.extend8_s`, `i64.extend16_s`, `i64.extend32_s` (`0xC0`–`0xC4`).
* `nontrapping-fptoint`: saturating float-to-int conversions (`0xFC 0x00`–`0x07`).
* `bulk-memory` (memory 0 subset): `memory.copy`, `memory.fill`, `memory.init`, `data.drop`.




Unsupported extensions (rejected at compile time via `-mcpu=mvp` and at load time):
* `threads` / shared memory / atomics (`0xFE`), `simd` (`0xFD`), `reference-types`, `multivalue` block signatures, `tail-call`, `exception-handling`, `memory64`, `multi-memory`, and `gc`.




Recommended C-to-WASM compilation command (pins Clang to the supported profile): clang target=wasm32-unknown-unknown -mcpu=mvp -O2 -nostdlib \ -mmutable-globals -msign-ext -mbulk-memory -mnontrapping-fptoint \ -Wl,no-entry -Wl,export-all -Wl,allow-undefined \ -Wl,-z,stack-size=1024 app.c -o app.wasm 


    
## Public Types Documentation




### typedef SYN\_WASM\_HostCancelFunc 

_Optional cleanup callback invoked by_ [_**syn\_wasm\_abort()**_](syn__wasm_8h.md#function-syn_wasm_abort) _when a pending host call is cancelled._
```C++
typedef void(* SYN_WASM_HostCancelFunc) (SYN_WASM_Context *ctx, void *user);
```





**Parameters:**


* `ctx` Pointer to Wasm execution context. 
* `user` Per-import user pointer registered via [**syn\_wasm\_register\_host\_ex()**](syn__wasm_8h.md#function-syn_wasm_register_host_ex). 




        

<hr>



### typedef SYN\_WASM\_HostFunc 

_Protothread coroutine host function signature callable from Wasm bytecode._ 
```C++
typedef SYN_PT_Status(* SYN_WASM_HostFunc) (SYN_PT *pt, SYN_WASM_Context *ctx, const uint64_t *args, uint8_t argc, uint64_t *ret);
```



Polled with `ctx->host_pt` until returning `PT_ENDED` (normal completion) or `PT_EXITED` (host trap). Yielding `PT_IDLE` suspends the guest call with `SYN_WASM_WAITING`, while yielding `PT_MORE` yields a cooperative chunk with `SYN_WASM_YIELDED`. Arguments in `args` are copied into `ctx->host_args` and remain valid across yields. `ret` is always non-NULL and zero-initialized when invoked by the VM.




**Parameters:**


* `pt` Host coroutine protothread control block (`&ctx->host_pt`). 
* `ctx` Pointer to Wasm execution context. 
* `args` Copied array of 64-bit argument slots (`NULL` when `argc == 0`). 
* `argc` Number of arguments (`<=  SYN_WASM_MAX_HOST_ARGS`). 
* `ret` Output slot for 64-bit return value when `result_count > 0`. 



**Returns:**

`PT_ENDED` on completion, `PT_IDLE` / `PT_MORE` while suspended, or `PT_EXITED` on trap. 





        

<hr>



### union SYN\_WASM\_HostScratch 

_Per-call scratch storage preserved across host coroutine yields._ 
```C++

```




<hr>



### enum SYN\_WASM\_LoadError 

_Load-time module validation result and rejection reason._ 
```C++
enum SYN_WASM_LoadError {
    SYN_WASM_LOAD_OK = 0,
    SYN_WASM_LOAD_ERR_INVALID_ARG,
    SYN_WASM_LOAD_ERR_BAD_HEADER,
    SYN_WASM_LOAD_ERR_MALFORMED,
    SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED,
    SYN_WASM_LOAD_ERR_UNSUPPORTED
};
```




<hr>



### enum SYN\_WASM\_Status 

_WebAssembly VM execution status codes and traps._ 
```C++
enum SYN_WASM_Status {
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
};
```




<hr>
## Public Functions Documentation




### function syn\_wasm\_abort 

_Abort any active Wasm execution and cancel any in-flight host coroutine._ 
```C++
void syn_wasm_abort (
    SYN_WASM_Context * ctx
) 
```



Resets VM call/operand stacks and sets `ctx->status` to `SYN_WASM_HALTED`. When `ctx->host_pending` was true, clears `active_task->delay_until` (if owned) and invokes any registered `cancel(ctx, user)` hook while `ctx->active_task` is still readable, then clears `ctx->active_task`.




**Parameters:**


* `ctx` Pointer to context. 




        

<hr>



### function syn\_wasm\_call 

_Prepare context to call a function index._ 
```C++
bool syn_wasm_call (
    SYN_WASM_Context * ctx,
    uint16_t func_index
) 
```



Rejects the call and returns `false` if a previous host coroutine is still in flight (`ctx->host_pending == true`); let the pending call finish or call [**syn\_wasm\_abort()**](syn__wasm_8h.md#function-syn_wasm_abort) first.




**Parameters:**


* `ctx` Pointer to context. 
* `func_index` Function index to invoke. 



**Returns:**

true if call frame or pending host coroutine initialized (or completed synchronously). 





        

<hr>



### function syn\_wasm\_exec\_pt 

_Child protothread that executes a Wasm function call in instruction quanta._ 
```C++
SYN_PT_Status syn_wasm_exec_pt (
    SYN_PT * pt,
    SYN_Task * task,
    SYN_WASM_Context * ctx,
    uint16_t quantum_instructions
) 
```



Propagates `task` to `ctx->active_task` (may be NULL outside a scheduler), steps `ctx` by `quantum_instructions` opcodes per slice (defaults to SYN\_WASM\_DEFAULT\_QUANTUM when 0), yields PT\_IDLE via [**PT\_PASS(pt)**](syn__pt_8h.md#define-pt_pass) on SYN\_WASM\_WAITING, and yields PT\_MORE via [**PT\_CONTINUE(pt)**](syn__pt_8h.md#define-pt_continue) on SYN\_WASM\_YIELDED. Composes directly with PT\_SPAWN and [**PT\_SPAWN\_IF\_NEEDED(pt, task, child, syn\_wasm\_exec\_pt(child, task, ctx, quantum))**](syn__sched_8h.md#define-pt_spawn_if_needed).




**Parameters:**


* `pt` Child protothread control block. 
* `task` Owning scheduler task (or NULL for standalone polling). 
* `ctx` Initialized Wasm context with an active [**syn\_wasm\_call()**](syn__wasm_8h.md#function-syn_wasm_call) frame. 
* `quantum_instructions` Opcodes per slice (0 uses SYN\_WASM\_DEFAULT\_QUANTUM). 



**Returns:**

PT\_IDLE while waiting on async host I/O/timer, PT\_MORE while executing quanta, PT\_ENDED on SYN\_WASM\_HALTED, or PT\_EXITED on NULL/trap. 





        

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



### function syn\_wasm\_host\_micros 

_Standard host coroutine returning_ `syn_port_get_tick_us()` _(_`() -> i32` _or_`() -> i64` _)._
```C++
SYN_PT_Status syn_wasm_host_micros (
    SYN_PT * pt,
    SYN_WASM_Context * ctx,
    const uint64_t * args,
    uint8_t argc,
    uint64_t * ret
) 
```





**Parameters:**


* `pt` Host protothread control block. 
* `ctx` Wasm execution context. 
* `args` Argument array (unused). 
* `argc` Argument count (unused). 
* `ret` Receives current microsecond system tick. 



**Returns:**

`PT_ENDED` on success, or `PT_EXITED` if `pt` or `ret` is NULL. 





        

<hr>



### function syn\_wasm\_host\_millis 

_Standard host coroutine returning_ `syn_port_get_tick_ms()` _(_`() -> i32` _or_`() -> i64` _)._
```C++
SYN_PT_Status syn_wasm_host_millis (
    SYN_PT * pt,
    SYN_WASM_Context * ctx,
    const uint64_t * args,
    uint8_t argc,
    uint64_t * ret
) 
```





**Parameters:**


* `pt` Host protothread control block. 
* `ctx` Wasm execution context. 
* `args` Argument array (unused). 
* `argc` Argument count (unused). 
* `ret` Receives current millisecond system tick. 



**Returns:**

`PT_ENDED` on success, or `PT_EXITED` if `pt` or `ret` is NULL. 





        

<hr>



### function syn\_wasm\_host\_sleep\_ms 

_Standard non-spinning sleep host coroutine (_ `(i32 ms) -> ()` _or_`(i32 ms) -> i32` _)._
```C++
SYN_PT_Status syn_wasm_host_sleep_ms (
    SYN_PT * pt,
    SYN_WASM_Context * ctx,
    const uint64_t * args,
    uint8_t argc,
    uint64_t * ret
) 
```



Non-positive signed 32-bit durations (`<= 0`) complete immediately with `PT_ENDED`. Positive durations use `PT_TASK_DELAY_MS (pt, ctx->active_task, ms)` when bound to a scheduler task (enabling true zero-polling sleep and tickless idle), and fall back to standalone `PT_DELAY_MS (pt, &ctx->host_scratch.u32[0], ms)` when `ctx->active_task == NULL`.




**Parameters:**


* `pt` Host protothread control block. 
* `ctx` Wasm execution context. 
* `args` Argument array (`args[0]` is signed 32-bit delay duration in milliseconds). 
* `argc` Argument count (must be `>= 1`). 
* `ret` Receives `0` on completion. 



**Returns:**

`PT_IDLE` while sleeping, `PT_ENDED` when elapsed, or `PT_EXITED` on invalid args. 





        

<hr>



### function syn\_wasm\_host\_user 

_Return the per-import user pointer for the currently executing or pending host call._ 
```C++
void * syn_wasm_host_user (
    const SYN_WASM_Context * ctx
) 
```





**Parameters:**


* `ctx` Pointer to context. 



**Returns:**

Registered `user` pointer for `ctx->host_import_idx`, or NULL when no host call is active. 





        

<hr>



### function syn\_wasm\_host\_yield 

_Standard cooperative round-pass host coroutine (_ `() -> ()` _or_`() -> i32` _)._
```C++
SYN_PT_Status syn_wasm_host_yield (
    SYN_PT * pt,
    SYN_WASM_Context * ctx,
    const uint64_t * args,
    uint8_t argc,
    uint64_t * ret
) 
```



Yields `PT_IDLE` once via `PT_PASS(pt)` so equal- and lower-priority tasks can run during the current scheduler round, then completes with `PT_ENDED` on the next poll.




**Parameters:**


* `pt` Host protothread control block. 
* `ctx` Wasm execution context (unused). 
* `args` Argument array (unused). 
* `argc` Argument count (unused). 
* `ret` Receives `0` on completion. 



**Returns:**

`PT_IDLE` on first poll, `PT_ENDED` on resume, or `PT_EXITED` if `pt` or `ret` is NULL. 





        

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



### function syn\_wasm\_init\_ex 

_Initialize execution context with imported global variable bindings._ 
```C++
bool syn_wasm_init_ex (
    SYN_WASM_Context * ctx,
    const SYN_WASM_Module * mod,
    uint8_t * linear_mem,
    uint32_t mem_size,
    const uint64_t * import_globals,
    uint16_t import_global_count
) 
```





**Parameters:**


* `ctx` Pointer to context structure. 
* `mod` Pointer to loaded module descriptor. 
* `linear_mem` Pointer to statically-allocated linear memory byte array. 
* `mem_size` Capacity of linear memory buffer in bytes. 
* `import_globals` Optional array of initial 64-bit values for imported globals. 
* `import_global_count` Number of entries in `import_globals`. 



**Returns:**

true if initialized successfully. 





        

<hr>



### function syn\_wasm\_load\_error\_str 

_Return a human-readable description for a load-time validation status code._ 
```C++
const char * syn_wasm_load_error_str (
    SYN_WASM_LoadError err
) 
```





**Parameters:**


* `err` Load error code from [**syn\_wasm\_module\_load\_ex()**](syn__wasm_8h.md#function-syn_wasm_module_load_ex) or mod-&gt;load\_error. 



**Returns:**

Constant NUL-terminated string describing `err`. 





        

<hr>



### function syn\_wasm\_mem\_cptr 

_Obtain a read-only bounds-checked pointer into guest linear memory._ 
```C++
const void * syn_wasm_mem_cptr (
    const SYN_WASM_Context * ctx,
    uint32_t offset,
    uint32_t size
) 
```





**Parameters:**


* `ctx` Pointer to Wasm execution context. 
* `offset` Byte offset into guest linear memory. 
* `size` Required accessible byte span (0 still validates `offset <= linear_mem_size`). 



**Returns:**

Const host pointer to `linear_mem + offset`, or NULL on out-of-bounds/overflow/NULL. 





        

<hr>



### function syn\_wasm\_mem\_ptr 

_Obtain a mutable bounds-checked pointer into guest linear memory._ 
```C++
void * syn_wasm_mem_ptr (
    SYN_WASM_Context * ctx,
    uint32_t offset,
    uint32_t size
) 
```



Guest linear memory is backed by a fixed static arena (`memory.grow` only increases `linear_mem_size` in place without reallocation), so a non-NULL pointer returned here remains valid across cooperative coroutine yields.




**Parameters:**


* `ctx` Pointer to Wasm execution context. 
* `offset` Byte offset into guest linear memory. 
* `size` Required accessible byte span (0 still validates `offset <= linear_mem_size`). 



**Returns:**

Mutable host pointer to `linear_mem + offset`, or NULL on out-of-bounds/overflow/NULL. 





        

<hr>



### function syn\_wasm\_module\_load 

_Parse and validate a Wasm binary module from flash buffer (zero memory allocation)._ 
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

true if valid and within the supported WebAssembly profile and limits. 





        

<hr>



### function syn\_wasm\_module\_load\_ex 

_Parse and validate a Wasm binary module with explicit rejection diagnostics._ 
```C++
bool syn_wasm_module_load_ex (
    SYN_WASM_Module * mod,
    const uint8_t * bytes,
    uint32_t size,
    SYN_WASM_LoadError * out_error
) 
```





**Parameters:**


* `mod` Pointer to module descriptor to populate. 
* `bytes` Pointer to flash buffer containing .wasm binary. 
* `size` Length of wasm binary buffer. 
* `out_error` Optional output receiving SYN\_WASM\_LOAD\_OK or the specific rejection reason. 



**Returns:**

true if valid and within the supported WebAssembly profile and limits. 





        

<hr>



### function syn\_wasm\_register\_host 

_Register a protothread host coroutine for an imported Wasm function slot._ 
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
* `func` Host coroutine callback. 



**Returns:**

true if registered successfully. 





        

<hr>



### function syn\_wasm\_register\_host\_ex 

_Register a protothread host coroutine with optional cancellation hook and user state._ 
```C++
bool syn_wasm_register_host_ex (
    SYN_WASM_Context * ctx,
    uint16_t import_index,
    SYN_WASM_HostFunc func,
    SYN_WASM_HostCancelFunc cancel,
    void * user
) 
```





**Parameters:**


* `ctx` Pointer to context. 
* `import_index` Index of import function slot (0 .. import\_func\_count - 1). 
* `func` Host coroutine callback. 
* `cancel` Optional cleanup callback invoked by [**syn\_wasm\_abort()**](syn__wasm_8h.md#function-syn_wasm_abort) while pending (or NULL). 
* `user` Optional per-import user pointer retrievable via [**syn\_wasm\_host\_user()**](syn__wasm_8h.md#function-syn_wasm_host_user) (or NULL). 



**Returns:**

true if registered successfully (fails if `import_index` is currently pending). 





        

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

SYN\_WASM\_YIELDED (instruction quota or PT\_MORE), SYN\_WASM\_WAITING (host PT\_IDLE), SYN\_WASM\_HALTED (finished), SYN\_WASM\_OK, or a SYN\_WASM\_TRAP\_\* code. 





        

<hr>



### function syn\_wasm\_task 

_Cooperative scheduler task entry point for a WebAssembly VM context._ 
```C++
SYN_PT_Status syn_wasm_task (
    SYN_PT * pt,
    SYN_Task * task
) 
```



Expects `task->user_data` to point to an initialized `SYN_WASM_Context`. Binds `ctx->active_task = task`, executes active calls in SYN\_WASM\_DEFAULT\_QUANTUM slices, yields PT\_IDLE via [**PT\_PASS(pt)**](syn__pt_8h.md#define-pt_pass) when halted or parked on an async host call (`SYN_WASM_WAITING`), defers via [**PT\_CONTINUE\_IF\_NEEDED(pt, task)**](syn__sched_8h.md#define-pt_continue_if_needed) on `SYN_WASM_YIELDED` when a peer task requires CPU time, and terminates with PT\_ENDED on a VM fault trap.




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



### define SYN\_WASM\_HOST\_RETURN 

_Complete a synchronous Wasm host coroutine with a 64-bit return value._ 
```C++
#define SYN_WASM_HOST_RETURN (
    pt,
    ret_ptr,
    val_expr
) `/* multi line expression */`
```



Resets `pt`, writes `val_expr` to `*(ret_ptr)` when non-NULL, and returns `PT_ENDED`.




**Parameters:**


* `pt` Host protothread control block (`SYN_PT *`). 
* `ret_ptr` Output return slot pointer (`uint64_t *`). 
* `val_expr` Return value expression convertible to `uint64_t`. 




        

<hr>



### define SYN\_WASM\_HOST\_SYNC 

_Define a_ `static` _non-yielding Wasm host function returning_`uint64_t` _._
```C++
#define SYN_WASM_HOST_SYNC (
    name,
    ctx_var,
    args_var,
    argc_var
) `/* multi line expression */`
```



Generates a `SYN_WASM_HostFunc`-compatible `static` wrapper named `name` around a synchronous C function body so one-shot host callbacks remain concise.




**Parameters:**


* `name` Identifier of the generated `static` `SYN_WASM_HostFunc`. 
* `ctx_var` Parameter name for `SYN_WASM_Context *`. 
* `args_var` Parameter name for `const uint64_t *`. 
* `argc_var` Parameter name for `uint8_t`. 




        

<hr>



### define SYN\_WASM\_MAX\_CALL\_DEPTH 

```C++
#define SYN_WASM_MAX_CALL_DEPTH `32`
```



Maximum call frame stack depth 


        

<hr>



### define SYN\_WASM\_MAX\_DATA\_SEGS 

```C++
#define SYN_WASM_MAX_DATA_SEGS `16`
```



Maximum data segments per module 


        

<hr>



### define SYN\_WASM\_MAX\_EXPORTS 

```C++
#define SYN_WASM_MAX_EXPORTS `SYN_WASM_MAX_FUNCTIONS`
```



Maximum exported items per module 


        

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



### define SYN\_WASM\_MAX\_HOST\_ARGS 

```C++
#define SYN_WASM_MAX_HOST_ARGS `16`
```



Maximum argument count for imported host functions 


        

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



### define SYN\_WASM\_MAX\_TABLE\_ELEMS 

```C++
#define SYN_WASM_MAX_TABLE_ELEMS `64`
```



Maximum indirect function table slots 


        

<hr>



### define SYN\_WASM\_MAX\_TYPES 

```C++
#define SYN_WASM_MAX_TYPES `64`
```



Maximum function type signatures per module 


        

<hr>



### define SYN\_WASM\_TABLE\_NULL 

_Sentinel value for an uninitialized indirect function table slot._ 
```C++
#define SYN_WASM_TABLE_NULL `0xFFFFU`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/vm/syn_wasm.h`

