

# File syn\_canopen.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_canopen.c**](syn__canopen_8c.md)

[Go to the source code of this file](syn__canopen_8c_source.md)

_CANopen DS301 Slave Protocol Engine implementation._ 

* `#include "syntropic/proto/syn_canopen.h"`
* `#include "syntropic/util/syn_pack.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_canopen\_get\_tx**](#function-syn_canopen_get_tx) ([**SYN\_CANOpenNode**](structSYN__CANOpenNode.md) \* node, uint32\_t \* out\_cob\_id, uint8\_t \* out\_data, uint8\_t \* out\_len) <br>_Retrieve pending transmit CAN frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_canopen\_init**](#function-syn_canopen_init) ([**SYN\_CANOpenNode**](structSYN__CANOpenNode.md) \* node, const [**SYN\_CANOpenNodeConfig**](structSYN__CANOpenNodeConfig.md) \* cfg, const [**SYN\_CANOpenODEntry**](structSYN__CANOpenODEntry.md) \* od\_table, size\_t od\_count) <br>_Initialize a CANopen DS301 Node._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_canopen\_od\_read**](#function-syn_canopen_od_read) ([**SYN\_CANOpenNode**](structSYN__CANOpenNode.md) \* node, uint16\_t index, uint8\_t subindex, void \* buf, size\_t buf\_size, size\_t \* out\_len) <br>_Directly read an entry from the Object Dictionary._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_canopen\_od\_write**](#function-syn_canopen_od_write) ([**SYN\_CANOpenNode**](structSYN__CANOpenNode.md) \* node, uint16\_t index, uint8\_t subindex, const void \* data, size\_t len) <br>_Directly write an entry to the Object Dictionary._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_canopen\_process\_rx**](#function-syn_canopen_process_rx) ([**SYN\_CANOpenNode**](structSYN__CANOpenNode.md) \* node, uint32\_t cob\_id, const uint8\_t \* data, uint8\_t len) <br>_Process an incoming CAN frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_canopen\_send\_emcy**](#function-syn_canopen_send_emcy) ([**SYN\_CANOpenNode**](structSYN__CANOpenNode.md) \* node, uint16\_t err\_code, uint8\_t err\_reg) <br>_Transmit an Emergency (EMCY) message._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_canopen\_update**](#function-syn_canopen_update) ([**SYN\_CANOpenNode**](structSYN__CANOpenNode.md) \* node, uint32\_t dt\_ms) <br>_Periodic timer update (call from tick thread or loop)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  const [**SYN\_CANOpenODEntry**](structSYN__CANOpenODEntry.md) \* | [**canopen\_find\_od**](#function-canopen_find_od) (const [**SYN\_CANOpenNode**](structSYN__CANOpenNode.md) \* node, uint16\_t index, uint8\_t subindex) <br>_Helper function to find an Object Dictionary entry._  |
|  void | [**canopen\_queue\_tx**](#function-canopen_queue_tx) ([**SYN\_CANOpenNode**](structSYN__CANOpenNode.md) \* node, uint32\_t cob\_id, const uint8\_t \* data, uint8\_t len) <br>_Helper function to queue a CAN transmit frame._  |
|  void | [**canopen\_send\_sdo\_abort**](#function-canopen_send_sdo_abort) ([**SYN\_CANOpenNode**](structSYN__CANOpenNode.md) \* node, uint16\_t index, uint8\_t subindex, uint32\_t abort\_code) <br>_Send an SDO Abort frame._  |


























## Public Functions Documentation




### function syn\_canopen\_get\_tx 

_Retrieve pending transmit CAN frame._ 
```C++
bool syn_canopen_get_tx (
    SYN_CANOpenNode * node,
    uint32_t * out_cob_id,
    uint8_t * out_data,
    uint8_t * out_len
) 
```





**Parameters:**


* `node` Pointer to node handle. 
* `out_cob_id` Output COB-ID pointer. 
* `out_data` Output 8-byte buffer pointer. 
* `out_len` Output frame length pointer. 



**Returns:**

true if a frame was popped, false if no frame pending. 





        

<hr>



### function syn\_canopen\_init 

_Initialize a CANopen DS301 Node._ 
```C++
SYN_Status syn_canopen_init (
    SYN_CANOpenNode * node,
    const SYN_CANOpenNodeConfig * cfg,
    const SYN_CANOpenODEntry * od_table,
    size_t od_count
) 
```





**Parameters:**


* `node` Pointer to node handle. 
* `cfg` Pointer to node configuration. 
* `od_table` Array of Object Dictionary entries. 
* `od_count` Number of entries in od\_table. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid inputs. 





        

<hr>



### function syn\_canopen\_od\_read 

_Directly read an entry from the Object Dictionary._ 
```C++
SYN_Status syn_canopen_od_read (
    SYN_CANOpenNode * node,
    uint16_t index,
    uint8_t subindex,
    void * buf,
    size_t buf_size,
    size_t * out_len
) 
```





**Parameters:**


* `node` Pointer to node handle. 
* `index` 16-bit OD Index. 
* `subindex` 8-bit OD Subindex. 
* `buf` Output buffer pointer. 
* `buf_size` Maximum buffer capacity. 
* `out_len` Pointer to store read byte length. 



**Returns:**

SYN\_OK on success, error code if not found or unauthorized. 





        

<hr>



### function syn\_canopen\_od\_write 

_Directly write an entry to the Object Dictionary._ 
```C++
SYN_Status syn_canopen_od_write (
    SYN_CANOpenNode * node,
    uint16_t index,
    uint8_t subindex,
    const void * data,
    size_t len
) 
```





**Parameters:**


* `node` Pointer to node handle. 
* `index` 16-bit OD Index. 
* `subindex` 8-bit OD Subindex. 
* `data` Input data pointer. 
* `len` Input data length in bytes. 



**Returns:**

SYN\_OK on success, error code if not found or unauthorized. 





        

<hr>



### function syn\_canopen\_process\_rx 

_Process an incoming CAN frame._ 
```C++
SYN_Status syn_canopen_process_rx (
    SYN_CANOpenNode * node,
    uint32_t cob_id,
    const uint8_t * data,
    uint8_t len
) 
```





**Parameters:**


* `node` Pointer to node handle. 
* `cob_id` Received 11-bit CAN COB-ID. 
* `data` Payload buffer (up to 8 bytes). 
* `len` Payload length. 



**Returns:**

SYN\_OK on success, error code on failure. 





        

<hr>



### function syn\_canopen\_send\_emcy 

_Transmit an Emergency (EMCY) message._ 
```C++
SYN_Status syn_canopen_send_emcy (
    SYN_CANOpenNode * node,
    uint16_t err_code,
    uint8_t err_reg
) 
```





**Parameters:**


* `node` Pointer to node handle. 
* `err_code` 16-bit CANopen Emergency Error Code. 
* `err_reg` 8-bit Error Register value (Index 0x1001). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid parameters. 





        

<hr>



### function syn\_canopen\_update 

_Periodic timer update (call from tick thread or loop)._ 
```C++
SYN_Status syn_canopen_update (
    SYN_CANOpenNode * node,
    uint32_t dt_ms
) 
```





**Parameters:**


* `node` Pointer to node handle. 
* `dt_ms` Elapsed time since last update in milliseconds. 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Public Static Functions Documentation




### function canopen\_find\_od 

_Helper function to find an Object Dictionary entry._ 
```C++
static const SYN_CANOpenODEntry * canopen_find_od (
    const SYN_CANOpenNode * node,
    uint16_t index,
    uint8_t subindex
) 
```





**Parameters:**


* `node` Pointer to node handle. 
* `index` 16-bit OD Index. 
* `subindex` 8-bit OD Subindex. 



**Returns:**

Pointer to entry if found, NULL otherwise. 





        

<hr>



### function canopen\_queue\_tx 

_Helper function to queue a CAN transmit frame._ 
```C++
static void canopen_queue_tx (
    SYN_CANOpenNode * node,
    uint32_t cob_id,
    const uint8_t * data,
    uint8_t len
) 
```





**Parameters:**


* `node` Pointer to node handle. 
* `cob_id` 11-bit CAN COB-ID. 
* `data` Payload data pointer. 
* `len` Payload length. 




        

<hr>



### function canopen\_send\_sdo\_abort 

_Send an SDO Abort frame._ 
```C++
static void canopen_send_sdo_abort (
    SYN_CANOpenNode * node,
    uint16_t index,
    uint8_t subindex,
    uint32_t abort_code
) 
```





**Parameters:**


* `node` Pointer to node handle. 
* `index` 16-bit OD Index. 
* `subindex` 8-bit OD Subindex. 
* `abort_code` 32-bit SDO Abort code. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_canopen.c`

