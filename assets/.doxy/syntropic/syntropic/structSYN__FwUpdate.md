

# Struct SYN\_FwUpdate



[**ClassList**](annotated.md) **>** [**SYN\_FwUpdate**](structSYN__FwUpdate.md)



_Firmware update context - manages streaming writes to flash._ 

* `#include <syn_fwupdate.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  uint32\_t | [**bytes\_written**](#variable-bytes_written)  <br> |
|  uint32\_t | [**crc\_state**](#variable-crc_state)  <br> |
|  uint32\_t | [**data\_addr**](#variable-data_addr)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**error**](#variable-error)  <br> |
|  uint32\_t | [**max\_size**](#variable-max_size)  <br> |
|  uint8\_t \* | [**page\_buf**](#variable-page_buf)  <br> |
|  uint16\_t | [**page\_buf\_size**](#variable-page_buf_size)  <br> |
|  uint16\_t | [**page\_buf\_used**](#variable-page_buf_used)  <br> |
|  uint32\_t | [**slot\_addr**](#variable-slot_addr)  <br> |












































## Public Attributes Documentation




### variable active 

```C++
bool SYN_FwUpdate::active;
```



Update in progress? 
 


        

<hr>



### variable bytes\_written 

```C++
uint32_t SYN_FwUpdate::bytes_written;
```



Total bytes written so far 
 


        

<hr>



### variable crc\_state 

```C++
uint32_t SYN_FwUpdate::crc_state;
```



Running CRC-32 state 
 


        

<hr>



### variable data\_addr 

```C++
uint32_t SYN_FwUpdate::data_addr;
```



Flash address for image data (after hdr) 


        

<hr>



### variable error 

```C++
bool SYN_FwUpdate::error;
```



Error occurred? 
 


        

<hr>



### variable max\_size 

```C++
uint32_t SYN_FwUpdate::max_size;
```



Maximum image size (excl. header) 
 


        

<hr>



### variable page\_buf 

```C++
uint8_t* SYN_FwUpdate::page_buf;
```



Caller-provided write buffer 
 


        

<hr>



### variable page\_buf\_size 

```C++
uint16_t SYN_FwUpdate::page_buf_size;
```



Buffer size (flash page granularity) 
 


        

<hr>



### variable page\_buf\_used 

```C++
uint16_t SYN_FwUpdate::page_buf_used;
```



Bytes currently buffered 
 


        

<hr>



### variable slot\_addr 

```C++
uint32_t SYN_FwUpdate::slot_addr;
```



Flash base address of target slot 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_fwupdate.h`

