

# File syn\_soft\_spi.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_soft\_spi.h**](syn__soft__spi_8h.md)

[Go to the source code of this file](syn__soft__spi_8h_source.md)

_Software SPI (bit-banging) driver._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_SoftSPI**](structSYN__SoftSPI.md) <br>_Software SPI instance._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_SPIMode**](#enum-syn_spimode)  <br>_SPI Clock Mode._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_soft\_spi\_deselect**](#function-syn_soft_spi_deselect) ([**SYN\_SoftSPI**](structSYN__SoftSPI.md) \* spi) <br>_Deassert chip select (drive inactive)._  |
|  void | [**syn\_soft\_spi\_init**](#function-syn_soft_spi_init) ([**SYN\_SoftSPI**](structSYN__SoftSPI.md) \* spi, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) sck, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) mosi, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) miso, [**SYN\_SPIMode**](syn__soft__spi_8h.md#enum-syn_spimode) mode, uint32\_t delay\_loops) <br>_Initialize the soft SPI pins._  |
|  void | [**syn\_soft\_spi\_select**](#function-syn_soft_spi_select) ([**SYN\_SoftSPI**](structSYN__SoftSPI.md) \* spi) <br>_Assert chip select (drive active)._  |
|  void | [**syn\_soft\_spi\_set\_cs**](#function-syn_soft_spi_set_cs) ([**SYN\_SoftSPI**](structSYN__SoftSPI.md) \* spi, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) cs\_pin, [**bool**](syn__defs_8h.md#enum-bool) active\_low) <br>_Set an optional chip select pin._  |
|  uint8\_t | [**syn\_soft\_spi\_transfer**](#function-syn_soft_spi_transfer) (const [**SYN\_SoftSPI**](structSYN__SoftSPI.md) \* spi, uint8\_t data) <br>_Transfer a single byte (read and write simultaneously)._  |
|  void | [**syn\_soft\_spi\_transfer\_bulk**](#function-syn_soft_spi_transfer_bulk) ([**SYN\_SoftSPI**](structSYN__SoftSPI.md) \* spi, const uint8\_t \* tx, uint8\_t \* rx, size\_t len) <br>_Transfer multiple bytes._  |




























## Detailed Description


Implements SPI master functionality using any GPIO pins. Relies on [**syn\_port\_gpio.h**](syn__port__gpio_8h.md) for pin manipulation. 


    
## Public Types Documentation




### enum SYN\_SPIMode 

_SPI Clock Mode._ 
```C++
enum SYN_SPIMode {
    SYN_SPI_MODE_0 = 0,
    SYN_SPI_MODE_1 = 1,
    SYN_SPI_MODE_2 = 2,
    SYN_SPI_MODE_3 = 3
};
```




<hr>
## Public Functions Documentation




### function syn\_soft\_spi\_deselect 

_Deassert chip select (drive inactive)._ 
```C++
void syn_soft_spi_deselect (
    SYN_SoftSPI * spi
) 
```





**Parameters:**


* `spi` Soft SPI instance. 




        

<hr>



### function syn\_soft\_spi\_init 

_Initialize the soft SPI pins._ 
```C++
void syn_soft_spi_init (
    SYN_SoftSPI * spi,
    SYN_GPIO_Pin sck,
    SYN_GPIO_Pin mosi,
    SYN_GPIO_Pin miso,
    SYN_SPIMode mode,
    uint32_t delay_loops
) 
```





**Parameters:**


* `spi` Pointer to SPI instance. 
* `sck` SCK pin. 
* `mosi` MOSI pin. 
* `miso` MISO pin. 
* `mode` SPI Mode (0-3). 
* `delay_loops` Number of iterations for a half-clock delay. 




        

<hr>



### function syn\_soft\_spi\_select 

_Assert chip select (drive active)._ 
```C++
void syn_soft_spi_select (
    SYN_SoftSPI * spi
) 
```





**Parameters:**


* `spi` Soft SPI instance. 




        

<hr>



### function syn\_soft\_spi\_set\_cs 

_Set an optional chip select pin._ 
```C++
void syn_soft_spi_set_cs (
    SYN_SoftSPI * spi,
    SYN_GPIO_Pin cs_pin,
    bool active_low
) 
```



When set, syn\_soft\_spi\_select/deselect can be used to assert/deassert CS.




**Parameters:**


* `spi` SPI instance. 
* `cs_pin` Chip select GPIO pin. 
* `active_low` true if CS is active-low (most common). 




        

<hr>



### function syn\_soft\_spi\_transfer 

_Transfer a single byte (read and write simultaneously)._ 
```C++
uint8_t syn_soft_spi_transfer (
    const SYN_SoftSPI * spi,
    uint8_t data
) 
```





**Parameters:**


* `spi` Pointer to SPI instance. 
* `data` Byte to write out on MOSI. 



**Returns:**

Byte read in from MISO. 





        

<hr>



### function syn\_soft\_spi\_transfer\_bulk 

_Transfer multiple bytes._ 
```C++
void syn_soft_spi_transfer_bulk (
    SYN_SoftSPI * spi,
    const uint8_t * tx,
    uint8_t * rx,
    size_t len
) 
```





**Parameters:**


* `spi` Pointer to SPI instance. 
* `tx` Buffer to transmit (can be NULL if rx-only). 
* `rx` Buffer to receive into (can be NULL if tx-only). 
* `len` Number of bytes to transfer. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_soft_spi.h`

