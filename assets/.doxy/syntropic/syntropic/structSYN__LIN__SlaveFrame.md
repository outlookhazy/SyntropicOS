

# Struct SYN\_LIN\_SlaveFrame



[**ClassList**](annotated.md) **>** [**SYN\_LIN\_SlaveFrame**](structSYN__LIN__SlaveFrame.md)



_Configuration for a single frame ID handled by a LIN Slave._ 

* `#include <syn_lin.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  [**SYN\_LIN\_ChecksumMode**](syn__lin_8h.md#enum-syn_lin_checksummode) | [**checksum\_mode**](#variable-checksum_mode)  <br> |
|  uint8\_t | [**data**](#variable-data)  <br> |
|  [**SYN\_LIN\_SlotDirection**](syn__lin_8h.md#enum-syn_lin_slotdirection) | [**dir**](#variable-dir)  <br> |
|  uint8\_t | [**id**](#variable-id)  <br> |
|  uint8\_t | [**len**](#variable-len)  <br> |












































## Public Attributes Documentation




### variable active 

```C++
bool SYN_LIN_SlaveFrame::active;
```



Entry active flag 


        

<hr>



### variable checksum\_mode 

```C++
SYN_LIN_ChecksumMode SYN_LIN_SlaveFrame::checksum_mode;
```



Classic vs. Enhanced checksum 


        

<hr>



### variable data 

```C++
uint8_t SYN_LIN_SlaveFrame::data[SYN_LIN_DATA_MAX];
```



Published data buffer 


        

<hr>



### variable dir 

```C++
SYN_LIN_SlotDirection SYN_LIN_SlaveFrame::dir;
```



SUBSCRIBE = slave receives, PUBLISH = slave transmits 


        

<hr>



### variable id 

```C++
uint8_t SYN_LIN_SlaveFrame::id;
```



6-bit Frame ID (0..63) 


        

<hr>



### variable len 

```C++
uint8_t SYN_LIN_SlaveFrame::len;
```



Payload length (1..8 bytes) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lin.h`

