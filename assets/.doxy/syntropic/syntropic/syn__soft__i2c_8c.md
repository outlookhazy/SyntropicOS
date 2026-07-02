

# File syn\_soft\_i2c.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_soft\_i2c.c**](syn__soft__i2c_8c.md)

[Go to the source code of this file](syn__soft__i2c_8c_source.md)



* `#include "syn_soft_i2c.h"`
* `#include "syn_gpio.h"`
* `#include "../util/syn_assert.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_soft\_i2c\_init**](#function-syn_soft_i2c_init) ([**SYN\_SoftI2C**](structSYN__SoftI2C.md) \* i2c, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) scl, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) sda, uint32\_t delay\_loops) <br>_Initialize the soft I2C pins._  |
|  uint8\_t | [**syn\_soft\_i2c\_read**](#function-syn_soft_i2c_read) (const [**SYN\_SoftI2C**](structSYN__SoftI2C.md) \* i2c, [**bool**](syn__defs_8h.md#enum-bool) ack) <br>_Read a byte from the I2C bus._  |
|  void | [**syn\_soft\_i2c\_start**](#function-syn_soft_i2c_start) (const [**SYN\_SoftI2C**](structSYN__SoftI2C.md) \* i2c) <br>_Generate an I2C START condition._  |
|  void | [**syn\_soft\_i2c\_stop**](#function-syn_soft_i2c_stop) (const [**SYN\_SoftI2C**](structSYN__SoftI2C.md) \* i2c) <br>_Generate an I2C STOP condition._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_soft\_i2c\_write**](#function-syn_soft_i2c_write) (const [**SYN\_SoftI2C**](structSYN__SoftI2C.md) \* i2c, uint8\_t data) <br>_Write a byte to the I2C bus._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_soft\_i2c\_write\_read**](#function-syn_soft_i2c_write_read) ([**SYN\_SoftI2C**](structSYN__SoftI2C.md) \* i2c, uint8\_t dev\_addr, const uint8\_t \* tx\_data, size\_t tx\_len, uint8\_t \* rx\_data, size\_t rx\_len) <br>_Perform a write-then-read I2C transaction._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**i2c\_delay**](#function-i2c_delay) (const [**SYN\_SoftI2C**](structSYN__SoftI2C.md) \* i2c) <br> |
|  void | [**scl\_high**](#function-scl_high) (const [**SYN\_SoftI2C**](structSYN__SoftI2C.md) \* i2c) <br> |
|  void | [**scl\_low**](#function-scl_low) (const [**SYN\_SoftI2C**](structSYN__SoftI2C.md) \* i2c) <br> |
|  void | [**sda\_high**](#function-sda_high) (const [**SYN\_SoftI2C**](structSYN__SoftI2C.md) \* i2c) <br> |
|  void | [**sda\_low**](#function-sda_low) (const [**SYN\_SoftI2C**](structSYN__SoftI2C.md) \* i2c) <br> |


























## Public Functions Documentation




### function syn\_soft\_i2c\_init 

_Initialize the soft I2C pins._ 
```C++
void syn_soft_i2c_init (
    SYN_SoftI2C * i2c,
    SYN_GPIO_Pin scl,
    SYN_GPIO_Pin sda,
    uint32_t delay_loops
) 
```





**Parameters:**


* `i2c` Pointer to I2C instance. 
* `scl` SCL pin. 
* `sda` SDA pin. 
* `delay_loops` Number of iterations for a half-clock delay. 




        

<hr>



### function syn\_soft\_i2c\_read 

_Read a byte from the I2C bus._ 
```C++
uint8_t syn_soft_i2c_read (
    const SYN_SoftI2C * i2c,
    bool ack
) 
```





**Parameters:**


* `i2c` Pointer to I2C instance. 
* `ack` true to ACK the byte, false to NACK (end of read). 



**Returns:**

The byte read. 





        

<hr>



### function syn\_soft\_i2c\_start 

_Generate an I2C START condition._ 
```C++
void syn_soft_i2c_start (
    const SYN_SoftI2C * i2c
) 
```





**Parameters:**


* `i2c` Pointer to I2C instance. 




        

<hr>



### function syn\_soft\_i2c\_stop 

_Generate an I2C STOP condition._ 
```C++
void syn_soft_i2c_stop (
    const SYN_SoftI2C * i2c
) 
```





**Parameters:**


* `i2c` Pointer to I2C instance. 




        

<hr>



### function syn\_soft\_i2c\_write 

_Write a byte to the I2C bus._ 
```C++
bool syn_soft_i2c_write (
    const SYN_SoftI2C * i2c,
    uint8_t data
) 
```





**Parameters:**


* `i2c` Pointer to I2C instance. 
* `data` Byte to write. 



**Returns:**

true if the slave ACKed, false if NACKed. 





        

<hr>



### function syn\_soft\_i2c\_write\_read 

_Perform a write-then-read I2C transaction._ 
```C++
bool syn_soft_i2c_write_read (
    SYN_SoftI2C * i2c,
    uint8_t dev_addr,
    const uint8_t * tx_data,
    size_t tx_len,
    uint8_t * rx_data,
    size_t rx_len
) 
```



Sends a START, writes the device address + tx\_data, then issues a repeated START, reads rx\_len bytes into rx\_data, and sends a STOP. This covers the common "write register address, read data" pattern.




**Parameters:**


* `i2c` I2C instance. 
* `dev_addr` 7-bit device address (will be left-shifted internally). 
* `tx_data` Data to write (e.g., register address). Can be NULL if tx\_len is 0. 
* `tx_len` Number of bytes to write. 
* `rx_data` Buffer to read into. Can be NULL if rx\_len is 0. 
* `rx_len` Number of bytes to read. 



**Returns:**

true if all bytes were ACKed, false on NACK. 





        

<hr>
## Public Static Functions Documentation




### function i2c\_delay 

```C++
static void i2c_delay (
    const SYN_SoftI2C * i2c
) 
```




<hr>



### function scl\_high 

```C++
static void scl_high (
    const SYN_SoftI2C * i2c
) 
```




<hr>



### function scl\_low 

```C++
static void scl_low (
    const SYN_SoftI2C * i2c
) 
```




<hr>



### function sda\_high 

```C++
static void sda_high (
    const SYN_SoftI2C * i2c
) 
```




<hr>



### function sda\_low 

```C++
static void sda_low (
    const SYN_SoftI2C * i2c
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_soft_i2c.c`

