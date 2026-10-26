

# Struct SYN\_SD



[**ClassList**](annotated.md) **>** [**SYN\_SD**](structSYN__SD.md)



_SD card driver handle (caller-owned, zero heap allocation)._ [More...](#detailed-description)

* `#include <syn_sd.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**cs\_pin**](#variable-cs_pin)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**initialized**](#variable-initialized)  <br> |
|  uint32\_t | [**sector\_count**](#variable-sector_count)  <br> |
|  uint8\_t | [**spi\_bus**](#variable-spi_bus)  <br> |
|  [**SYN\_SD\_Type**](syn__sd_8h.md#enum-syn_sd_type) | [**type**](#variable-type)  <br> |












































## Detailed Description


Allocate statically and pass to [**syn\_sd\_init()**](syn__sd_8c.md#function-syn_sd_init). Treat as opaque after init. 


    
## Public Attributes Documentation




### variable cs\_pin 

```C++
SYN_GPIO_Pin SYN_SD::cs_pin;
```



Chip-select GPIO pin 
 


        

<hr>



### variable initialized 

```C++
bool SYN_SD::initialized;
```



true after a successful [**syn\_sd\_init()**](syn__sd_8c.md#function-syn_sd_init) 
 


        

<hr>



### variable sector\_count 

```C++
uint32_t SYN_SD::sector_count;
```



Total 512-byte sectors (from CSD) 
 


        

<hr>



### variable spi\_bus 

```C++
uint8_t SYN_SD::spi_bus;
```



SPI bus index (syn\_port\_spi\_\* argument) 
 


        

<hr>



### variable type 

```C++
SYN_SD_Type SYN_SD::type;
```



Detected card type (SDSC or SDHC) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_sd.h`

