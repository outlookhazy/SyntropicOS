

# Struct SYN\_DMA\_RingBuf



[**ClassList**](annotated.md) **>** [**SYN\_DMA\_RingBuf**](structSYN__DMA__RingBuf.md)



_Continuous circular DMA reception stream controller._ 

* `#include <syn_dma.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t \* | [**buf**](#variable-buf)  <br> |
|  size\_t | [**capacity**](#variable-capacity)  <br> |
|  [**SYN\_DMA**](structSYN__DMA.md) \* | [**dma**](#variable-dma)  <br> |
|  volatile size\_t | [**tail**](#variable-tail)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wrap\_drained**](#variable-wrap_drained)  <br> |












































## Public Attributes Documentation




### variable buf 

```C++
uint8_t* SYN_DMA_RingBuf::buf;
```



Caller-allocated circular reception buffer 


        

<hr>



### variable capacity 

```C++
size_t SYN_DMA_RingBuf::capacity;
```



Total capacity of buffer in bytes 


        

<hr>



### variable dma 

```C++
SYN_DMA* SYN_DMA_RingBuf::dma;
```



Underlying DMA channel 


        

<hr>



### variable tail 

```C++
volatile size_t SYN_DMA_RingBuf::tail;
```



Application read pointer index 


        

<hr>



### variable wrap\_drained 

```C++
bool SYN_DMA_RingBuf::wrap_drained;
```



True when full capacity drained at remaining=0 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_dma.h`

