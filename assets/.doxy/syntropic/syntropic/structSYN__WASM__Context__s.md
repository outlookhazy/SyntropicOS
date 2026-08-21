

# Struct SYN\_WASM\_Context\_s



[**ClassList**](annotated.md) **>** [**SYN\_WASM\_Context\_s**](structSYN__WASM__Context__s.md)



_WebAssembly virtual machine execution context._ 

* `#include <syn_wasm.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**call\_depth**](#variable-call_depth)  <br> |
|  [**SYN\_WASM\_CallFrame**](structSYN__WASM__CallFrame.md) | [**call\_stack**](#variable-call_stack)  <br> |
|  uint16\_t | [**global\_count**](#variable-global_count)  <br> |
|  uint64\_t | [**globals**](#variable-globals)  <br> |
|  uint16\_t | [**host\_func\_count**](#variable-host_func_count)  <br> |
|  [**SYN\_WASM\_HostFunc**](syn__wasm_8h.md#typedef-syn_wasm_hostfunc) | [**host\_funcs**](#variable-host_funcs)  <br> |
|  uint8\_t | [**label\_depth**](#variable-label_depth)  <br> |
|  [**SYN\_WASM\_Label**](structSYN__WASM__Label.md) | [**label\_stack**](#variable-label_stack)  <br> |
|  uint8\_t \* | [**linear\_mem**](#variable-linear_mem)  <br> |
|  uint32\_t | [**linear\_mem\_size**](#variable-linear_mem_size)  <br> |
|  uint16\_t | [**local\_count**](#variable-local_count)  <br> |
|  uint64\_t | [**locals**](#variable-locals)  <br> |
|  const [**SYN\_WASM\_Module**](structSYN__WASM__Module.md) \*uint32\_t | [**pc**](#variable-pc)  <br> |
|  uint32\_t | [**sp**](#variable-sp)  <br> |
|  uint64\_t | [**stack**](#variable-stack)  <br> |
|  [**SYN\_WASM\_Status**](syn__wasm_8h.md#enum-syn_wasm_status) | [**status**](#variable-status)  <br> |
|  void \* | [**user\_ctx**](#variable-user_ctx)  <br> |












































## Public Attributes Documentation




### variable call\_depth 

```C++
uint8_t SYN_WASM_Context::call_depth;
```



Active call depth 


        

<hr>



### variable call\_stack 

```C++
SYN_WASM_CallFrame SYN_WASM_Context::call_stack[SYN_WASM_MAX_CALL_DEPTH];
```



Call frame stack array 


        

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



### variable host\_func\_count 

```C++
uint16_t SYN_WASM_Context::host_func_count;
```



Registered host function count 


        

<hr>



### variable host\_funcs 

```C++
SYN_WASM_HostFunc SYN_WASM_Context::host_funcs[SYN_WASM_MAX_HOST_FUNCS];
```



Host function table 


        

<hr>



### variable label\_depth 

```C++
uint8_t SYN_WASM_Context::label_depth;
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



### variable linear\_mem\_size 

```C++
uint32_t SYN_WASM_Context::linear_mem_size;
```



Size of linear RAM memory buffer in bytes 


        

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



### variable pc 

```C++
const SYN_WASM_Module* uint32_t SYN_WASM_Context::pc;
```



&lt; Pointer to target Wasm module Program counter opcode offset 


        

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



### variable user\_ctx 

```C++
void* SYN_WASM_Context::user_ctx;
```



Custom user context pointer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/vm/syn_wasm.h`

