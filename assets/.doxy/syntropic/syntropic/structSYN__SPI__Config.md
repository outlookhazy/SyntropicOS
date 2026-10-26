

# Struct SYN\_SPI\_Config



[**ClassList**](annotated.md) **>** [**SYN\_SPI\_Config**](structSYN__SPI__Config.md)



_SPI driver instance configuration parameters._ 

* `#include <syn_spi.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**baudrate\_hz**](#variable-baudrate_hz)  <br> |
|  [**SYN\_SPI\_Mode**](syn__spi_8h.md#enum-syn_spi_mode) | [**mode**](#variable-mode)  <br> |
|  [**SYN\_SPI\_Role**](syn__spi_8h.md#enum-syn_spi_role) | [**role**](#variable-role)  <br> |
|  uint8\_t | [**spi\_id**](#variable-spi_id)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**use\_dma**](#variable-use_dma)  <br> |












































## Public Attributes Documentation




### variable baudrate\_hz 

```C++
uint32_t SYN_SPI_Config::baudrate_hz;
```



Clock frequency in Hz (e.g. 1000000) 


        

<hr>



### variable mode 

```C++
SYN_SPI_Mode SYN_SPI_Config::mode;
```



SPI clock phase & polarity mode 


        

<hr>



### variable role 

```C++
SYN_SPI_Role SYN_SPI_Config::role;
```



Master or Slave mode 


        

<hr>



### variable spi\_id 

```C++
uint8_t SYN_SPI_Config::spi_id;
```



Hardware SPI instance (0 = SPI1) 


        

<hr>



### variable use\_dma 

```C++
bool SYN_SPI_Config::use_dma;
```



Enable DMA transfers 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_spi.h`

