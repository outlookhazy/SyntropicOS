

# Struct SYN\_SPI\_Transaction



[**ClassList**](annotated.md) **>** [**SYN\_SPI\_Transaction**](structSYN__SPI__Transaction.md)



_SPI Transaction Descriptor._ 

* `#include <syn_spi_queue.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**baudrate\_hz**](#variable-baudrate_hz)  <br> |
|  uint8\_t | [**bus**](#variable-bus)  <br> |
|  [**SYN\_SPI\_Queue\_Callback**](syn__spi__queue_8h.md#typedef-syn_spi_queue_callback) | [**callback**](#variable-callback)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**cs\_pin**](#variable-cs_pin)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**keep\_cs\_active**](#variable-keep_cs_active)  <br> |
|  size\_t | [**len**](#variable-len)  <br> |
|  [**SYN\_SPI\_Mode**](syn__spi_8h.md#enum-syn_spi_mode) | [**mode**](#variable-mode)  <br> |
|  uint8\_t \* | [**rx\_data**](#variable-rx_data)  <br> |
|  const uint8\_t \* | [**tx\_data**](#variable-tx_data)  <br> |
|  void \* | [**user\_data**](#variable-user_data)  <br> |












































## Public Attributes Documentation




### variable baudrate\_hz 

```C++
uint32_t SYN_SPI_Transaction::baudrate_hz;
```



Target baud rate in Hz (0 = default 1 MHz) 


        

<hr>



### variable bus 

```C++
uint8_t SYN_SPI_Transaction::bus;
```



SPI bus index 


        

<hr>



### variable callback 

```C++
SYN_SPI_Queue_Callback SYN_SPI_Transaction::callback;
```



Completion callback 


        

<hr>



### variable cs\_pin 

```C++
SYN_GPIO_Pin SYN_SPI_Transaction::cs_pin;
```



Target Chip Select GPIO pin 


        

<hr>



### variable keep\_cs\_active 

```C++
bool SYN_SPI_Transaction::keep_cs_active;
```



If true, leaves CS low upon transfer completion 


        

<hr>



### variable len 

```C++
size_t SYN_SPI_Transaction::len;
```



Transfer byte count 


        

<hr>



### variable mode 

```C++
SYN_SPI_Mode SYN_SPI_Transaction::mode;
```



Target SPI mode (0-3) 


        

<hr>



### variable rx\_data 

```C++
uint8_t* SYN_SPI_Transaction::rx_data;
```



RX buffer (NULL → discard RX) 


        

<hr>



### variable tx\_data 

```C++
const uint8_t* SYN_SPI_Transaction::tx_data;
```



TX buffer (NULL → send dummy 0xFF) 


        

<hr>



### variable user\_data 

```C++
void* SYN_SPI_Transaction::user_data;
```



User context pointer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_spi_queue.h`

