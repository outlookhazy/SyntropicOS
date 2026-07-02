

# Struct SYN\_UART



[**ClassList**](annotated.md) **>** [**SYN\_UART**](structSYN__UART.md)



_UART driver handle._ [More...](#detailed-description)

* `#include <syn_uart.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**initialized**](#variable-initialized)  <br> |
|  [**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) | [**instance**](#variable-instance)  <br> |
|  uint8\_t | [**rx\_buf**](#variable-rx_buf)  <br> |
|  [**SYN\_RingBuf**](structSYN__RingBuf.md) | [**rx\_rb**](#variable-rx_rb)  <br> |
|  uint8\_t | [**tx\_buf**](#variable-tx_buf)  <br> |
|  [**SYN\_RingBuf**](structSYN__RingBuf.md) | [**tx\_rb**](#variable-tx_rb)  <br> |












































## Detailed Description


The user allocates this struct (stack or static) and passes it to [**syn\_uart\_init()**](syn__uart_8c.md#function-syn_uart_init). The backing buffers are embedded so no dynamic allocation is needed. 


    
## Public Attributes Documentation




### variable initialized 

```C++
bool SYN_UART::initialized;
```



Initialization flag status 


        

<hr>



### variable instance 

```C++
SYN_UARTInstance SYN_UART::instance;
```



Hardware UART peripheral index 


        

<hr>



### variable rx\_buf 

```C++
uint8_t SYN_UART::rx_buf[SYN_UART_RX_BUF_SIZE];
```



Physical storage memory for RX ring buffer 


        

<hr>



### variable rx\_rb 

```C++
SYN_RingBuf SYN_UART::rx_rb;
```



Receive ring buffer control block 


        

<hr>



### variable tx\_buf 

```C++
uint8_t SYN_UART::tx_buf[SYN_UART_TX_BUF_SIZE];
```



Physical storage memory for TX ring buffer 


        

<hr>



### variable tx\_rb 

```C++
SYN_RingBuf SYN_UART::tx_rb;
```



Transmit ring buffer control block 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_uart.h`

