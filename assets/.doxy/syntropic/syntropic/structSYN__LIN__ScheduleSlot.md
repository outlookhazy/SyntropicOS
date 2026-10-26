

# Struct SYN\_LIN\_ScheduleSlot



[**ClassList**](annotated.md) **>** [**SYN\_LIN\_ScheduleSlot**](structSYN__LIN__ScheduleSlot.md)



_Schedule table slot entry for LIN Master._ 

* `#include <syn_lin.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_LIN\_ChecksumMode**](syn__lin_8h.md#enum-syn_lin_checksummode) | [**checksum\_mode**](#variable-checksum_mode)  <br> |
|  uint8\_t | [**data**](#variable-data)  <br> |
|  uint32\_t | [**delay\_ms**](#variable-delay_ms)  <br> |
|  [**SYN\_LIN\_SlotDirection**](syn__lin_8h.md#enum-syn_lin_slotdirection) | [**dir**](#variable-dir)  <br> |
|  uint8\_t | [**id**](#variable-id)  <br> |
|  uint8\_t | [**len**](#variable-len)  <br> |












































## Public Attributes Documentation




### variable checksum\_mode 

```C++
SYN_LIN_ChecksumMode SYN_LIN_ScheduleSlot::checksum_mode;
```



Classic vs. Enhanced checksum 


        

<hr>



### variable data 

```C++
uint8_t SYN_LIN_ScheduleSlot::data[SYN_LIN_DATA_MAX];
```



Data to publish (if PUBLISH) 


        

<hr>



### variable delay\_ms 

```C++
uint32_t SYN_LIN_ScheduleSlot::delay_ms;
```



Delay slot duration in milliseconds 


        

<hr>



### variable dir 

```C++
SYN_LIN_SlotDirection SYN_LIN_ScheduleSlot::dir;
```



Master publish or subscribe 


        

<hr>



### variable id 

```C++
uint8_t SYN_LIN_ScheduleSlot::id;
```



6-bit LIN Frame ID (0..63) 


        

<hr>



### variable len 

```C++
uint8_t SYN_LIN_ScheduleSlot::len;
```



Payload length (1..8 bytes) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lin.h`

