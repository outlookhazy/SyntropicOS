

# Struct SYN\_WASM\_Context\_s



[**ClassList**](annotated.md) **>** [**SYN\_WASM\_Context\_s**](structSYN__WASM__Context__s.md)



_WebAssembly virtual machine execution context._ 

* `#include <syn_wasm.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_Task**](structSYN__Task.md) \* | [**active\_task**](#variable-active_task)  <br> |
|  uint16\_t | [**call\_depth**](#variable-call_depth)  <br> |
|  [**SYN\_WASM\_CallFrame**](structSYN__WASM__CallFrame.md) | [**call\_stack**](#variable-call_stack)  <br> |
|  uint64\_t | [**data\_dropped**](#variable-data_dropped)  <br> |
|  uint16\_t | [**global\_count**](#variable-global_count)  <br> |
|  uint64\_t | [**globals**](#variable-globals)  <br> |
|  uint8\_t | [**host\_argc**](#variable-host_argc)  <br> |
|  uint64\_t | [**host\_args**](#variable-host_args)  <br> |
|  [**SYN\_WASM\_HostCancelFunc**](syn__wasm_8h.md#typedef-syn_wasm_hostcancelfunc) | [**host\_cancels**](#variable-host_cancels)  <br> |
|  uint16\_t | [**host\_func\_count**](#variable-host_func_count)  <br> |
|  [**SYN\_WASM\_HostFunc**](syn__wasm_8h.md#typedef-syn_wasm_hostfunc) | [**host\_funcs**](#variable-host_funcs)  <br> |
|  uint16\_t | [**host\_import\_idx**](#variable-host_import_idx)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**host\_pending**](#variable-host_pending)  <br> |
|  [**SYN\_PT**](structSYN__PT.md) | [**host\_pt**](#variable-host_pt)  <br> |
|  [**SYN\_WASM\_HostScratch**](unionSYN__WASM__HostScratch.md) | [**host\_scratch**](#variable-host_scratch)  <br> |
|  void \* | [**host\_users**](#variable-host_users)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**host\_waiting**](#variable-host_waiting)  <br> |
|  uint16\_t | [**label\_depth**](#variable-label_depth)  <br> |
|  [**SYN\_WASM\_Label**](structSYN__WASM__Label.md) | [**label\_stack**](#variable-label_stack)  <br> |
|  uint8\_t \* | [**linear\_mem**](#variable-linear_mem)  <br> |
|  uint32\_t | [**linear\_mem\_cap**](#variable-linear_mem_cap)  <br> |
|  uint32\_t | [**linear\_mem\_size**](#variable-linear_mem_size)  <br> |
|  uint16\_t | [**local\_count**](#variable-local_count)  <br> |
|  uint64\_t | [**locals**](#variable-locals)  <br> |
|  const [**SYN\_WASM\_Module**](structSYN__WASM__Module.md) \* | [**module**](#variable-module)  <br> |
|  uint32\_t | [**pc**](#variable-pc)  <br> |
|  uint32\_t | [**sp**](#variable-sp)  <br> |
|  uint64\_t | [**stack**](#variable-stack)  <br> |
|  [**SYN\_WASM\_Status**](syn__wasm_8h.md#enum-syn_wasm_status) | [**status**](#variable-status)  <br> |
|  uint16\_t | [**table\_element\_count**](#variable-table_element_count)  <br> |
|  uint16\_t | [**table\_elements**](#variable-table_elements)  <br> |
|  void \* | [**user\_ctx**](#variable-user_ctx)  <br> |












































## Public Attributes Documentation




### variable active\_task 

```C++
SYN_Task* SYN_WASM_Context::active_task;
```



Owning scheduler task (or NULL standalone) 


        

<hr>



### variable call\_depth 

```C++
uint16_t SYN_WASM_Context::call_depth;
```



Active call depth 


        

<hr>



### variable call\_stack 

```C++
SYN_WASM_CallFrame SYN_WASM_Context::call_stack[SYN_WASM_MAX_CALL_DEPTH];
```



Call frame stack array 


        

<hr>



### variable data\_dropped 

```C++
uint64_t SYN_WASM_Context::data_dropped[(SYN_WASM_MAX_DATA_SEGS+63)/64];
```



Dropped data segment bitmask 


        

<hr>



### variable global\_count 

```C++
uint16_t SYN_WASM_Context::global_count;
```



Global variable count 


        

<hr>



### variable globals 

```C++
uint64_t SYN_WASM_Context::globals[SYN_WASM_MAX_GLOBALS];
```



Global variables array 


        

<hr>



### variable host\_argc 

```C++
uint8_t SYN_WASM_Context::host_argc;
```



Active host call argument count 


        

<hr>



### variable host\_args 

```C++
uint64_t SYN_WASM_Context::host_args[SYN_WASM_MAX_HOST_ARGS];
```



Copied arguments for active host call 


        

<hr>



### variable host\_cancels 

```C++
SYN_WASM_HostCancelFunc SYN_WASM_Context::host_cancels[SYN_WASM_MAX_HOST_FUNCS];
```



Host cancel callbacks 


        

<hr>



### variable host\_func\_count 

```C++
uint16_t SYN_WASM_Context::host_func_count;
```



Registered host function bound 


        

<hr>



### variable host\_funcs 

```C++
SYN_WASM_HostFunc SYN_WASM_Context::host_funcs[SYN_WASM_MAX_HOST_FUNCS];
```



Host coroutine table 


        

<hr>



### variable host\_import\_idx 

```C++
uint16_t SYN_WASM_Context::host_import_idx;
```



Active/pending imported function index 


        

<hr>



### variable host\_pending 

```C++
bool SYN_WASM_Context::host_pending;
```



True while a host coroutine is in flight 


        

<hr>



### variable host\_pt 

```C++
SYN_PT SYN_WASM_Context::host_pt;
```



Active host coroutine control block 


        

<hr>



### variable host\_scratch 

```C++
SYN_WASM_HostScratch SYN_WASM_Context::host_scratch;
```



Per-call cross-yield scratch union 


        

<hr>



### variable host\_users 

```C++
void* SYN_WASM_Context::host_users[SYN_WASM_MAX_HOST_FUNCS];
```



Per-import user pointers 


        

<hr>



### variable host\_waiting 

```C++
bool SYN_WASM_Context::host_waiting;
```



True when pending host yielded PT\_IDLE 


        

<hr>



### variable label\_depth 

```C++
uint16_t SYN_WASM_Context::label_depth;
```



Active label depth 


        

<hr>



### variable label\_stack 

```C++
SYN_WASM_Label SYN_WASM_Context::label_stack[SYN_WASM_MAX_LABELS];
```



Control block label stack array 


        

<hr>



### variable linear\_mem 

```C++
uint8_t* SYN_WASM_Context::linear_mem;
```



Pointer to linear RAM memory buffer 


        

<hr>



### variable linear\_mem\_cap 

```C++
uint32_t SYN_WASM_Context::linear_mem_cap;
```



Total backing capacity of linear\_mem buffer in bytes 


        

<hr>



### variable linear\_mem\_size 

```C++
uint32_t SYN_WASM_Context::linear_mem_size;
```



Active linear RAM memory size in bytes 


        

<hr>



### variable local\_count 

```C++
uint16_t SYN_WASM_Context::local_count;
```



Active local variable count 


        

<hr>



### variable locals 

```C++
uint64_t SYN_WASM_Context::locals[SYN_WASM_MAX_LOCALS];
```



Local variables array 


        

<hr>



### variable module 

```C++
const SYN_WASM_Module* SYN_WASM_Context::module;
```



Pointer to target Wasm module 


        

<hr>



### variable pc 

```C++
uint32_t SYN_WASM_Context::pc;
```



Program counter opcode offset 


        

<hr>



### variable sp 

```C++
uint32_t SYN_WASM_Context::sp;
```



Operand stack pointer 


        

<hr>



### variable stack 

```C++
uint64_t SYN_WASM_Context::stack[SYN_WASM_MAX_STACK];
```



Operand stack array 


        

<hr>



### variable status 

```C++
SYN_WASM_Status SYN_WASM_Context::status;
```



VM status and trap code 


        

<hr>



### variable table\_element\_count 

```C++
uint16_t SYN_WASM_Context::table_element_count;
```



Populated indirect call table bound 


        

<hr>



### variable table\_elements 

```C++
uint16_t SYN_WASM_Context::table_elements[SYN_WASM_MAX_TABLE_ELEMS];
```



Instance indirect call table 


        

<hr>



### variable user\_ctx 

```C++
void* SYN_WASM_Context::user_ctx;
```



Custom user context pointer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/vm/syn_wasm.h`

