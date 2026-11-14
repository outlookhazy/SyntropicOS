

# Struct SYN\_DMA\_Config



[**ClassList**](annotated.md) **>** [**SYN\_DMA\_Config**](structSYN__DMA__Config.md)



_Configuration parameters for a DMA channel._ 

* `#include <syn_dma.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_DMA\_Callback**](syn__dma_8h.md#typedef-syn_dma_callback) | [**callback**](#variable-callback)  <br> |
|  uint8\_t | [**channel\_id**](#variable-channel_id)  <br> |
|  [**SYN\_DMA\_Size**](syn__port__dma_8h.md#enum-syn_dma_size) | [**data\_size**](#variable-data_size)  <br> |
|  [**SYN\_DMA\_Direction**](syn__port__dma_8h.md#enum-syn_dma_direction) | [**dir**](#variable-dir)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dst\_inc**](#variable-dst_inc)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**src\_inc**](#variable-src_inc)  <br> |
|  void \* | [**user\_ctx**](#variable-user_ctx)  <br> |












































## Public Attributes Documentation




### variable callback 

```C++
SYN_DMA_Callback SYN_DMA_Config::callback;
```



Optional ISR event callback 


        

<hr>



### variable channel\_id 

```C++
uint8_t SYN_DMA_Config::channel_id;
```



Hardware DMA channel index 


        

<hr>



### variable data\_size 

```C++
SYN_DMA_Size SYN_DMA_Config::data_size;
```



Data width (1, 2, or 4 bytes) 


        

<hr>



### variable dir 

```C++
SYN_DMA_Direction SYN_DMA_Config::dir;
```



Transfer direction 


        

<hr>



### variable dst\_inc 

```C++
bool SYN_DMA_Config::dst_inc;
```



Auto-increment destination address 


        

<hr>



### variable src\_inc 

```C++
bool SYN_DMA_Config::src_inc;
```



Auto-increment source address 


        

<hr>



### variable user\_ctx 

```C++
void* SYN_DMA_Config::user_ctx;
```



User context passed to callback 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_dma.h`

