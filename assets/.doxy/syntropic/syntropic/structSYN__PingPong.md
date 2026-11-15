

# Struct SYN\_PingPong



[**ClassList**](annotated.md) **>** [**SYN\_PingPong**](structSYN__PingPong.md)



_Ping-pong (double) buffer — zero-copy DMA pattern._ 

* `#include <syn_pingpong.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  volatile size\_t | [**active\_idx**](#variable-active_idx)  <br> |
|  uint8\_t \* | [**buf**](#variable-buf)  <br> |
|  volatile size\_t | [**data\_ready**](#variable-data_ready)  <br> |
|  volatile size\_t | [**overruns**](#variable-overruns)  <br> |
|  size\_t | [**size**](#variable-size)  <br> |












































## Public Attributes Documentation




### variable active\_idx 

```C++
volatile size_t SYN_PingPong::active_idx;
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
volatile size_t SYN_PingPong::data_ready;
```



Non-zero when ready buffer has data 


        

<hr>



### variable overruns 

```C++
volatile size_t SYN_PingPong::overruns;
```



Count of swaps rejected while unconsumed 


        

<hr>



### variable size 

```C++
size_t SYN_PingPong::size;
```



Size of each buffer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_pingpong.h`

