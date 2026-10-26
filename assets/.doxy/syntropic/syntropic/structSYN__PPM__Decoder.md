

# Struct SYN\_PPM\_Decoder



[**ClassList**](annotated.md) **>** [**SYN\_PPM\_Decoder**](structSYN__PPM__Decoder.md)



[More...](#detailed-description)

* `#include <syn_ppm.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**channel\_count**](#variable-channel_count)  <br> |
|  uint16\_t | [**channels**](#variable-channels)  <br> |
|  uint8\_t | [**current\_channel**](#variable-current_channel)  <br> |
|  uint32\_t | [**frames\_received**](#variable-frames_received)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**in\_frame**](#variable-in_frame)  <br> |












































## Detailed Description


PPM Decoder Instance. 


    
## Public Attributes Documentation




### variable channel\_count 

```C++
uint8_t SYN_PPM_Decoder::channel_count;
```



Total channels detected in frame. 


        

<hr>



### variable channels 

```C++
uint16_t SYN_PPM_Decoder::channels[SYN_PPM_MAX_CHANNELS];
```



Channel pulse widths in microseconds. 


        

<hr>



### variable current\_channel 

```C++
uint8_t SYN_PPM_Decoder::current_channel;
```



Current decoding channel index. 


        

<hr>



### variable frames\_received 

```C++
uint32_t SYN_PPM_Decoder::frames_received;
```



Total valid PPM frames received. 


        

<hr>



### variable in\_frame 

```C++
bool SYN_PPM_Decoder::in_frame;
```



True if decoder is synced to frame boundary. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/input/syn_ppm.h`

