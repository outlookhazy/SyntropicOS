

# Struct SYN\_UDS\_DIDEntry



[**ClassList**](annotated.md) **>** [**SYN\_UDS\_DIDEntry**](structSYN__UDS__DIDEntry.md)



_Data Identifier (DID) Registry Entry._ 

* `#include <syn_uds.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t \* | [**data**](#variable-data)  <br> |
|  uint16\_t | [**did**](#variable-did)  <br> |
|  uint16\_t | [**len**](#variable-len)  <br> |
|  uint16\_t | [**security\_mask**](#variable-security_mask)  <br> |
|  uint8\_t | [**session\_mask**](#variable-session_mask)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**writable**](#variable-writable)  <br> |












































## Public Attributes Documentation




### variable data 

```C++
uint8_t* SYN_UDS_DIDEntry::data;
```



Pointer to DID data buffer 


        

<hr>



### variable did 

```C++
uint16_t SYN_UDS_DIDEntry::did;
```



16-bit DID identifier code 


        

<hr>



### variable len 

```C++
uint16_t SYN_UDS_DIDEntry::len;
```



Byte length of DID data 


        

<hr>



### variable security\_mask 

```C++
uint16_t SYN_UDS_DIDEntry::security_mask;
```



Permitted security level bitmask 


        

<hr>



### variable session\_mask 

```C++
uint8_t SYN_UDS_DIDEntry::session_mask;
```



Permitted session bitmask 


        

<hr>



### variable writable 

```C++
bool SYN_UDS_DIDEntry::writable;
```



True if DID is writable 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_uds.h`

