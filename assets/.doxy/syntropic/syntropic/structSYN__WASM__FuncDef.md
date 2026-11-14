

# Struct SYN\_WASM\_FuncDef



[**ClassList**](annotated.md) **>** [**SYN\_WASM\_FuncDef**](structSYN__WASM__FuncDef.md)



_WebAssembly function definition metadata._ 

* `#include <syn_wasm.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**code\_offset**](#variable-code_offset)  <br> |
|  uint32\_t | [**code\_size**](#variable-code_size)  <br> |
|  uint8\_t | [**param\_count**](#variable-param_count)  <br> |
|  uint8\_t | [**result\_count**](#variable-result_count)  <br> |
|  uint32\_t | [**type\_idx**](#variable-type_idx)  <br> |












































## Public Attributes Documentation




### variable code\_offset 

```C++
uint32_t SYN_WASM_FuncDef::code_offset;
```



Code offset in module bytes 


        

<hr>



### variable code\_size 

```C++
uint32_t SYN_WASM_FuncDef::code_size;
```



Size of bytecode instructions 


        

<hr>



### variable param\_count 

```C++
uint8_t SYN_WASM_FuncDef::param_count;
```



Number of input parameters 


        

<hr>



### variable result\_count 

```C++
uint8_t SYN_WASM_FuncDef::result_count;
```



Number of return values 


        

<hr>



### variable type\_idx 

```C++
uint32_t SYN_WASM_FuncDef::type_idx;
```



Type section index 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/vm/syn_wasm.h`

