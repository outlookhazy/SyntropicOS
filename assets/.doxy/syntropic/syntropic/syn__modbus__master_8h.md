

# File syn\_modbus\_master.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_modbus\_master.h**](syn__modbus__master_8h.md)

[Go to the source code of this file](syn__modbus__master_8h_source.md)

_Non-blocking Modbus RTU Master / Client protocol engine._ 

* `#include "../common/syn_defs.h"`
* `#include "syn_modbus.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_ModbusMaster**](structSYN__ModbusMaster.md) <br>_Modbus Master instance configuration & transaction state._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_ModbusMaster\_State**](#enum-syn_modbusmaster_state)  <br>_Modbus Master transaction state._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_modbus\_master\_feed**](#function-syn_modbus_master_feed) ([**SYN\_ModbusMaster**](structSYN__ModbusMaster.md) \* m, uint8\_t byte) <br>_Feed a received byte into the Master RX stream buffer._  |
|  void | [**syn\_modbus\_master\_init**](#function-syn_modbus_master_init) ([**SYN\_ModbusMaster**](structSYN__ModbusMaster.md) \* m, uint32\_t timeout\_ms) <br>_Initialize a Modbus Master instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_modbus\_master\_mask\_write\_register**](#function-syn_modbus_master_mask_write_register) ([**SYN\_ModbusMaster**](structSYN__ModbusMaster.md) \* m, uint8\_t slave\_addr, uint16\_t reg\_addr, uint16\_t and\_mask, uint16\_t or\_mask) <br>_Issue a Mask Write Register (FC 0x16) request._  |
|  [**SYN\_ModbusMaster\_State**](syn__modbus__master_8h.md#enum-syn_modbusmaster_state) | [**syn\_modbus\_master\_process**](#function-syn_modbus_master_process) ([**SYN\_ModbusMaster**](structSYN__ModbusMaster.md) \* m, uint32\_t current\_tick\_ms) <br>_Non-blocking state machine processor._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_modbus\_master\_read\_coils**](#function-syn_modbus_master_read_coils) ([**SYN\_ModbusMaster**](structSYN__ModbusMaster.md) \* m, uint8\_t slave\_addr, uint16\_t start\_addr, uint16\_t count) <br>_Issue a Read Coils (FC 0x01) request._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_modbus\_master\_read\_discrete\_inputs**](#function-syn_modbus_master_read_discrete_inputs) ([**SYN\_ModbusMaster**](structSYN__ModbusMaster.md) \* m, uint8\_t slave\_addr, uint16\_t start\_addr, uint16\_t count) <br>_Issue a Read Discrete Inputs (FC 0x02) request._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_modbus\_master\_read\_fifo\_queue**](#function-syn_modbus_master_read_fifo_queue) ([**SYN\_ModbusMaster**](structSYN__ModbusMaster.md) \* m, uint8\_t slave\_addr, uint16\_t fifo\_addr) <br>_Issue a Read FIFO Queue (FC 0x18) request._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_modbus\_master\_read\_holding**](#function-syn_modbus_master_read_holding) ([**SYN\_ModbusMaster**](structSYN__ModbusMaster.md) \* m, uint8\_t slave\_addr, uint16\_t start\_addr, uint16\_t count) <br>_Issue a Read Holding Registers (FC 0x03) request._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_modbus\_master\_read\_input**](#function-syn_modbus_master_read_input) ([**SYN\_ModbusMaster**](structSYN__ModbusMaster.md) \* m, uint8\_t slave\_addr, uint16\_t start\_addr, uint16\_t count) <br>_Issue a Read Input Registers (FC 0x04) request._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_modbus\_master\_report\_server\_id**](#function-syn_modbus_master_report_server_id) ([**SYN\_ModbusMaster**](structSYN__ModbusMaster.md) \* m, uint8\_t slave\_addr) <br>_Issue a Report Server ID (FC 0x11) request._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_modbus\_master\_write\_multiple**](#function-syn_modbus_master_write_multiple) ([**SYN\_ModbusMaster**](structSYN__ModbusMaster.md) \* m, uint8\_t slave\_addr, uint16\_t start\_addr, uint16\_t count, const uint16\_t \* values) <br>_Issue a Write Multiple Registers (FC 0x10) request._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_modbus\_master\_write\_multiple\_coils**](#function-syn_modbus_master_write_multiple_coils) ([**SYN\_ModbusMaster**](structSYN__ModbusMaster.md) \* m, uint8\_t slave\_addr, uint16\_t start\_addr, uint16\_t count, const uint8\_t \* coil\_bytes) <br>_Issue a Write Multiple Coils (FC 0x0F) request._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_modbus\_master\_write\_single**](#function-syn_modbus_master_write_single) ([**SYN\_ModbusMaster**](structSYN__ModbusMaster.md) \* m, uint8\_t slave\_addr, uint16\_t reg\_addr, uint16\_t value) <br>_Issue a Write Single Register (FC 0x06) request._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_modbus\_master\_write\_single\_coil**](#function-syn_modbus_master_write_single_coil) ([**SYN\_ModbusMaster**](structSYN__ModbusMaster.md) \* m, uint8\_t slave\_addr, uint16\_t coil\_addr, [**bool**](syn__defs_8h.md#enum-bool) state) <br>_Issue a Write Single Coil (FC 0x05) request._  |




























## Public Types Documentation




### enum SYN\_ModbusMaster\_State 

_Modbus Master transaction state._ 
```C++
enum SYN_ModbusMaster_State {
    SYN_MB_MASTER_STATE_IDLE = 0,
    SYN_MB_MASTER_STATE_WAITING_RESPONSE,
    SYN_MB_MASTER_STATE_COMPLETE,
    SYN_MB_MASTER_STATE_TIMEOUT,
    SYN_MB_MASTER_STATE_ERROR
};
```




<hr>
## Public Functions Documentation




### function syn\_modbus\_master\_feed 

_Feed a received byte into the Master RX stream buffer._ 
```C++
void syn_modbus_master_feed (
    SYN_ModbusMaster * m,
    uint8_t byte
) 
```





**Parameters:**


* `m` Master instance. 
* `byte` Received byte. 




        

<hr>



### function syn\_modbus\_master\_init 

_Initialize a Modbus Master instance._ 
```C++
void syn_modbus_master_init (
    SYN_ModbusMaster * m,
    uint32_t timeout_ms
) 
```





**Parameters:**


* `m` Master instance. 
* `timeout_ms` Response timeout in milliseconds (e.g. 500 ms). 




        

<hr>



### function syn\_modbus\_master\_mask\_write\_register 

_Issue a Mask Write Register (FC 0x16) request._ 
```C++
SYN_Status syn_modbus_master_mask_write_register (
    SYN_ModbusMaster * m,
    uint8_t slave_addr,
    uint16_t reg_addr,
    uint16_t and_mask,
    uint16_t or_mask
) 
```





**Parameters:**


* `m` Master instance. 
* `slave_addr` Target slave address (1–247). 
* `reg_addr` Register address. 
* `and_mask` AND mask. 
* `or_mask` OR mask. 



**Returns:**

SYN\_OK on success, SYN\_BUSY if transaction in progress. 





        

<hr>



### function syn\_modbus\_master\_process 

_Non-blocking state machine processor._ 
```C++
SYN_ModbusMaster_State syn_modbus_master_process (
    SYN_ModbusMaster * m,
    uint32_t current_tick_ms
) 
```





**Parameters:**


* `m` Master instance. 
* `current_tick_ms` Current system tick in milliseconds. 



**Returns:**

Current Master state. 





        

<hr>



### function syn\_modbus\_master\_read\_coils 

_Issue a Read Coils (FC 0x01) request._ 
```C++
SYN_Status syn_modbus_master_read_coils (
    SYN_ModbusMaster * m,
    uint8_t slave_addr,
    uint16_t start_addr,
    uint16_t count
) 
```





**Parameters:**


* `m` Master instance. 
* `slave_addr` Target slave address (1–247). 
* `start_addr` Starting coil address. 
* `count` Number of coils to read (1–2000). 



**Returns:**

SYN\_OK on success, SYN\_BUSY if transaction in progress. 





        

<hr>



### function syn\_modbus\_master\_read\_discrete\_inputs 

_Issue a Read Discrete Inputs (FC 0x02) request._ 
```C++
SYN_Status syn_modbus_master_read_discrete_inputs (
    SYN_ModbusMaster * m,
    uint8_t slave_addr,
    uint16_t start_addr,
    uint16_t count
) 
```





**Parameters:**


* `m` Master instance. 
* `slave_addr` Target slave address (1–247). 
* `start_addr` Starting input address. 
* `count` Number of inputs to read (1–2000). 



**Returns:**

SYN\_OK on success, SYN\_BUSY if transaction in progress. 





        

<hr>



### function syn\_modbus\_master\_read\_fifo\_queue 

_Issue a Read FIFO Queue (FC 0x18) request._ 
```C++
SYN_Status syn_modbus_master_read_fifo_queue (
    SYN_ModbusMaster * m,
    uint8_t slave_addr,
    uint16_t fifo_addr
) 
```





**Parameters:**


* `m` Master instance. 
* `slave_addr` Target slave address (1–247). 
* `fifo_addr` FIFO register address. 



**Returns:**

SYN\_OK on success, SYN\_BUSY if transaction in progress. 





        

<hr>



### function syn\_modbus\_master\_read\_holding 

_Issue a Read Holding Registers (FC 0x03) request._ 
```C++
SYN_Status syn_modbus_master_read_holding (
    SYN_ModbusMaster * m,
    uint8_t slave_addr,
    uint16_t start_addr,
    uint16_t count
) 
```





**Parameters:**


* `m` Master instance. 
* `slave_addr` Target slave address (1–247). 
* `start_addr` Starting register address. 
* `count` Number of registers to read (1–125). 



**Returns:**

SYN\_OK on success, SYN\_BUSY if transaction in progress. 





        

<hr>



### function syn\_modbus\_master\_read\_input 

_Issue a Read Input Registers (FC 0x04) request._ 
```C++
SYN_Status syn_modbus_master_read_input (
    SYN_ModbusMaster * m,
    uint8_t slave_addr,
    uint16_t start_addr,
    uint16_t count
) 
```





**Parameters:**


* `m` Master instance. 
* `slave_addr` Target slave address (1–247). 
* `start_addr` Starting register address. 
* `count` Number of registers to read (1–125). 



**Returns:**

SYN\_OK on success, SYN\_BUSY if transaction in progress. 





        

<hr>



### function syn\_modbus\_master\_report\_server\_id 

_Issue a Report Server ID (FC 0x11) request._ 
```C++
SYN_Status syn_modbus_master_report_server_id (
    SYN_ModbusMaster * m,
    uint8_t slave_addr
) 
```





**Parameters:**


* `m` Master instance. 
* `slave_addr` Target slave address (1–247). 



**Returns:**

SYN\_OK on success, SYN\_BUSY if transaction in progress. 





        

<hr>



### function syn\_modbus\_master\_write\_multiple 

_Issue a Write Multiple Registers (FC 0x10) request._ 
```C++
SYN_Status syn_modbus_master_write_multiple (
    SYN_ModbusMaster * m,
    uint8_t slave_addr,
    uint16_t start_addr,
    uint16_t count,
    const uint16_t * values
) 
```





**Parameters:**


* `m` Master instance. 
* `slave_addr` Target slave address (1–247). 
* `start_addr` Starting register address. 
* `count` Number of registers (1–121). 
* `values` Array of register values to write. 



**Returns:**

SYN\_OK on success, SYN\_BUSY if transaction in progress. 





        

<hr>



### function syn\_modbus\_master\_write\_multiple\_coils 

_Issue a Write Multiple Coils (FC 0x0F) request._ 
```C++
SYN_Status syn_modbus_master_write_multiple_coils (
    SYN_ModbusMaster * m,
    uint8_t slave_addr,
    uint16_t start_addr,
    uint16_t count,
    const uint8_t * coil_bytes
) 
```





**Parameters:**


* `m` Master instance. 
* `slave_addr` Target slave address (1–247). 
* `start_addr` Starting coil address. 
* `count` Number of coils (1–1968). 
* `coil_bytes` Packed byte array containing coil states. 



**Returns:**

SYN\_OK on success, SYN\_BUSY if transaction in progress. 





        

<hr>



### function syn\_modbus\_master\_write\_single 

_Issue a Write Single Register (FC 0x06) request._ 
```C++
SYN_Status syn_modbus_master_write_single (
    SYN_ModbusMaster * m,
    uint8_t slave_addr,
    uint16_t reg_addr,
    uint16_t value
) 
```





**Parameters:**


* `m` Master instance. 
* `slave_addr` Target slave address (1–247). 
* `reg_addr` Register address. 
* `value` Value to write. 



**Returns:**

SYN\_OK on success, SYN\_BUSY if transaction in progress. 





        

<hr>



### function syn\_modbus\_master\_write\_single\_coil 

_Issue a Write Single Coil (FC 0x05) request._ 
```C++
SYN_Status syn_modbus_master_write_single_coil (
    SYN_ModbusMaster * m,
    uint8_t slave_addr,
    uint16_t coil_addr,
    bool state
) 
```





**Parameters:**


* `m` Master instance. 
* `slave_addr` Target slave address (1–247). 
* `coil_addr` Coil address. 
* `state` Coil state (true = ON / 0xFF00, false = OFF / 0x0000). 



**Returns:**

SYN\_OK on success, SYN\_BUSY if transaction in progress. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_modbus_master.h`

