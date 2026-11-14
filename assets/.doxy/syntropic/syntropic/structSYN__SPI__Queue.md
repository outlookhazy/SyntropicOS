

# Struct SYN\_SPI\_Queue



[**ClassList**](annotated.md) **>** [**SYN\_SPI\_Queue**](structSYN__SPI__Queue.md)



_SPI Transaction Queue instance handle._ 

* `#include <syn_spi_queue.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  uint8\_t | [**bus**](#variable-bus)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**bus\_configured**](#variable-bus_configured)  <br> |
|  uint32\_t | [**configured\_baud\_hz**](#variable-configured_baud_hz)  <br> |
|  uint8\_t | [**configured\_bus**](#variable-configured_bus)  <br> |
|  uint8\_t | [**configured\_mode**](#variable-configured_mode)  <br> |
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



### variable bus\_configured 

```C++
bool SYN_SPI_Queue::bus_configured;
```



True once bus initialized 


        

<hr>



### variable configured\_baud\_hz 

```C++
uint32_t SYN_SPI_Queue::configured_baud_hz;
```



Last configured clock in Hz 


        

<hr>



### variable configured\_bus 

```C++
uint8_t SYN_SPI_Queue::configured_bus;
```



Last configured bus index 


        

<hr>



### variable configured\_mode 

```C++
uint8_t SYN_SPI_Queue::configured_mode;
```



Last configured SPI mode 


        

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

