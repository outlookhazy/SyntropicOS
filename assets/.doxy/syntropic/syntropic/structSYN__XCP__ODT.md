

# Struct SYN\_XCP\_ODT



[**ClassList**](annotated.md) **>** [**SYN\_XCP\_ODT**](structSYN__XCP__ODT.md)



_ODT (Object Descriptor Table) structure._ 

* `#include <syn_xcp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_XCP\_ODTEntry**](structSYN__XCP__ODTEntry.md) | [**entries**](#variable-entries)  <br> |
|  uint8\_t | [**entry\_count**](#variable-entry_count)  <br> |












































## Public Attributes Documentation




### variable entries 

```C++
SYN_XCP_ODTEntry SYN_XCP_ODT::entries[SYN_XCP_MAX_ENTRIES_PER_ODT];
```



ODT entries array 


        

<hr>



### variable entry\_count 

```C++
uint8_t SYN_XCP_ODT::entry_count;
```



Active entry count 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_xcp.h`

