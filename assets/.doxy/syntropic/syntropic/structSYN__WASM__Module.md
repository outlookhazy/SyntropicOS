

# Struct SYN\_WASM\_Module



[**ClassList**](annotated.md) **>** [**SYN\_WASM\_Module**](structSYN__WASM__Module.md)



_Parsed WebAssembly module header and export registry._ 

* `#include <syn_wasm.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const uint8\_t \* | [**bytes**](#variable-bytes)  <br> |
|  uint16\_t | [**export\_count**](#variable-export_count)  <br> |
|  struct [**SYN\_WASM\_Module**](structSYN__WASM__Module.md) | [**exports**](#variable-exports)  <br> |
|  uint16\_t | [**func\_count**](#variable-func_count)  <br> |
|  uint16\_t | [**func\_idx**](#variable-func_idx)  <br> |
|  [**SYN\_WASM\_FuncDef**](structSYN__WASM__FuncDef.md) | [**funcs**](#variable-funcs)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_start\_func**](#variable-has_start_func)  <br> |
|  uint16\_t | [**import\_func\_count**](#variable-import_func_count)  <br> |
|  uint16\_t | [**name\_len**](#variable-name_len)  <br> |
|  uint32\_t | [**name\_offset**](#variable-name_offset)  <br> |
|  uint32\_t | [**size**](#variable-size)  <br> |
|  uint32\_t | [**start\_func\_idx**](#variable-start_func_idx)  <br> |
|  uint16\_t | [**table\_element\_count**](#variable-table_element_count)  <br> |
|  uint16\_t | [**table\_elements**](#variable-table_elements)  <br> |












































## Public Attributes Documentation




### variable bytes 

```C++
const uint8_t* SYN_WASM_Module::bytes;
```



Pointer to raw WebAssembly binary 


        

<hr>



### variable export\_count 

```C++
uint16_t SYN_WASM_Module::export_count;
```



Export count 


        

<hr>



### variable exports 

```C++
struct SYN_WASM_Module SYN_WASM_Module::exports[SYN_WASM_MAX_FUNCTIONS];
```



Export registry 


        

<hr>



### variable func\_count 

```C++
uint16_t SYN_WASM_Module::func_count;
```



Total function count 


        

<hr>



### variable func\_idx 

```C++
uint16_t SYN_WASM_Module::func_idx;
```



Function index 


        

<hr>



### variable funcs 

```C++
SYN_WASM_FuncDef SYN_WASM_Module::funcs[SYN_WASM_MAX_FUNCTIONS];
```



Function registry 


        

<hr>



### variable has\_start\_func 

```C++
bool SYN_WASM_Module::has_start_func;
```



True if module has start function 


        

<hr>



### variable import\_func\_count 

```C++
uint16_t SYN_WASM_Module::import_func_count;
```



Imported host function count 


        

<hr>



### variable name\_len 

```C++
uint16_t SYN_WASM_Module::name_len;
```



Export name length 


        

<hr>



### variable name\_offset 

```C++
uint32_t SYN_WASM_Module::name_offset;
```



Export name offset 


        

<hr>



### variable size 

```C++
uint32_t SYN_WASM_Module::size;
```



Size of binary in bytes 


        

<hr>



### variable start\_func\_idx 

```C++
uint32_t SYN_WASM_Module::start_func_idx;
```



Start function index 


        

<hr>



### variable table\_element\_count 

```C++
uint16_t SYN_WASM_Module::table_element_count;
```



Element count 


        

<hr>



### variable table\_elements 

```C++
uint16_t SYN_WASM_Module::table_elements[64];
```



Element table for indirect calls 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/vm/syn_wasm.h`

