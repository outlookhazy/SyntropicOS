

# Struct SYN\_FwImageHeader



[**ClassList**](annotated.md) **>** [**SYN\_FwImageHeader**](structSYN__FwImageHeader.md)



_Firmware image header — stored at the start of each flash slot._ [More...](#detailed-description)

* `#include <syn_fwimage.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**header\_crc**](#variable-header_crc)  <br> |
|  uint32\_t | [**image\_crc**](#variable-image_crc)  <br> |
|  uint32\_t | [**image\_size**](#variable-image_size)  <br> |
|  uint32\_t | [**magic**](#variable-magic)  <br> |
|  uint8\_t | [**reserved**](#variable-reserved)  <br> |
|  uint8\_t | [**state**](#variable-state)  <br> |
|  uint32\_t | [**version\_code**](#variable-version_code)  <br> |












































## Detailed Description


Base size: 24 bytes. When SYN\_FW\_USE\_HMAC is enabled, adds a 32-byte HMAC field for a total of 56 bytes. 


    
## Public Attributes Documentation




### variable header\_crc 

```C++
uint32_t SYN_FwImageHeader::header_crc;
```



CRC-32 over all preceding fields 
 


        

<hr>



### variable image\_crc 

```C++
uint32_t SYN_FwImageHeader::image_crc;
```



CRC-32 over the firmware binary 
 


        

<hr>



### variable image\_size 

```C++
uint32_t SYN_FwImageHeader::image_size;
```



Firmware binary size (excl. header) 
 


        

<hr>



### variable magic 

```C++
uint32_t SYN_FwImageHeader::magic;
```



SYN\_FW\_MAGIC 
 


        

<hr>



### variable reserved 

```C++
uint8_t SYN_FwImageHeader::reserved[3];
```



Pad to 4-byte alignment 
 


        

<hr>



### variable state 

```C++
uint8_t SYN_FwImageHeader::state;
```



SYN\_FwState 
 


        

<hr>



### variable version\_code 

```C++
uint32_t SYN_FwImageHeader::version_code;
```



Packed version (SYN\_VERSION\_CODE fmt) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_fwimage.h`

