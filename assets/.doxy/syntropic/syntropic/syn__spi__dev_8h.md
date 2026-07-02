

# File syn\_spi\_dev.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_spi\_dev.h**](syn__spi__dev_8h.md)

[Go to the source code of this file](syn__spi__dev_8h_source.md)

_SPI device register helpers — thin layer over port SPI._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_spi.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <string.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_SPIDev**](structSYN__SPIDev.md) <br>_SPI device descriptor — bus, CS, read-bit convention._  |
























## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_spi\_dev\_init**](#function-syn_spi_dev_init) ([**SYN\_SPIDev**](structSYN__SPIDev.md) \* dev, uint8\_t bus, uint8\_t cs, uint8\_t read\_bit) <br>_Initialize an SPI device descriptor._  |
|  uint8\_t | [**syn\_spi\_dev\_read8**](#function-syn_spi_dev_read8) (const [**SYN\_SPIDev**](structSYN__SPIDev.md) \* dev, uint8\_t reg) <br>_Read a single 8-bit register._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_spi\_dev\_read\_burst**](#function-syn_spi_dev_read_burst) (const [**SYN\_SPIDev**](structSYN__SPIDev.md) \* dev, uint8\_t reg, uint8\_t \* buf, size\_t len) <br>_Burst-read multiple registers._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_spi\_dev\_write8**](#function-syn_spi_dev_write8) (const [**SYN\_SPIDev**](structSYN__SPIDev.md) \* dev, uint8\_t reg, uint8\_t val) <br>_Write a single 8-bit register._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_spi\_dev\_write\_burst**](#function-syn_spi_dev_write_burst) (const [**SYN\_SPIDev**](structSYN__SPIDev.md) \* dev, uint8\_t reg, const uint8\_t \* data, size\_t len) <br>_Burst-write multiple registers._  |


























## Detailed Description


Provides register-level access for SPI devices. Handles CS assertion, register address framing, and read/write bit.




**
**


```C++
SYN_SPIDev mpu;
syn_spi_dev_init(&mpu, 0, 0, 0x80);  // bus 0, CS 0, read bit = 0x80

uint8_t who = syn_spi_dev_read8(&mpu, 0x75);  // WHO_AM_I
syn_spi_dev_write8(&mpu, 0x6B, 0x00);         // wake up
```
 





    
## Public Static Functions Documentation




### function syn\_spi\_dev\_init 

_Initialize an SPI device descriptor._ 
```C++
static inline void syn_spi_dev_init (
    SYN_SPIDev * dev,
    uint8_t bus,
    uint8_t cs,
    uint8_t read_bit
) 
```





**Parameters:**


* `dev` Device instance. 
* `bus` SPI bus number. 
* `cs` Chip-select index. 
* `read_bit` Bit OR'd into register address for reads. 




        

<hr>



### function syn\_spi\_dev\_read8 

_Read a single 8-bit register._ 
```C++
static inline uint8_t syn_spi_dev_read8 (
    const SYN_SPIDev * dev,
    uint8_t reg
) 
```





**Parameters:**


* `dev` SPI device. 
* `reg` Register address. 



**Returns:**

Register value. 





        

<hr>



### function syn\_spi\_dev\_read\_burst 

_Burst-read multiple registers._ 
```C++
static inline SYN_Status syn_spi_dev_read_burst (
    const SYN_SPIDev * dev,
    uint8_t reg,
    uint8_t * buf,
    size_t len
) 
```





**Parameters:**


* `dev` SPI device. 
* `reg` Starting register address. 
* `buf` Buffer to read into. 
* `len` Number of bytes to read. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_spi\_dev\_write8 

_Write a single 8-bit register._ 
```C++
static inline SYN_Status syn_spi_dev_write8 (
    const SYN_SPIDev * dev,
    uint8_t reg,
    uint8_t val
) 
```





**Parameters:**


* `dev` SPI device. 
* `reg` Register address. 
* `val` Value to write. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_spi\_dev\_write\_burst 

_Burst-write multiple registers._ 
```C++
static inline SYN_Status syn_spi_dev_write_burst (
    const SYN_SPIDev * dev,
    uint8_t reg,
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `dev` SPI device. 
* `reg` Starting register address. 
* `data` Data to write. 
* `len` Number of bytes. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_spi_dev.h`

