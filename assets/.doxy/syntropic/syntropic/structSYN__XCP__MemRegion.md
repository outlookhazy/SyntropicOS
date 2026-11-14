

# Struct SYN\_XCP\_MemRegion



[**ClassList**](annotated.md) **>** [**SYN\_XCP\_MemRegion**](structSYN__XCP__MemRegion.md)



_Registered memory region descriptor for bounds-checked XCP access._ 

* `#include <syn_xcp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**base\_addr**](#variable-base_addr)  <br> |
|  uint8\_t \* | [**host\_ptr**](#variable-host_ptr)  <br> |
|  uint32\_t | [**size**](#variable-size)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**writable**](#variable-writable)  <br> |












































## Public Attributes Documentation




### variable base\_addr 

```C++
uint32_t SYN_XCP_MemRegion::base_addr;
```



32-bit XCP protocol base address 


        

<hr>



### variable host\_ptr 

```C++
uint8_t* SYN_XCP_MemRegion::host_ptr;
```



Backing host memory pointer 


        

<hr>



### variable size 

```C++
uint32_t SYN_XCP_MemRegion::size;
```



Region byte length 


        

<hr>



### variable writable 

```C++
bool SYN_XCP_MemRegion::writable;
```



True if calibration/download writes allowed 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_xcp.h`

