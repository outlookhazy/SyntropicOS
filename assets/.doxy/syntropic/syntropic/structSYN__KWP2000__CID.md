

# Struct SYN\_KWP2000\_CID



[**ClassList**](annotated.md) **>** [**SYN\_KWP2000\_CID**](structSYN__KWP2000__CID.md)



_Registered Common Identifier (CID) descriptor._ 

* `#include <syn_kwp2000.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**cid**](#variable-cid)  <br> |
|  const void \* | [**data**](#variable-data)  <br> |
|  uint16\_t | [**len**](#variable-len)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**writable**](#variable-writable)  <br> |












































## Public Attributes Documentation




### variable cid 

```C++
uint16_t SYN_KWP2000_CID::cid;
```



2-byte Common Identifier (e.g. 0xF190) 
 


        

<hr>



### variable data 

```C++
const void* SYN_KWP2000_CID::data;
```



Pointer to data buffer 
 


        

<hr>



### variable len 

```C++
uint16_t SYN_KWP2000_CID::len;
```



Length of data buffer in bytes 
 


        

<hr>



### variable writable 

```C++
bool SYN_KWP2000_CID::writable;
```



Is CID writable via Service 0x2E? 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_kwp2000.h`

