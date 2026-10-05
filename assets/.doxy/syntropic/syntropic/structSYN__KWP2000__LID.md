

# Struct SYN\_KWP2000\_LID



[**ClassList**](annotated.md) **>** [**SYN\_KWP2000\_LID**](structSYN__KWP2000__LID.md)



_Registered Local Identifier (LID) descriptor._ 

* `#include <syn_kwp2000.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const void \* | [**data**](#variable-data)  <br> |
|  uint16\_t | [**len**](#variable-len)  <br> |
|  uint8\_t | [**lid**](#variable-lid)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**writable**](#variable-writable)  <br> |












































## Public Attributes Documentation




### variable data 

```C++
const void* SYN_KWP2000_LID::data;
```



Pointer to data buffer 
 


        

<hr>



### variable len 

```C++
uint16_t SYN_KWP2000_LID::len;
```



Length of data buffer in bytes 
 


        

<hr>



### variable lid 

```C++
uint8_t SYN_KWP2000_LID::lid;
```



1-byte Local Identifier (e.g. 0x01, 0x10) 


        

<hr>



### variable writable 

```C++
bool SYN_KWP2000_LID::writable;
```



Is LID writable via Service 0x3B/0x2E? 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_kwp2000.h`

