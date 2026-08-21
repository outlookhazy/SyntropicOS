

# File syn\_i2c.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_i2c.h**](syn__i2c_8h.md)

[Go to the source code of this file](syn__i2c_8h_source.md)

_Hardware-decoupled I2C driver supporting Master/Slave roles & register access._ 

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_i2c.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_I2C**](structSYN__I2C.md) <br>_I2C driver handle structure._  |
| struct | [**SYN\_I2C\_Config**](structSYN__I2C__Config.md) <br>_I2C driver instance configuration parameters._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_I2C\_Role**](#enum-syn_i2c_role)  <br>_I2C operational mode role._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_i2c\_deinit**](#function-syn_i2c_deinit) ([**SYN\_I2C**](structSYN__I2C.md) \* i2c) <br>_De-initialize an I2C instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_i2c\_init**](#function-syn_i2c_init) ([**SYN\_I2C**](structSYN__I2C.md) \* i2c, const [**SYN\_I2C\_Config**](structSYN__I2C__Config.md) \* cfg) <br>_Initialize an I2C instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_i2c\_read\_reg**](#function-syn_i2c_read_reg) ([**SYN\_I2C**](structSYN__I2C.md) \* i2c, uint16\_t addr, uint8\_t reg, uint8\_t \* val) <br>_Read an 8-bit register from a slave device._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_i2c\_transfer**](#function-syn_i2c_transfer) ([**SYN\_I2C**](structSYN__I2C.md) \* i2c, uint16\_t addr, const uint8\_t \* tx, size\_t tx\_len, uint8\_t \* rx, size\_t rx\_len) <br>_Perform raw buffer transfer (Write, Read, or Write-Read restart)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_i2c\_write\_reg**](#function-syn_i2c_write_reg) ([**SYN\_I2C**](structSYN__I2C.md) \* i2c, uint16\_t addr, uint8\_t reg, uint8\_t val) <br>_Write an 8-bit register on a slave device._  |




























## Public Types Documentation




### enum SYN\_I2C\_Role 

_I2C operational mode role._ 
```C++
enum SYN_I2C_Role {
    SYN_I2C_ROLE_MASTER = 0,
    SYN_I2C_ROLE_SLAVE
};
```




<hr>
## Public Functions Documentation




### function syn\_i2c\_deinit 

_De-initialize an I2C instance._ 
```C++
SYN_Status syn_i2c_deinit (
    SYN_I2C * i2c
) 
```





**Parameters:**


* `i2c` Pointer to [**SYN\_I2C**](structSYN__I2C.md) handle. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_i2c\_init 

_Initialize an I2C instance._ 
```C++
SYN_Status syn_i2c_init (
    SYN_I2C * i2c,
    const SYN_I2C_Config * cfg
) 
```





**Parameters:**


* `i2c` Pointer to [**SYN\_I2C**](structSYN__I2C.md) handle. 
* `cfg` Configuration structure. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_i2c\_read\_reg 

_Read an 8-bit register from a slave device._ 
```C++
SYN_Status syn_i2c_read_reg (
    SYN_I2C * i2c,
    uint16_t addr,
    uint8_t reg,
    uint8_t * val
) 
```





**Parameters:**


* `i2c` Pointer to initialized I2C handle. 
* `addr` 7-bit slave address. 
* `reg` Register index byte. 
* `val` Pointer to store output register byte. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_i2c\_transfer 

_Perform raw buffer transfer (Write, Read, or Write-Read restart)._ 
```C++
SYN_Status syn_i2c_transfer (
    SYN_I2C * i2c,
    uint16_t addr,
    const uint8_t * tx,
    size_t tx_len,
    uint8_t * rx,
    size_t rx_len
) 
```





**Parameters:**


* `i2c` Pointer to initialized I2C handle. 
* `addr` Target 7-bit slave address. 
* `tx` TX data buffer (can be NULL). 
* `tx_len` TX byte count. 
* `rx` RX data buffer (can be NULL). 
* `rx_len` RX byte count. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_i2c\_write\_reg 

_Write an 8-bit register on a slave device._ 
```C++
SYN_Status syn_i2c_write_reg (
    SYN_I2C * i2c,
    uint16_t addr,
    uint8_t reg,
    uint8_t val
) 
```





**Parameters:**


* `i2c` Pointer to initialized I2C handle. 
* `addr` 7-bit slave address. 
* `reg` Register index byte. 
* `val` Byte to write into register. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_i2c.h`

