

# Struct SYN\_SPI\_Config



[**ClassList**](annotated.md) **>** [**SYN\_SPI\_Config**](structSYN__SPI__Config.md)



_SPI bus configuration._ 

* `#include <syn_port_spi.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**bit\_order**](#variable-bit_order)  <br> |
|  uint8\_t | [**bus**](#variable-bus)  <br> |
|  uint32\_t | [**clock\_hz**](#variable-clock_hz)  <br> |
|  [**SYN\_SPI\_Mode**](syn__port__spi_8h.md#enum-syn_spi_mode) | [**mode**](#variable-mode)  <br> |












































## Public Attributes Documentation




### variable bit\_order 

```C++
uint8_t SYN_SPI_Config::bit_order;
```



0 = MSB first (default), 1 = LSB 
 


        

<hr>



### variable bus 

```C++
uint8_t SYN_SPI_Config::bus;
```



SPI bus index (0, 1, ...) 
 


        

<hr>



### variable clock\_hz 

```C++
uint32_t SYN_SPI_Config::clock_hz;
```



SPI clock frequency 
 


        

<hr>



### variable mode 

```C++
SYN_SPI_Mode SYN_SPI_Config::mode;
```



Clock polarity / phase 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_spi.h`

