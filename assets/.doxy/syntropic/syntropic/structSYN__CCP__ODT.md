

# Struct SYN\_CCP\_ODT



[**ClassList**](annotated.md) **>** [**SYN\_CCP\_ODT**](structSYN__CCP__ODT.md)



_ODT (Object Descriptor Table) structure._ 

* `#include <syn_ccp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_CCP\_ODTEntry**](structSYN__CCP__ODTEntry.md) | [**entries**](#variable-entries)  <br> |
|  uint8\_t | [**entry\_count**](#variable-entry_count)  <br> |












































## Public Attributes Documentation




### variable entries 

```C++
SYN_CCP_ODTEntry SYN_CCP_ODT::entries[SYN_CCP_MAX_ENTRIES_PER_ODT];
```



ODT entry array 


        

<hr>



### variable entry\_count 

```C++
uint8_t SYN_CCP_ODT::entry_count;
```



Active entry count 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ccp.h`

