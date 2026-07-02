

# Struct SYN\_DMA\_Config



[**ClassList**](annotated.md) **>** [**SYN\_DMA\_Config**](structSYN__DMA__Config.md)



_DMA channel configuration — caller-owned._ 

* `#include <syn_port_dma.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_DMA\_Callback**](syn__port__dma_8h.md#typedef-syn_dma_callback) | [**callback**](#variable-callback)  <br> |
|  uint8\_t | [**channel**](#variable-channel)  <br> |
|  [**SYN\_DMA\_Dir**](syn__port__dma_8h.md#enum-syn_dma_dir) | [**direction**](#variable-direction)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dst\_incr**](#variable-dst_incr)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**src\_incr**](#variable-src_incr)  <br> |
|  void \* | [**user\_data**](#variable-user_data)  <br> |
|  [**SYN\_DMA\_Width**](syn__port__dma_8h.md#enum-syn_dma_width) | [**width**](#variable-width)  <br> |












































## Public Attributes Documentation




### variable callback 

```C++
SYN_DMA_Callback SYN_DMA_Config::callback;
```



Completion callback (ISR ctx) 
 


        

<hr>



### variable channel 

```C++
uint8_t SYN_DMA_Config::channel;
```



DMA channel number 
 


        

<hr>



### variable direction 

```C++
SYN_DMA_Dir SYN_DMA_Config::direction;
```



Transfer direction 
 


        

<hr>



### variable dst\_incr 

```C++
bool SYN_DMA_Config::dst_incr;
```



Increment destination address? 
 


        

<hr>



### variable src\_incr 

```C++
bool SYN_DMA_Config::src_incr;
```



Increment source address? 
 


        

<hr>



### variable user\_data 

```C++
void* SYN_DMA_Config::user_data;
```



User context for callback 
 


        

<hr>



### variable width 

```C++
SYN_DMA_Width SYN_DMA_Config::width;
```



Source/dest data width 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_dma.h`

