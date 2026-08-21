

# Struct SYN\_SBC\_FrameInfo



[**ClassList**](annotated.md) **>** [**SYN\_SBC\_FrameInfo**](structSYN__SBC__FrameInfo.md)



_SBC Parsed Frame Header Information._ 

* `#include <syn_sbc.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_SBC\_AllocMethod**](syn__sbc_8h.md#enum-syn_sbc_allocmethod) | [**alloc**](#variable-alloc)  <br> |
|  uint8\_t | [**bitpool**](#variable-bitpool)  <br> |
|  uint8\_t | [**blocks**](#variable-blocks)  <br> |
|  uint8\_t | [**channels**](#variable-channels)  <br> |
|  uint16\_t | [**frame\_len**](#variable-frame_len)  <br> |
|  uint8\_t | [**join**](#variable-join)  <br> |
|  [**SYN\_SBC\_ChannelMode**](syn__sbc_8h.md#enum-syn_sbc_channelmode) | [**mode**](#variable-mode)  <br> |
|  uint8\_t | [**sample\_rate\_enum**](#variable-sample_rate_enum)  <br> |
|  uint16\_t | [**sample\_rate\_hz**](#variable-sample_rate_hz)  <br> |
|  uint8\_t | [**subbands**](#variable-subbands)  <br> |












































## Public Attributes Documentation




### variable alloc 

```C++
SYN_SBC_AllocMethod SYN_SBC_FrameInfo::alloc;
```



Allocation method 
 


        

<hr>



### variable bitpool 

```C++
uint8_t SYN_SBC_FrameInfo::bitpool;
```



Bitpool value (1-250) 
 


        

<hr>



### variable blocks 

```C++
uint8_t SYN_SBC_FrameInfo::blocks;
```



Block count (4, 8, 12, 16) 
 


        

<hr>



### variable channels 

```C++
uint8_t SYN_SBC_FrameInfo::channels;
```



Channel count (1 or 2) 
 


        

<hr>



### variable frame\_len 

```C++
uint16_t SYN_SBC_FrameInfo::frame_len;
```



Total frame length in bytes 
 


        

<hr>



### variable join 

```C++
uint8_t SYN_SBC_FrameInfo::join;
```



Joint stereo subband mask 
 


        

<hr>



### variable mode 

```C++
SYN_SBC_ChannelMode SYN_SBC_FrameInfo::mode;
```



Channel mode 
 


        

<hr>



### variable sample\_rate\_enum 

```C++
uint8_t SYN_SBC_FrameInfo::sample_rate_enum;
```



0: 16kHz, 1: 32kHz, 2: 44.1kHz, 3: 48kHz 


        

<hr>



### variable sample\_rate\_hz 

```C++
uint16_t SYN_SBC_FrameInfo::sample_rate_hz;
```



Sample rate in Hz 
 


        

<hr>



### variable subbands 

```C++
uint8_t SYN_SBC_FrameInfo::subbands;
```



Subband count (4 or 8) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_sbc.h`

