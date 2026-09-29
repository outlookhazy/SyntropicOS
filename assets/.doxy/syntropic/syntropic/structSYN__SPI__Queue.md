

# Struct SYN\_SPI\_Queue



[**ClassList**](annotated.md) **>** [**SYN\_SPI\_Queue**](structSYN__SPI__Queue.md)



_SPI Transaction Queue instance handle._ 

* `#include <syn_spi_queue.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  uint8\_t | [**bus**](#variable-bus)  <br> |
|  uint16\_t | [**count**](#variable-count)  <br> |
|  uint16\_t | [**head**](#variable-head)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**initialized**](#variable-initialized)  <br> |
|  [**SYN\_SPI\_Transaction**](structSYN__SPI__Transaction.md) | [**ring**](#variable-ring)  <br> |
|  uint16\_t | [**tail**](#variable-tail)  <br> |












































## Public Attributes Documentation




### variable active 

```C++
bool SYN_SPI_Queue::active;
```



Transaction in progress 


        

<hr>



### variable bus 

```C++
uint8_t SYN_SPI_Queue::bus;
```



Target SPI bus index 


        

<hr>



### variable count 

```C++
uint16_t SYN_SPI_Queue::count;
```



Current enqueued count 


        

<hr>



### variable head 

```C++
uint16_t SYN_SPI_Queue::head;
```



Head index (pop position) 


        

<hr>



### variable initialized 

```C++
bool SYN_SPI_Queue::initialized;
```



Initialization flag 


        

<hr>



### variable ring 

```C++
SYN_SPI_Transaction SYN_SPI_Queue::ring[SYN_SPI_QUEUE_MAX_DEPTH];
```



Transaction ring buffer 


        

<hr>



### variable tail 

```C++
uint16_t SYN_SPI_Queue::tail;
```



Tail index (push position) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_spi_queue.h`

