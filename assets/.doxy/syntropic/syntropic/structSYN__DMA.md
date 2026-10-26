

# Struct SYN\_DMA



[**ClassList**](annotated.md) **>** [**SYN\_DMA**](structSYN__DMA.md)



_DMA Channel instance object._ 

* `#include <syn_dma.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_DMA\_Config**](structSYN__DMA__Config.md) | [**cfg**](#variable-cfg)  <br> |
|  void \* | [**current\_dst**](#variable-current_dst)  <br> |
|  size\_t | [**current\_len**](#variable-current_len)  <br> |
|  const void \* | [**current\_src**](#variable-current_src)  <br> |
|  uint32\_t | [**errors\_cnt**](#variable-errors_cnt)  <br> |
|  [**SYN\_DMA\_State**](syn__dma_8h.md#enum-syn_dma_state) | [**state**](#variable-state)  <br> |
|  uint32\_t | [**transfers\_cnt**](#variable-transfers_cnt)  <br> |












































## Public Attributes Documentation




### variable cfg 

```C++
SYN_DMA_Config SYN_DMA::cfg;
```



Channel static configuration 


        

<hr>



### variable current\_dst 

```C++
void* SYN_DMA::current_dst;
```



Active transfer destination address 


        

<hr>



### variable current\_len 

```C++
size_t SYN_DMA::current_len;
```



Active transfer length in bytes 


        

<hr>



### variable current\_src 

```C++
const void* SYN_DMA::current_src;
```



Active transfer source address 


        

<hr>



### variable errors\_cnt 

```C++
uint32_t SYN_DMA::errors_cnt;
```



Total transfer errors count 


        

<hr>



### variable state 

```C++
SYN_DMA_State SYN_DMA::state;
```



Current channel execution state 


        

<hr>



### variable transfers\_cnt 

```C++
uint32_t SYN_DMA::transfers_cnt;
```



Total successful transfers count 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_dma.h`

