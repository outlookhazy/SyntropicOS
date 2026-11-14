

# Struct SYN\_UART\_Config



[**ClassList**](annotated.md) **>** [**SYN\_UART\_Config**](structSYN__UART__Config.md)



_Configuration struct for UART initialization with optional DMA._ 

* `#include <syn_uart.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**baudrate**](#variable-baudrate)  <br> |
|  [**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) | [**instance**](#variable-instance)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**use\_dma**](#variable-use_dma)  <br> |












































## Public Attributes Documentation




### variable baudrate 

```C++
uint32_t SYN_UART_Config::baudrate;
```



Desired baud rate 


        

<hr>



### variable instance 

```C++
SYN_UARTInstance SYN_UART_Config::instance;
```



Hardware UART peripheral index 


        

<hr>



### variable use\_dma 

```C++
bool SYN_UART_Config::use_dma;
```



Enable DMA for this UART instance 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_uart.h`

