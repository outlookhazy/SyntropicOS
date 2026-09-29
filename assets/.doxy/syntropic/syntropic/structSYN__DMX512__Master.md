

# Struct SYN\_DMX512\_Master



[**ClassList**](annotated.md) **>** [**SYN\_DMX512\_Master**](structSYN__DMX512__Master.md)



_DMX512 Master Handle._ 

* `#include <syn_dmx512.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**channels**](#variable-channels)  <br> |
|  uint16\_t | [**num\_channels**](#variable-num_channels)  <br> |
|  uint8\_t | [**start\_code**](#variable-start_code)  <br> |












































## Public Attributes Documentation




### variable channels 

```C++
uint8_t SYN_DMX512_Master::channels[SYN_DMX512_MAX_CHANNELS];
```



Universe channel buffer (1..512) 


        

<hr>



### variable num\_channels 

```C++
uint16_t SYN_DMX512_Master::num_channels;
```



Configured universe length 
 


        

<hr>



### variable start\_code 

```C++
uint8_t SYN_DMX512_Master::start_code;
```



Active start code (default 0x00) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_dmx512.h`

