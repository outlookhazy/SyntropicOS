

# File syn\_canopen.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_canopen.h**](syn__canopen_8h.md)

[Go to the source code of this file](syn__canopen_8h_source.md)

_CANopen DS301 Slave Protocol Engine._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include "../common/syn_defs.h"`
* `#include "../drivers/syn_can.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_CANOpenNode**](structSYN__CANOpenNode.md) <br>_CANopen Node State Handle._  |
| struct | [**SYN\_CANOpenNodeConfig**](structSYN__CANOpenNodeConfig.md) <br>_CANopen Node Configuration._  |
| struct | [**SYN\_CANOpenODEntry**](structSYN__CANOpenODEntry.md) <br>_Object Dictionary Entry Definition._  |
| struct | [**SYN\_CANOpenPDOMap**](structSYN__CANOpenPDOMap.md) <br>_PDO Mapping Configuration._  |






















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



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_CANOPEN\_ACCESS\_RO**](syn__canopen_8h.md#define-syn_canopen_access_ro)  `0x01U`<br>_OD Access Rights._  |
| define  | [**SYN\_CANOPEN\_ACCESS\_RW**](syn__canopen_8h.md#define-syn_canopen_access_rw)  `0x03U`<br> |
| define  | [**SYN\_CANOPEN\_ACCESS\_WO**](syn__canopen_8h.md#define-syn_canopen_access_wo)  `0x02U`<br> |
| define  | [**SYN\_CANOPEN\_NMT\_CMD\_PREOP**](syn__canopen_8h.md#define-syn_canopen_nmt_cmd_preop)  `0x80U`<br> |
| define  | [**SYN\_CANOPEN\_NMT\_CMD\_RESET\_COMM**](syn__canopen_8h.md#define-syn_canopen_nmt_cmd_reset_comm)  `0x82U`<br> |
| define  | [**SYN\_CANOPEN\_NMT\_CMD\_RESET\_NODE**](syn__canopen_8h.md#define-syn_canopen_nmt_cmd_reset_node)  `0x81U`<br> |
| define  | [**SYN\_CANOPEN\_NMT\_CMD\_START**](syn__canopen_8h.md#define-syn_canopen_nmt_cmd_start)  `0x01U`<br>_CANopen NMT Command Specifiers._  |
| define  | [**SYN\_CANOPEN\_NMT\_CMD\_STOP**](syn__canopen_8h.md#define-syn_canopen_nmt_cmd_stop)  `0x02U`<br> |
| define  | [**SYN\_CANOPEN\_NMT\_STATE\_BOOTUP**](syn__canopen_8h.md#define-syn_canopen_nmt_state_bootup)  `0x00U`<br>_CANopen NMT States._  |
| define  | [**SYN\_CANOPEN\_NMT\_STATE\_OPERATIONAL**](syn__canopen_8h.md#define-syn_canopen_nmt_state_operational)  `0x05U`<br> |
| define  | [**SYN\_CANOPEN\_NMT\_STATE\_PREOP**](syn__canopen_8h.md#define-syn_canopen_nmt_state_preop)  `0x7FU`<br> |
| define  | [**SYN\_CANOPEN\_NMT\_STATE\_STOPPED**](syn__canopen_8h.md#define-syn_canopen_nmt_state_stopped)  `0x04U`<br> |
| define  | [**SYN\_CANOPEN\_SDO\_ABORT\_NONE**](syn__canopen_8h.md#define-syn_canopen_sdo_abort_none)  `0x00000000UL`<br>_SDO Abort Codes._  |
| define  | [**SYN\_CANOPEN\_SDO\_ABORT\_NOT\_EXIST**](syn__canopen_8h.md#define-syn_canopen_sdo_abort_not_exist)  `0x06020000UL`<br> |
| define  | [**SYN\_CANOPEN\_SDO\_ABORT\_READ\_ONLY**](syn__canopen_8h.md#define-syn_canopen_sdo_abort_read_only)  `0x06010002UL`<br> |
| define  | [**SYN\_CANOPEN\_SDO\_ABORT\_SUBINDEX\_NOT\_EXIST**](syn__canopen_8h.md#define-syn_canopen_sdo_abort_subindex_not_exist)  `0x06090011UL`<br> |
| define  | [**SYN\_CANOPEN\_SDO\_ABORT\_TOGGLE\_BIT**](syn__canopen_8h.md#define-syn_canopen_sdo_abort_toggle_bit)  `0x05030000UL`<br> |
| define  | [**SYN\_CANOPEN\_SDO\_ABORT\_TYPE\_MISMATCH**](syn__canopen_8h.md#define-syn_canopen_sdo_abort_type_mismatch)  `0x06070010UL`<br> |
| define  | [**SYN\_CANOPEN\_SDO\_ABORT\_UNSUPPORTED\_ACCESS**](syn__canopen_8h.md#define-syn_canopen_sdo_abort_unsupported_access)  `0x06010000UL`<br> |
| define  | [**SYN\_CANOPEN\_SDO\_ABORT\_WRITE\_ONLY**](syn__canopen_8h.md#define-syn_canopen_sdo_abort_write_only)  `0x06010001UL`<br> |
| define  | [**SYN\_CANOPEN\_TYPE\_I16**](syn__canopen_8h.md#define-syn_canopen_type_i16)  `0x03U`<br> |
| define  | [**SYN\_CANOPEN\_TYPE\_I32**](syn__canopen_8h.md#define-syn_canopen_type_i32)  `0x04U`<br> |
| define  | [**SYN\_CANOPEN\_TYPE\_I8**](syn__canopen_8h.md#define-syn_canopen_type_i8)  `0x02U`<br>_OD Data Types._  |
| define  | [**SYN\_CANOPEN\_TYPE\_U16**](syn__canopen_8h.md#define-syn_canopen_type_u16)  `0x06U`<br> |
| define  | [**SYN\_CANOPEN\_TYPE\_U32**](syn__canopen_8h.md#define-syn_canopen_type_u32)  `0x07U`<br> |
| define  | [**SYN\_CANOPEN\_TYPE\_U8**](syn__canopen_8h.md#define-syn_canopen_type_u8)  `0x05U`<br> |

## Detailed Description


Implements a lightweight, non-blocking, zero-malloc CANopen DS301 slave stack including Object Dictionary, SDO server, RPDO/TPDO engine, NMT state machine, Heartbeat producer, and Emergency (EMCY) alarms. 


    
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
## Macro Definition Documentation





### define SYN\_CANOPEN\_ACCESS\_RO 

_OD Access Rights._ 
```C++
#define SYN_CANOPEN_ACCESS_RO `0x01U`
```



Read-Only access 


        

<hr>



### define SYN\_CANOPEN\_ACCESS\_RW 

```C++
#define SYN_CANOPEN_ACCESS_RW `0x03U`
```



Read/Write access 


        

<hr>



### define SYN\_CANOPEN\_ACCESS\_WO 

```C++
#define SYN_CANOPEN_ACCESS_WO `0x02U`
```



Write-Only access 


        

<hr>



### define SYN\_CANOPEN\_NMT\_CMD\_PREOP 

```C++
#define SYN_CANOPEN_NMT_CMD_PREOP `0x80U`
```



Enter Pre-Operational 


        

<hr>



### define SYN\_CANOPEN\_NMT\_CMD\_RESET\_COMM 

```C++
#define SYN_CANOPEN_NMT_CMD_RESET_COMM `0x82U`
```



Reset Communication 


        

<hr>



### define SYN\_CANOPEN\_NMT\_CMD\_RESET\_NODE 

```C++
#define SYN_CANOPEN_NMT_CMD_RESET_NODE `0x81U`
```



Reset Node 


        

<hr>



### define SYN\_CANOPEN\_NMT\_CMD\_START 

_CANopen NMT Command Specifiers._ 
```C++
#define SYN_CANOPEN_NMT_CMD_START `0x01U`
```



Start Remote Node 


        

<hr>



### define SYN\_CANOPEN\_NMT\_CMD\_STOP 

```C++
#define SYN_CANOPEN_NMT_CMD_STOP `0x02U`
```



Stop Remote Node 


        

<hr>



### define SYN\_CANOPEN\_NMT\_STATE\_BOOTUP 

_CANopen NMT States._ 
```C++
#define SYN_CANOPEN_NMT_STATE_BOOTUP `0x00U`
```



Bootup State 


        

<hr>



### define SYN\_CANOPEN\_NMT\_STATE\_OPERATIONAL 

```C++
#define SYN_CANOPEN_NMT_STATE_OPERATIONAL `0x05U`
```



Operational State 


        

<hr>



### define SYN\_CANOPEN\_NMT\_STATE\_PREOP 

```C++
#define SYN_CANOPEN_NMT_STATE_PREOP `0x7FU`
```



Pre-Operational State 


        

<hr>



### define SYN\_CANOPEN\_NMT\_STATE\_STOPPED 

```C++
#define SYN_CANOPEN_NMT_STATE_STOPPED `0x04U`
```



Stopped State 


        

<hr>



### define SYN\_CANOPEN\_SDO\_ABORT\_NONE 

_SDO Abort Codes._ 
```C++
#define SYN_CANOPEN_SDO_ABORT_NONE `0x00000000UL`
```



No error 


        

<hr>



### define SYN\_CANOPEN\_SDO\_ABORT\_NOT\_EXIST 

```C++
#define SYN_CANOPEN_SDO_ABORT_NOT_EXIST `0x06020000UL`
```



Object does not exist in dictionary 


        

<hr>



### define SYN\_CANOPEN\_SDO\_ABORT\_READ\_ONLY 

```C++
#define SYN_CANOPEN_SDO_ABORT_READ_ONLY `0x06010002UL`
```



Attempt to write a read only object 


        

<hr>



### define SYN\_CANOPEN\_SDO\_ABORT\_SUBINDEX\_NOT\_EXIST 

```C++
#define SYN_CANOPEN_SDO_ABORT_SUBINDEX_NOT_EXIST `0x06090011UL`
```



Subindex does not exist 


        

<hr>



### define SYN\_CANOPEN\_SDO\_ABORT\_TOGGLE\_BIT 

```C++
#define SYN_CANOPEN_SDO_ABORT_TOGGLE_BIT `0x05030000UL`
```



Toggle bit not altered 


        

<hr>



### define SYN\_CANOPEN\_SDO\_ABORT\_TYPE\_MISMATCH 

```C++
#define SYN_CANOPEN_SDO_ABORT_TYPE_MISMATCH `0x06070010UL`
```



Data type mismatch, length mismatch 


        

<hr>



### define SYN\_CANOPEN\_SDO\_ABORT\_UNSUPPORTED\_ACCESS 

```C++
#define SYN_CANOPEN_SDO_ABORT_UNSUPPORTED_ACCESS `0x06010000UL`
```



Unsupported access to object 


        

<hr>



### define SYN\_CANOPEN\_SDO\_ABORT\_WRITE\_ONLY 

```C++
#define SYN_CANOPEN_SDO_ABORT_WRITE_ONLY `0x06010001UL`
```



Attempt to read a write only object 


        

<hr>



### define SYN\_CANOPEN\_TYPE\_I16 

```C++
#define SYN_CANOPEN_TYPE_I16 `0x03U`
```



Signed 16-bit integer 


        

<hr>



### define SYN\_CANOPEN\_TYPE\_I32 

```C++
#define SYN_CANOPEN_TYPE_I32 `0x04U`
```



Signed 32-bit integer 


        

<hr>



### define SYN\_CANOPEN\_TYPE\_I8 

_OD Data Types._ 
```C++
#define SYN_CANOPEN_TYPE_I8 `0x02U`
```



Signed 8-bit integer 


        

<hr>



### define SYN\_CANOPEN\_TYPE\_U16 

```C++
#define SYN_CANOPEN_TYPE_U16 `0x06U`
```



Unsigned 16-bit integer 


        

<hr>



### define SYN\_CANOPEN\_TYPE\_U32 

```C++
#define SYN_CANOPEN_TYPE_U32 `0x07U`
```



Unsigned 32-bit integer 


        

<hr>



### define SYN\_CANOPEN\_TYPE\_U8 

```C++
#define SYN_CANOPEN_TYPE_U8 `0x05U`
```



Unsigned 8-bit integer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_canopen.h`

