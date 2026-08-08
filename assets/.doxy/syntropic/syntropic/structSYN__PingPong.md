

# Struct SYN\_PingPong



[**ClassList**](annotated.md) **>** [**SYN\_PingPong**](structSYN__PingPong.md)



_Ping-pong (double) buffer — zero-copy DMA pattern._ 

* `#include <syn_pingpong.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**active\_idx**](#variable-active_idx)  <br> |
|  uint8\_t \* | [**buf**](#variable-buf)  <br> |
|  volatile [**bool**](syn__defs_8h.md#enum-bool) | [**data\_ready**](#variable-data_ready)  <br> |
|  size\_t | [**size**](#variable-size)  <br> |












































## Public Attributes Documentation




### variable active\_idx 

```C++
uint8_t SYN_PingPong::active_idx;
```



Index (0 or 1) of the active buffer 
 


        

<hr>



### variable buf 

```C++
uint8_t* SYN_PingPong::buf[2];
```



Two buffer pointers 
 


        

<hr>



### variable data\_ready 

```C++
volatile bool SYN_PingPong::data_ready;
```



True when ready buffer has data 
 


        

<hr>



### variable size 

```C++
size_t SYN_PingPong::size;
```



Size of each buffer 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_pingpong.h`

