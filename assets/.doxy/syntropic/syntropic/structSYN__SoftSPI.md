

# Struct SYN\_SoftSPI



[**ClassList**](annotated.md) **>** [**SYN\_SoftSPI**](structSYN__SoftSPI.md)



_Software SPI instance._ 

* `#include <syn_soft_spi.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_GPIO\_State**](syn__defs_8h.md#enum-syn_gpio_state) | [**active\_state**](#variable-active_state)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**cpha**](#variable-cpha)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**cs\_active\_low**](#variable-cs_active_low)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**cs\_pin**](#variable-cs_pin)  <br> |
|  uint32\_t | [**delay\_loops**](#variable-delay_loops)  <br> |
|  [**SYN\_GPIO\_State**](syn__defs_8h.md#enum-syn_gpio_state) | [**idle\_state**](#variable-idle_state)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**miso**](#variable-miso)  <br> |
|  [**SYN\_SPIMode**](syn__soft__spi_8h.md#enum-syn_spimode) | [**mode**](#variable-mode)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**mosi**](#variable-mosi)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**sck**](#variable-sck)  <br> |












































## Public Attributes Documentation




### variable active\_state 

```C++
SYN_GPIO_State SYN_SoftSPI::active_state;
```



Cached active pin state 


        

<hr>



### variable cpha 

```C++
bool SYN_SoftSPI::cpha;
```



Clock phase cache 


        

<hr>



### variable cs\_active\_low 

```C++
bool SYN_SoftSPI::cs_active_low;
```



CS active logic polarity 


        

<hr>



### variable cs\_pin 

```C++
SYN_GPIO_Pin SYN_SoftSPI::cs_pin;
```



Chip select GPIO pin identifier 


        

<hr>



### variable delay\_loops 

```C++
uint32_t SYN_SoftSPI::delay_loops;
```



Iteration count for half-clock software delay 


        

<hr>



### variable idle\_state 

```C++
SYN_GPIO_State SYN_SoftSPI::idle_state;
```



Cached idle pin state 


        

<hr>



### variable miso 

```C++
SYN_GPIO_Pin SYN_SoftSPI::miso;
```



MISO GPIO pin identifier 


        

<hr>



### variable mode 

```C++
SYN_SPIMode SYN_SoftSPI::mode;
```



SPI phase and polarity mode 


        

<hr>



### variable mosi 

```C++
SYN_GPIO_Pin SYN_SoftSPI::mosi;
```



MOSI GPIO pin identifier 


        

<hr>



### variable sck 

```C++
SYN_GPIO_Pin SYN_SoftSPI::sck;
```



SCK GPIO pin identifier 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_soft_spi.h`

