

# Struct SYN\_CCP\_MemRegion



[**ClassList**](annotated.md) **>** [**SYN\_CCP\_MemRegion**](structSYN__CCP__MemRegion.md)



_Registered memory region descriptor for bounds-checked MTA/DAQ access._ 

* `#include <syn_ccp.h>`





















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
uint32_t SYN_CCP_MemRegion::base_addr;
```



32-bit CCP protocol base address 


        

<hr>



### variable host\_ptr 

```C++
uint8_t* SYN_CCP_MemRegion::host_ptr;
```



Backing host memory pointer 


        

<hr>



### variable size 

```C++
uint32_t SYN_CCP_MemRegion::size;
```



Region byte length 


        

<hr>



### variable writable 

```C++
bool SYN_CCP_MemRegion::writable;
```



True if downloads/programming are permitted 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ccp.h`

