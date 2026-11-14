

# File syn\_port\_i2c.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_i2c.h**](syn__port__i2c_8h.md)

[Go to the source code of this file](syn__port__i2c_8h_source.md)

_Port contract for Inter-Integrated Circuit (I2C) hardware._ 

* `#include "../common/syn_defs.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_i2c\_deinit**](#function-syn_port_i2c_deinit) (uint8\_t i2c\_id) <br>_De-initialize an I2C hardware instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_i2c\_init**](#function-syn_port_i2c_init) (uint8\_t i2c\_id, uint32\_t clock\_speed\_hz, uint8\_t role, uint16\_t own\_addr) <br>_Initialize an I2C hardware instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_i2c\_transfer**](#function-syn_port_i2c_transfer) (uint8\_t i2c\_id, uint16\_t addr, const uint8\_t \* tx, size\_t tx\_len, uint8\_t \* rx, size\_t rx\_len) <br>_Perform an I2C transaction (Write, Read, or Write-Then-Read restart)._  |




























## Public Functions Documentation




### function syn\_port\_i2c\_deinit 

_De-initialize an I2C hardware instance._ 
```C++
SYN_Status syn_port_i2c_deinit (
    uint8_t i2c_id
) 
```





**Parameters:**


* `i2c_id` Instance index. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_i2c\_init 

_Initialize an I2C hardware instance._ 
```C++
SYN_Status syn_port_i2c_init (
    uint8_t i2c_id,
    uint32_t clock_speed_hz,
    uint8_t role,
    uint16_t own_addr
) 
```





**Parameters:**


* `i2c_id` Instance index (0 = I2C1, 1 = I2C2). 
* `clock_speed_hz` Bus clock frequency in Hz (e.g. 100000, 400000). 
* `role` 0 = Master, 1 = Slave. 
* `own_addr` 7-bit own slave address (used in Slave mode). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_i2c\_transfer 

_Perform an I2C transaction (Write, Read, or Write-Then-Read restart)._ 
```C++
SYN_Status syn_port_i2c_transfer (
    uint8_t i2c_id,
    uint16_t addr,
    const uint8_t * tx,
    size_t tx_len,
    uint8_t * rx,
    size_t rx_len
) 
```





**Parameters:**


* `i2c_id` Instance index. 
* `addr` Target 7-bit slave address. 
* `tx` TX data buffer (can be NULL if rx\_len &gt; 0). 
* `tx_len` Bytes to transmit. 
* `rx` RX destination buffer (can be NULL if tx\_len &gt; 0). 
* `rx_len` Bytes to receive. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_i2c.h`

