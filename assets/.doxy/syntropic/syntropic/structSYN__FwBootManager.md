

# Struct SYN\_FwBootManager



[**ClassList**](annotated.md) **>** [**SYN\_FwBootManager**](structSYN__FwBootManager.md)



_A/B firmware boot manager — slot selection and rollback state._ 

* `#include <syn_fwboot.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**active\_slot**](#variable-active_slot)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**initialized**](#variable-initialized)  <br> |
|  uint32\_t | [**slot\_addr**](#variable-slot_addr)  <br> |
|  [**SYN\_FwImageHeader**](structSYN__FwImageHeader.md) | [**slot\_hdr**](#variable-slot_hdr)  <br> |












































## Public Attributes Documentation




### variable active\_slot 

```C++
uint8_t SYN_FwBootManager::active_slot;
```



Currently running slot 
 


        

<hr>



### variable initialized 

```C++
bool SYN_FwBootManager::initialized;
```



Init complete 
 


        

<hr>



### variable slot\_addr 

```C++
uint32_t SYN_FwBootManager::slot_addr[2];
```



Flash base of slot A and B 
 


        

<hr>



### variable slot\_hdr 

```C++
SYN_FwImageHeader SYN_FwBootManager::slot_hdr[2];
```



Cached headers 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_fwboot.h`

