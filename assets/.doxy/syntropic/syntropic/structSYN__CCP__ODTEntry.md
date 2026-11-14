

# Struct SYN\_CCP\_ODTEntry



[**ClassList**](annotated.md) **>** [**SYN\_CCP\_ODTEntry**](structSYN__CCP__ODTEntry.md)



_ODT Element mapping entry._ 

* `#include <syn_ccp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uintptr\_t | [**address**](#variable-address)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**host\_valid**](#variable-host_valid)  <br> |
|  uint8\_t | [**size**](#variable-size)  <br> |












































## Public Attributes Documentation




### variable address 

```C++
uintptr_t SYN_CCP_ODTEntry::address;
```



Memory target address 


        

<hr>



### variable host\_valid 

```C++
bool SYN_CCP_ODTEntry::host_valid;
```



True if address was set via trusted host pointer 


        

<hr>



### variable size 

```C++
uint8_t SYN_CCP_ODTEntry::size;
```



Byte size (1, 2, 4 bytes) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ccp.h`

