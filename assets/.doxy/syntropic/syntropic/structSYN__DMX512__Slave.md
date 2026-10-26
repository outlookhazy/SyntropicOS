

# Struct SYN\_DMX512\_Slave



[**ClassList**](annotated.md) **>** [**SYN\_DMX512\_Slave**](structSYN__DMX512__Slave.md)



_DMX512 Slave Handle._ 

* `#include <syn_dmx512.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**current\_slot**](#variable-current_slot)  <br> |
|  uint16\_t | [**footprint**](#variable-footprint)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**frame\_complete**](#variable-frame_complete)  <br> |
|  uint8\_t | [**rx\_channels**](#variable-rx_channels)  <br> |
|  [**SYN\_DMX512\_RxState**](syn__dmx512_8h.md#enum-syn_dmx512_rxstate) | [**rx\_state**](#variable-rx_state)  <br> |
|  uint16\_t | [**start\_address**](#variable-start_address)  <br> |
|  uint8\_t | [**start\_code**](#variable-start_code)  <br> |












































## Public Attributes Documentation




### variable current\_slot 

```C++
uint16_t SYN_DMX512_Slave::current_slot;
```



Current slot index 
 


        

<hr>



### variable footprint 

```C++
uint16_t SYN_DMX512_Slave::footprint;
```



Number of channels used 
 


        

<hr>



### variable frame\_complete 

```C++
bool SYN_DMX512_Slave::frame_complete;
```



Frame updated flag 
 


        

<hr>



### variable rx\_channels 

```C++
uint8_t SYN_DMX512_Slave::rx_channels[SYN_DMX512_MAX_CHANNELS];
```



Local footprint channels 


        

<hr>



### variable rx\_state 

```C++
SYN_DMX512_RxState SYN_DMX512_Slave::rx_state;
```



Receiver state 
 


        

<hr>



### variable start\_address 

```C++
uint16_t SYN_DMX512_Slave::start_address;
```



Start address (1..512) 
 


        

<hr>



### variable start\_code 

```C++
uint8_t SYN_DMX512_Slave::start_code;
```



Received start code 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_dmx512.h`

