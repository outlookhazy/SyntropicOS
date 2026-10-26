

# Struct SYN\_WASM\_CallFrame



[**ClassList**](annotated.md) **>** [**SYN\_WASM\_CallFrame**](structSYN__WASM__CallFrame.md)



_WebAssembly runtime call frame._ 

* `#include <syn_wasm.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**frame\_sp**](#variable-frame_sp)  <br> |
|  uint16\_t | [**func\_idx**](#variable-func_idx)  <br> |
|  uint16\_t | [**local\_base**](#variable-local_base)  <br> |
|  uint32\_t | [**return\_pc**](#variable-return_pc)  <br> |












































## Public Attributes Documentation




### variable frame\_sp 

```C++
uint32_t SYN_WASM_CallFrame::frame_sp;
```



Frame operand stack pointer 


        

<hr>



### variable func\_idx 

```C++
uint16_t SYN_WASM_CallFrame::func_idx;
```



Called function index 


        

<hr>



### variable local\_base 

```C++
uint16_t SYN_WASM_CallFrame::local_base;
```



Base index in locals array 


        

<hr>



### variable return\_pc 

```C++
uint32_t SYN_WASM_CallFrame::return_pc;
```



Return program counter 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/vm/syn_wasm.h`

