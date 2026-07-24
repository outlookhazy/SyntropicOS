

# File syn\_i2c\_dev.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_i2c\_dev.h**](syn__i2c__dev_8h.md)

[Go to the source code of this file](syn__i2c__dev_8h_source.md)

_I2C device register helpers — thin layer over port I2C._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_i2c.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_I2CDev**](structSYN__I2CDev.md) <br>_I2C device descriptor — bus number + 7-bit address._  |
























## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_i2c\_dev\_init**](#function-syn_i2c_dev_init) ([**SYN\_I2CDev**](structSYN__I2CDev.md) \* dev, uint8\_t bus, uint8\_t addr) <br>_Initialize device descriptor._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_i2c\_dev\_modify8**](#function-syn_i2c_dev_modify8) (const [**SYN\_I2CDev**](structSYN__I2CDev.md) \* dev, uint8\_t reg, uint8\_t mask, uint8\_t val) <br>_Modify register: read, mask, set bits, write back._  |
|  uint16\_t | [**syn\_i2c\_dev\_read16\_be**](#function-syn_i2c_dev_read16_be) (const [**SYN\_I2CDev**](structSYN__I2CDev.md) \* dev, uint8\_t reg) <br>_Read a 16-bit register (big-endian, MSB first)._  |
|  uint16\_t | [**syn\_i2c\_dev\_read16\_le**](#function-syn_i2c_dev_read16_le) (const [**SYN\_I2CDev**](structSYN__I2CDev.md) \* dev, uint8\_t reg) <br>_Read a 16-bit register (little-endian, LSB first)._  |
|  uint8\_t | [**syn\_i2c\_dev\_read8**](#function-syn_i2c_dev_read8) (const [**SYN\_I2CDev**](structSYN__I2CDev.md) \* dev, uint8\_t reg) <br>_Read a single 8-bit register._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_i2c\_dev\_read\_burst**](#function-syn_i2c_dev_read_burst) (const [**SYN\_I2CDev**](structSYN__I2CDev.md) \* dev, uint8\_t reg, uint8\_t \* buf, size\_t len) <br>_Burst read: read_ `len` _bytes starting from_`reg` _._ |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_i2c\_dev\_write16\_be**](#function-syn_i2c_dev_write16_be) (const [**SYN\_I2CDev**](structSYN__I2CDev.md) \* dev, uint8\_t reg, uint16\_t val) <br>_Write a 16-bit register (big-endian)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_i2c\_dev\_write8**](#function-syn_i2c_dev_write8) (const [**SYN\_I2CDev**](structSYN__I2CDev.md) \* dev, uint8\_t reg, uint8\_t val) <br>_Write a single 8-bit register._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_i2c\_dev\_write\_burst**](#function-syn_i2c_dev_write_burst) (const [**SYN\_I2CDev**](structSYN__I2CDev.md) \* dev, uint8\_t reg, const uint8\_t \* data, size\_t len) <br>_Burst write: write_ `len` _bytes starting from_`reg` _._ |


























## Detailed Description


Provides convenient register-level access for I2C sensors/peripherals. Eliminates boilerplate for reading/writing 8-bit and 16-bit registers and burst reads.




**
**


```C++
SYN_I2CDev bmp280;
syn_i2c_dev_init(&bmp280, 0, 0x76);  // bus 0, addr 0x76

uint8_t chip_id = syn_i2c_dev_read8(&bmp280, 0xD0);
syn_i2c_dev_write8(&bmp280, 0xF4, 0x27);  // ctrl_meas

uint8_t burst[6];
syn_i2c_dev_read_burst(&bmp280, 0xF7, burst, 6);
```
 





    
## Public Static Functions Documentation




### function syn\_i2c\_dev\_init 

_Initialize device descriptor._ 
```C++
static inline void syn_i2c_dev_init (
    SYN_I2CDev * dev,
    uint8_t bus,
    uint8_t addr
) 
```





**Parameters:**


* `dev` Device instance. 
* `bus` I2C bus number. 
* `addr` 7-bit device address. 




        

<hr>



### function syn\_i2c\_dev\_modify8 

_Modify register: read, mask, set bits, write back._ 
```C++
static inline SYN_Status syn_i2c_dev_modify8 (
    const SYN_I2CDev * dev,
    uint8_t reg,
    uint8_t mask,
    uint8_t val
) 
```





**Parameters:**


* `dev` Device. 
* `reg` Register address. 
* `mask` Bits to modify. 
* `val` New values for masked bits. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_i2c\_dev\_read16\_be 

_Read a 16-bit register (big-endian, MSB first)._ 
```C++
static inline uint16_t syn_i2c_dev_read16_be (
    const SYN_I2CDev * dev,
    uint8_t reg
) 
```





**Parameters:**


* `dev` Device. 
* `reg` Register address. 



**Returns:**

16-bit value (MSB first). 





        

<hr>



### function syn\_i2c\_dev\_read16\_le 

_Read a 16-bit register (little-endian, LSB first)._ 
```C++
static inline uint16_t syn_i2c_dev_read16_le (
    const SYN_I2CDev * dev,
    uint8_t reg
) 
```





**Parameters:**


* `dev` Device. 
* `reg` Register address. 



**Returns:**

16-bit value (LSB first). 





        

<hr>



### function syn\_i2c\_dev\_read8 

_Read a single 8-bit register._ 
```C++
static inline uint8_t syn_i2c_dev_read8 (
    const SYN_I2CDev * dev,
    uint8_t reg
) 
```





**Parameters:**


* `dev` Device. 
* `reg` Register address. 



**Returns:**

Register value. 





        

<hr>



### function syn\_i2c\_dev\_read\_burst 

_Burst read: read_ `len` _bytes starting from_`reg` _._
```C++
static inline SYN_Status syn_i2c_dev_read_burst (
    const SYN_I2CDev * dev,
    uint8_t reg,
    uint8_t * buf,
    size_t len
) 
```





**Parameters:**


* `dev` Device. 
* `reg` Starting register address. 
* `buf` Output buffer. 
* `len` Number of bytes to read. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_i2c\_dev\_write16\_be 

_Write a 16-bit register (big-endian)._ 
```C++
static inline SYN_Status syn_i2c_dev_write16_be (
    const SYN_I2CDev * dev,
    uint8_t reg,
    uint16_t val
) 
```





**Parameters:**


* `dev` Device. 
* `reg` Register address. 
* `val` 16-bit value to write. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_i2c\_dev\_write8 

_Write a single 8-bit register._ 
```C++
static inline SYN_Status syn_i2c_dev_write8 (
    const SYN_I2CDev * dev,
    uint8_t reg,
    uint8_t val
) 
```





**Parameters:**


* `dev` Device. 
* `reg` Register address. 
* `val` Value to write. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_i2c\_dev\_write\_burst 

_Burst write: write_ `len` _bytes starting from_`reg` _._
```C++
static inline SYN_Status syn_i2c_dev_write_burst (
    const SYN_I2CDev * dev,
    uint8_t reg,
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `dev` Device. 
* `reg` Starting register address. 
* `data` Data to write. 
* `len` Number of bytes (max 32). 



**Returns:**

SYN\_OK on success, SYN\_ERROR if len &gt; 32. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_i2c_dev.h`

