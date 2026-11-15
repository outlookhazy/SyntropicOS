

# Struct SYN\_PortDmaTransfer



[**ClassList**](annotated.md) **>** [**SYN\_PortDmaTransfer**](structSYN__PortDmaTransfer.md)



_Hardware DMA transfer hardware request parameters._ 

* `#include <syn_port_dma.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**channel\_id**](#variable-channel_id)  <br> |
|  size\_t | [**count**](#variable-count)  <br> |
|  [**SYN\_DMA\_Size**](syn__port__dma_8h.md#enum-syn_dma_size) | [**data\_size**](#variable-data_size)  <br> |
|  [**SYN\_DMA\_Direction**](syn__port__dma_8h.md#enum-syn_dma_direction) | [**dir**](#variable-dir)  <br> |
|  void \* | [**dst**](#variable-dst)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dst\_inc**](#variable-dst_inc)  <br> |
|  const void \* | [**src**](#variable-src)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**src\_inc**](#variable-src_inc)  <br> |












































## Public Attributes Documentation




### variable channel\_id 

```C++
uint8_t SYN_PortDmaTransfer::channel_id;
```



Hardware DMA channel index 


        

<hr>



### variable count 

```C++
size_t SYN_PortDmaTransfer::count;
```



Transfer count (elements) 


        

<hr>



### variable data\_size 

```C++
SYN_DMA_Size SYN_PortDmaTransfer::data_size;
```



Data element size (1, 2, 4) 


        

<hr>



### variable dir 

```C++
SYN_DMA_Direction SYN_PortDmaTransfer::dir;
```



Transfer direction 


        

<hr>



### variable dst 

```C++
void* SYN_PortDmaTransfer::dst;
```



Destination address 


        

<hr>



### variable dst\_inc 

```C++
bool SYN_PortDmaTransfer::dst_inc;
```



Auto-increment destination address 


        

<hr>



### variable src 

```C++
const void* SYN_PortDmaTransfer::src;
```



Source address 


        

<hr>



### variable src\_inc 

```C++
bool SYN_PortDmaTransfer::src_inc;
```



Auto-increment source address 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_dma.h`

