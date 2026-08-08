

# Struct SYN\_ParamStore



[**ClassList**](annotated.md) **>** [**SYN\_ParamStore**](structSYN__ParamStore.md)



_Wear-leveled flash parameter store instance._ 

* `#include <syn_param.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**active\_sector**](#variable-active_sector)  <br> |
|  uint16\_t | [**active\_slot**](#variable-active_slot)  <br> |
|  uint16\_t | [**data\_size**](#variable-data_size)  <br> |
|  uint32\_t | [**flash\_base**](#variable-flash_base)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**initialized**](#variable-initialized)  <br> |
|  uint16\_t | [**next\_seq**](#variable-next_seq)  <br> |
|  uint8\_t | [**sector\_count**](#variable-sector_count)  <br> |
|  uint32\_t | [**sector\_size**](#variable-sector_size)  <br> |
|  uint16\_t | [**slot\_size**](#variable-slot_size)  <br> |
|  uint16\_t | [**slots\_per\_sector**](#variable-slots_per_sector)  <br> |












































## Public Attributes Documentation




### variable active\_sector 

```C++
uint8_t SYN_ParamStore::active_sector;
```



Sector containing the active slot 
 


        

<hr>



### variable active\_slot 

```C++
uint16_t SYN_ParamStore::active_slot;
```



Slot index within the active sector 
 


        

<hr>



### variable data\_size 

```C++
uint16_t SYN_ParamStore::data_size;
```



Size of the parameter data block 
 


        

<hr>



### variable flash\_base 

```C++
uint32_t SYN_ParamStore::flash_base;
```



Start address of the flash region 
 


        

<hr>



### variable initialized 

```C++
bool SYN_ParamStore::initialized;
```



Init complete 
 


        

<hr>



### variable next\_seq 

```C++
uint16_t SYN_ParamStore::next_seq;
```



Next sequence number to use 
 


        

<hr>



### variable sector\_count 

```C++
uint8_t SYN_ParamStore::sector_count;
```



Number of sectors allocated 
 


        

<hr>



### variable sector\_size 

```C++
uint32_t SYN_ParamStore::sector_size;
```



Size of one flash sector (bytes) 
 


        

<hr>



### variable slot\_size 

```C++
uint16_t SYN_ParamStore::slot_size;
```



sizeof(header) + data\_size, aligned 
 


        

<hr>



### variable slots\_per\_sector 

```C++
uint16_t SYN_ParamStore::slots_per_sector;
```



Slots that fit in one sector 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_param.h`

