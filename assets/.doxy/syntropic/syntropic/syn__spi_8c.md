

# File syn\_spi.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_spi.c**](syn__spi_8c.md)

[Go to the source code of this file](syn__spi_8c_source.md)

_Hardware-decoupled SPI driver implementation._ 

* `#include "../util/syn_assert.h"`
* `#include "syn_spi.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_spi\_deinit**](#function-syn_spi_deinit) ([**SYN\_SPI**](structSYN__SPI.md) \* spi) <br>_De-initialize an SPI instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_spi\_init**](#function-syn_spi_init) ([**SYN\_SPI**](structSYN__SPI.md) \* spi, const [**SYN\_SPI\_Config**](structSYN__SPI__Config.md) \* cfg) <br>_Initialize an SPI instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_spi\_read**](#function-syn_spi_read) ([**SYN\_SPI**](structSYN__SPI.md) \* spi, uint8\_t \* rx, size\_t len) <br>_Read bytes in over SPI (transmits dummy 0xFF bytes)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_spi\_transfer**](#function-syn_spi_transfer) ([**SYN\_SPI**](structSYN__SPI.md) \* spi, const uint8\_t \* tx, uint8\_t \* rx, size\_t len) <br>_Full-duplex simultaneous SPI transmit and receive._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_spi\_write**](#function-syn_spi_write) ([**SYN\_SPI**](structSYN__SPI.md) \* spi, const uint8\_t \* tx, size\_t len) <br>_Write bytes out over SPI (discards incoming RX bytes)._  |




























## Public Functions Documentation




### function syn\_spi\_deinit 

_De-initialize an SPI instance._ 
```C++
SYN_Status syn_spi_deinit (
    SYN_SPI * spi
) 
```





**Parameters:**


* `spi` Pointer to [**SYN\_SPI**](structSYN__SPI.md) handle. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_spi\_init 

_Initialize an SPI instance._ 
```C++
SYN_Status syn_spi_init (
    SYN_SPI * spi,
    const SYN_SPI_Config * cfg
) 
```





**Parameters:**


* `spi` Pointer to [**SYN\_SPI**](structSYN__SPI.md) handle. 
* `cfg` Configuration structure. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_spi\_read 

_Read bytes in over SPI (transmits dummy 0xFF bytes)._ 
```C++
SYN_Status syn_spi_read (
    SYN_SPI * spi,
    uint8_t * rx,
    size_t len
) 
```





**Parameters:**


* `spi` Pointer to initialized SPI handle. 
* `rx` RX destination buffer. 
* `len` RX byte count. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_spi\_transfer 

_Full-duplex simultaneous SPI transmit and receive._ 
```C++
SYN_Status syn_spi_transfer (
    SYN_SPI * spi,
    const uint8_t * tx,
    uint8_t * rx,
    size_t len
) 
```





**Parameters:**


* `spi` Pointer to initialized SPI handle. 
* `tx` TX data buffer (can be NULL, transmits dummy 0xFF). 
* `rx` RX destination buffer (can be NULL). 
* `len` Transfer length in bytes. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_spi\_write 

_Write bytes out over SPI (discards incoming RX bytes)._ 
```C++
SYN_Status syn_spi_write (
    SYN_SPI * spi,
    const uint8_t * tx,
    size_t len
) 
```





**Parameters:**


* `spi` Pointer to initialized SPI handle. 
* `tx` TX data buffer. 
* `len` TX byte count. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_spi.c`

