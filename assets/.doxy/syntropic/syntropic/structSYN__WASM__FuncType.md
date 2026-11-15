

# Struct SYN\_WASM\_FuncType



[**ClassList**](annotated.md) **>** [**SYN\_WASM\_FuncType**](structSYN__WASM__FuncType.md)



_WebAssembly function signature descriptor (points into module bytes)._ 

* `#include <syn_wasm.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**param\_count**](#variable-param_count)  <br> |
|  uint32\_t | [**params\_offset**](#variable-params_offset)  <br> |
|  uint8\_t | [**result\_count**](#variable-result_count)  <br> |
|  uint32\_t | [**results\_offset**](#variable-results_offset)  <br> |












































## Public Attributes Documentation




### variable param\_count 

```C++
uint8_t SYN_WASM_FuncType::param_count;
```



Number of input parameters 


        

<hr>



### variable params\_offset 

```C++
uint32_t SYN_WASM_FuncType::params_offset;
```



Byte offset of parameter valtype vector in module 


        

<hr>



### variable result\_count 

```C++
uint8_t SYN_WASM_FuncType::result_count;
```



Number of return values 


        

<hr>



### variable results\_offset 

```C++
uint32_t SYN_WASM_FuncType::results_offset;
```



Byte offset of result valtype vector in module 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/vm/syn_wasm.h`

