

# File syn\_port\_spi.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_spi.h**](syn__port__spi_8h.md)

[Go to the source code of this file](syn__port__spi_8h_source.md)

_SPI port interface — implement these for your platform._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_SPI\_Config**](structSYN__SPI__Config.md) <br>_SPI bus configuration._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_SPI\_Mode**](#enum-syn_spi_mode)  <br>_SPI clock polarity/phase mode._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_spi\_cs\_assert**](#function-syn_port_spi_cs_assert) (uint8\_t bus, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) cs\_pin) <br>_Assert (pull low) an SPI chip-select pin._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_spi\_cs\_deassert**](#function-syn_port_spi_cs_deassert) (uint8\_t bus, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) cs\_pin) <br>_Deassert (release high) an SPI chip-select pin._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_spi\_deinit**](#function-syn_port_spi_deinit) (uint8\_t bus) <br>_Deinitialize an SPI bus._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_spi\_init**](#function-syn_port_spi_init) (const [**SYN\_SPI\_Config**](structSYN__SPI__Config.md) \* cfg) <br>_Initialize an SPI bus._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_spi\_transfer**](#function-syn_port_spi_transfer) (uint8\_t bus, const uint8\_t \* tx\_buf, uint8\_t \* rx\_buf, size\_t len) <br>_Full-duplex SPI transfer._  |




























## Detailed Description


The user provides implementations of these functions to connect SyntropicOS to their MCU's SPI peripheral. 


    
## Public Types Documentation




### enum SYN\_SPI\_Mode 

_SPI clock polarity/phase mode._ 
```C++
enum SYN_SPI_Mode {
    SYN_SPI_MODE_0 = 0,
    SYN_SPI_MODE_1 = 1,
    SYN_SPI_MODE_2 = 2,
    SYN_SPI_MODE_3 = 3
};
```




<hr>
## Public Functions Documentation




### function syn\_port\_spi\_cs\_assert 

_Assert (pull low) an SPI chip-select pin._ 
```C++
SYN_Status syn_port_spi_cs_assert (
    uint8_t bus,
    SYN_GPIO_Pin cs_pin
) 
```





**Parameters:**


* `bus` SPI bus index. 
* `cs_pin` Chip-select GPIO pin. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_spi\_cs\_deassert 

_Deassert (release high) an SPI chip-select pin._ 
```C++
SYN_Status syn_port_spi_cs_deassert (
    uint8_t bus,
    SYN_GPIO_Pin cs_pin
) 
```





**Parameters:**


* `bus` SPI bus index. 
* `cs_pin` Chip-select GPIO pin. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_spi\_deinit 

_Deinitialize an SPI bus._ 
```C++
SYN_Status syn_port_spi_deinit (
    uint8_t bus
) 
```





**Parameters:**


* `bus` SPI bus index. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_spi\_init 

_Initialize an SPI bus._ 
```C++
SYN_Status syn_port_spi_init (
    const SYN_SPI_Config * cfg
) 
```





**Parameters:**


* `cfg` SPI configuration. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_spi\_transfer 

_Full-duplex SPI transfer._ 
```C++
SYN_Status syn_port_spi_transfer (
    uint8_t bus,
    const uint8_t * tx_buf,
    uint8_t * rx_buf,
    size_t len
) 
```



Simultaneously transmits from tx\_buf and receives into rx\_buf. Either buffer can be NULL for half-duplex operation.




**Parameters:**


* `bus` SPI bus index. 
* `tx_buf` Transmit buffer (NULL to send 0x00/0xFF). 
* `rx_buf` Receive buffer (NULL to discard received data). 
* `len` Number of bytes to transfer. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_spi.h`

