

# Struct SYN\_WASM\_Module



[**ClassList**](annotated.md) **>** [**SYN\_WASM\_Module**](structSYN__WASM__Module.md)



_Parsed WebAssembly module header and export registry._ 

* `#include <syn_wasm.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const uint8\_t \* | [**bytes**](#variable-bytes)  <br> |
|  uint16\_t | [**data\_seg\_count**](#variable-data_seg_count)  <br> |
|  struct [**SYN\_WASM\_Module**](structSYN__WASM__Module.md) | [**data\_segs**](#variable-data_segs)  <br> |
|  uint16\_t | [**export\_count**](#variable-export_count)  <br> |
|  struct [**SYN\_WASM\_Module**](structSYN__WASM__Module.md) | [**exports**](#variable-exports)  <br> |
|  uint16\_t | [**func\_count**](#variable-func_count)  <br> |
|  uint16\_t | [**func\_idx**](#variable-func_idx)  <br> |
|  [**SYN\_WASM\_FuncDef**](structSYN__WASM__FuncDef.md) | [**funcs**](#variable-funcs)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_memory**](#variable-has_memory)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_start\_func**](#variable-has_start_func)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_table**](#variable-has_table)  <br> |
|  uint16\_t | [**import\_func\_count**](#variable-import_func_count)  <br> |
|  uint16\_t | [**import\_global\_count**](#variable-import_global_count)  <br> |
|  uint8\_t | [**kind**](#variable-kind)  <br> |
|  [**SYN\_WASM\_LoadError**](syn__wasm_8h.md#enum-syn_wasm_loaderror) | [**load\_error**](#variable-load_error)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**mem\_has\_max**](#variable-mem_has_max)  <br> |
|  uint32\_t | [**mem\_max\_pages**](#variable-mem_max_pages)  <br> |
|  uint32\_t | [**mem\_min\_pages**](#variable-mem_min_pages)  <br> |
|  uint16\_t | [**name\_len**](#variable-name_len)  <br> |
|  uint32\_t | [**name\_offset**](#variable-name_offset)  <br> |
|  uint32\_t | [**offset**](#variable-offset)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**passive**](#variable-passive)  <br> |
|  uint32\_t | [**size**](#variable-size)  <br> |
|  uint32\_t | [**start\_func\_idx**](#variable-start_func_idx)  <br> |
|  uint16\_t | [**table\_element\_count**](#variable-table_element_count)  <br> |
|  uint16\_t | [**table\_elements**](#variable-table_elements)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**table\_has\_max**](#variable-table_has_max)  <br> |
|  uint32\_t | [**table\_max\_elems**](#variable-table_max_elems)  <br> |
|  uint32\_t | [**table\_min\_elems**](#variable-table_min_elems)  <br> |
|  uint16\_t | [**type\_count**](#variable-type_count)  <br> |
|  [**SYN\_WASM\_FuncType**](structSYN__WASM__FuncType.md) | [**types**](#variable-types)  <br> |












































## Public Attributes Documentation




### variable bytes 

```C++
const uint8_t* SYN_WASM_Module::bytes;
```



Pointer to raw WebAssembly binary 


        

<hr>



### variable data\_seg\_count 

```C++
uint16_t SYN_WASM_Module::data_seg_count;
```



Parsed data segment count 


        

<hr>



### variable data\_segs 

```C++
struct SYN_WASM_Module SYN_WASM_Module::data_segs[SYN_WASM_MAX_DATA_SEGS];
```



Data segment descriptors 


        

<hr>



### variable export\_count 

```C++
uint16_t SYN_WASM_Module::export_count;
```



Export count 


        

<hr>



### variable exports 

```C++
struct SYN_WASM_Module SYN_WASM_Module::exports[SYN_WASM_MAX_EXPORTS];
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



Export target index (function/table/memory/global) 


        

<hr>



### variable funcs 

```C++
SYN_WASM_FuncDef SYN_WASM_Module::funcs[SYN_WASM_MAX_FUNCTIONS];
```



Function registry 


        

<hr>



### variable has\_memory 

```C++
bool SYN_WASM_Module::has_memory;
```



True if module declares/imports linear memory 


        

<hr>



### variable has\_start\_func 

```C++
bool SYN_WASM_Module::has_start_func;
```



True if module has start function 


        

<hr>



### variable has\_table 

```C++
bool SYN_WASM_Module::has_table;
```



True if module declares/imports table 


        

<hr>



### variable import\_func\_count 

```C++
uint16_t SYN_WASM_Module::import_func_count;
```



Imported host function count 


        

<hr>



### variable import\_global\_count 

```C++
uint16_t SYN_WASM_Module::import_global_count;
```



Imported global variable count 


        

<hr>



### variable kind 

```C++
uint8_t SYN_WASM_Module::kind;
```



Export kind: 0=func, 1=table, 2=memory, 3=global 


        

<hr>



### variable load\_error 

```C++
SYN_WASM_LoadError SYN_WASM_Module::load_error;
```



Most recent load-time validation status 


        

<hr>



### variable mem\_has\_max 

```C++
bool SYN_WASM_Module::mem_has_max;
```



True if linear memory has an upper page limit 


        

<hr>



### variable mem\_max\_pages 

```C++
uint32_t SYN_WASM_Module::mem_max_pages;
```



Declared maximum linear memory size in 64 KiB pages 


        

<hr>



### variable mem\_min\_pages 

```C++
uint32_t SYN_WASM_Module::mem_min_pages;
```



Declared minimum linear memory size in 64 KiB pages 


        

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



### variable offset 

```C++
uint32_t SYN_WASM_Module::offset;
```



Byte offset of passive payload in module bytes 


        

<hr>



### variable passive 

```C++
bool SYN_WASM_Module::passive;
```



True if passive segment usable by memory.init 


        

<hr>



### variable size 

```C++
uint32_t SYN_WASM_Module::size;
```



Size of binary in bytes


Byte length of passive payload (0 if active) 


        

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
uint16_t SYN_WASM_Module::table_elements[SYN_WASM_MAX_TABLE_ELEMS];
```



Element table for indirect calls 


        

<hr>



### variable table\_has\_max 

```C++
bool SYN_WASM_Module::table_has_max;
```



True if table has an upper limit 


        

<hr>



### variable table\_max\_elems 

```C++
uint32_t SYN_WASM_Module::table_max_elems;
```



Declared maximum table size 


        

<hr>



### variable table\_min\_elems 

```C++
uint32_t SYN_WASM_Module::table_min_elems;
```



Declared minimum table size 


        

<hr>



### variable type\_count 

```C++
uint16_t SYN_WASM_Module::type_count;
```



Parsed function signature count 


        

<hr>



### variable types 

```C++
SYN_WASM_FuncType SYN_WASM_Module::types[SYN_WASM_MAX_TYPES];
```



Function signature registry 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/vm/syn_wasm.h`

