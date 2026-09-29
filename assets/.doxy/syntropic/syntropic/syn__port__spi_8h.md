

# File syn\_port\_spi.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_spi.h**](syn__port__spi_8h.md)

[Go to the source code of this file](syn__port__spi_8h_source.md)

_Port contract for Serial Peripheral Interface (SPI) hardware._ 

* `#include "../common/syn_defs.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_spi\_deinit**](#function-syn_port_spi_deinit) (uint8\_t spi\_id) <br>_De-initialize an SPI hardware instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_spi\_init**](#function-syn_port_spi_init) (uint8\_t spi\_id, uint32\_t baudrate\_hz, uint8\_t mode, uint8\_t role) <br>_Initialize an SPI hardware instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_spi\_transfer**](#function-syn_port_spi_transfer) (uint8\_t spi\_id, const uint8\_t \* tx, uint8\_t \* rx, size\_t len) <br>_Perform a full-duplex simultaneous SPI transfer._  |




























## Public Functions Documentation




### function syn\_port\_spi\_deinit 

_De-initialize an SPI hardware instance._ 
```C++
SYN_Status syn_port_spi_deinit (
    uint8_t spi_id
) 
```





**Parameters:**


* `spi_id` Instance index. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_spi\_init 

_Initialize an SPI hardware instance._ 
```C++
SYN_Status syn_port_spi_init (
    uint8_t spi_id,
    uint32_t baudrate_hz,
    uint8_t mode,
    uint8_t role
) 
```





**Parameters:**


* `spi_id` Instance index (0 = SPI1, 1 = SPI2). 
* `baudrate_hz` Baudrate frequency in Hz (e.g. 1000000, 18000000). 
* `mode` SPI Mode: 0 (CPOL=0, CPHA=0), 1 (CPOL=0, CPHA=1), 2 (CPOL=1, CPHA=0), 3 (CPOL=1, CPHA=1). 
* `role` 0 = Master, 1 = Slave. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_spi\_transfer 

_Perform a full-duplex simultaneous SPI transfer._ 
```C++
SYN_Status syn_port_spi_transfer (
    uint8_t spi_id,
    const uint8_t * tx,
    uint8_t * rx,
    size_t len
) 
```





**Parameters:**


* `spi_id` Instance index. 
* `tx` TX data buffer (can be NULL, transmits 0xFF dummy bytes). 
* `rx` RX destination buffer (can be NULL). 
* `len` Transfer byte count. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_spi.h`

