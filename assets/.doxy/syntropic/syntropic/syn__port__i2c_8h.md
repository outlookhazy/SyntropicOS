

# File syn\_port\_i2c.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_i2c.h**](syn__port__i2c_8h.md)

[Go to the source code of this file](syn__port__i2c_8h_source.md)

_I2C port interface — implement these for your platform._ 

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_I2C\_Config**](structSYN__I2C__Config.md) <br>_I2C bus configuration._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_i2c\_deinit**](#function-syn_port_i2c_deinit) (uint8\_t bus) <br>_Deinitialize an I2C bus._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_i2c\_init**](#function-syn_port_i2c_init) (const [**SYN\_I2C\_Config**](structSYN__I2C__Config.md) \* cfg) <br>_Initialize an I2C bus._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_i2c\_read**](#function-syn_port_i2c_read) (uint8\_t bus, uint8\_t addr, uint8\_t \* data, size\_t len) <br>_Read data from an I2C device._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_i2c\_write**](#function-syn_port_i2c_write) (uint8\_t bus, uint8\_t addr, const uint8\_t \* data, size\_t len) <br>_Write data to an I2C device._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_i2c\_write\_read**](#function-syn_port_i2c_write_read) (uint8\_t bus, uint8\_t addr, const uint8\_t \* tx\_data, size\_t tx\_len, uint8\_t \* rx\_data, size\_t rx\_len) <br>_Write then read (register access pattern)._  |




























## Public Functions Documentation




### function syn\_port\_i2c\_deinit 

_Deinitialize an I2C bus._ 
```C++
SYN_Status syn_port_i2c_deinit (
    uint8_t bus
) 
```





**Parameters:**


* `bus` I2C bus index. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_i2c\_init 

_Initialize an I2C bus._ 
```C++
SYN_Status syn_port_i2c_init (
    const SYN_I2C_Config * cfg
) 
```





**Parameters:**


* `cfg` I2C configuration. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_i2c\_read 

_Read data from an I2C device._ 
```C++
SYN_Status syn_port_i2c_read (
    uint8_t bus,
    uint8_t addr,
    uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `bus` I2C bus index. 
* `addr` 7-bit device address. 
* `data` Buffer to receive data. 
* `len` Number of bytes to read. 



**Returns:**

SYN\_OK on ACK, SYN\_ERROR on NACK/timeout. 





        

<hr>



### function syn\_port\_i2c\_write 

_Write data to an I2C device._ 
```C++
SYN_Status syn_port_i2c_write (
    uint8_t bus,
    uint8_t addr,
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `bus` I2C bus index. 
* `addr` 7-bit device address. 
* `data` Data to write. 
* `len` Number of bytes. 



**Returns:**

SYN\_OK on ACK, SYN\_ERROR on NACK/timeout. 





        

<hr>



### function syn\_port\_i2c\_write\_read 

_Write then read (register access pattern)._ 
```C++
SYN_Status syn_port_i2c_write_read (
    uint8_t bus,
    uint8_t addr,
    const uint8_t * tx_data,
    size_t tx_len,
    uint8_t * rx_data,
    size_t rx_len
) 
```



Sends a write (typically a register address) followed by a repeated start and read. This is the most common I2C transaction pattern.




**Parameters:**


* `bus` I2C bus index. 
* `addr` 7-bit device address. 
* `tx_data` Data to write (e.g., register address). 
* `tx_len` Write length. 
* `rx_data` Buffer for read data. 
* `rx_len` Read length. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_i2c.h`

