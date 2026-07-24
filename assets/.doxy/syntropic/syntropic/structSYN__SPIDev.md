

# Struct SYN\_SPIDev



[**ClassList**](annotated.md) **>** [**SYN\_SPIDev**](structSYN__SPIDev.md)



_SPI device descriptor — bus, CS, read-bit convention._ 

* `#include <syn_spi_dev.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**bus**](#variable-bus)  <br> |
|  uint8\_t | [**cs**](#variable-cs)  <br> |
|  uint8\_t | [**read\_bit**](#variable-read_bit)  <br> |












































## Public Attributes Documentation




### variable bus 

```C++
uint8_t SYN_SPIDev::bus;
```



SPI bus number 
 


        

<hr>



### variable cs 

```C++
uint8_t SYN_SPIDev::cs;
```



Chip select index 
 


        

<hr>



### variable read\_bit 

```C++
uint8_t SYN_SPIDev::read_bit;
```



OR'd into register addr for reads (e.g., 0x80) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_spi_dev.h`

