

# File syn\_lin.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_lin.h**](syn__lin_8h.md)

[Go to the source code of this file](syn__lin_8h_source.md)

_LIN (Local Interconnect Network 2.1 / 2.2a & ISO 17987) Protocol Engine._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_LIN\_Frame**](structSYN__LIN__Frame.md) <br>_LIN Frame representation._  |
| struct | [**SYN\_LIN\_Master**](structSYN__LIN__Master.md) <br>_LIN Master state machine handle._  |
| struct | [**SYN\_LIN\_ScheduleSlot**](structSYN__LIN__ScheduleSlot.md) <br>_Schedule table slot entry for LIN Master._  |
| struct | [**SYN\_LIN\_Slave**](structSYN__LIN__Slave.md) <br>_LIN Slave state machine handle._  |
| struct | [**SYN\_LIN\_SlaveFrame**](structSYN__LIN__SlaveFrame.md) <br>_Configuration for a single frame ID handled by a LIN Slave._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_LIN\_ChecksumMode**](#enum-syn_lin_checksummode)  <br>_LIN Checksum calculation mode._  |
| enum  | [**SYN\_LIN\_SlotDirection**](#enum-syn_lin_slotdirection)  <br>_Direction of data for a Master schedule table slot._  |
| enum  | [**SYN\_LIN\_State**](#enum-syn_lin_state)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**syn\_lin\_calc\_checksum**](#function-syn_lin_calc_checksum) (uint8\_t pid, const uint8\_t \* data, uint8\_t len, [**SYN\_LIN\_ChecksumMode**](syn__lin_8h.md#enum-syn_lin_checksummode) mode) <br>_Compute LIN Checksum (Classic or Enhanced)._  |
|  uint8\_t | [**syn\_lin\_calc\_pid**](#function-syn_lin_calc_pid) (uint8\_t id) <br>_Compute LIN Protected Identifier (PID) from a 6-bit Frame ID._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_lin\_master\_init**](#function-syn_lin_master_init) ([**SYN\_LIN\_Master**](structSYN__LIN__Master.md) \* master, const [**SYN\_LIN\_ScheduleSlot**](structSYN__LIN__ScheduleSlot.md) \* slots, size\_t count) <br>_Initialize LIN Master engine with a schedule table._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_lin\_master\_step**](#function-syn_lin_master_step) ([**SYN\_LIN\_Master**](structSYN__LIN__Master.md) \* master, uint32\_t dt\_ms, const [**SYN\_LIN\_ScheduleSlot**](structSYN__LIN__ScheduleSlot.md) \*\* active\_slot) <br>_Step LIN Master schedule table timer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_lin\_slave\_add\_frame**](#function-syn_lin_slave_add_frame) ([**SYN\_LIN\_Slave**](structSYN__LIN__Slave.md) \* slave, uint8\_t id, uint8\_t len, [**SYN\_LIN\_SlotDirection**](syn__lin_8h.md#enum-syn_lin_slotdirection) dir, [**SYN\_LIN\_ChecksumMode**](syn__lin_8h.md#enum-syn_lin_checksummode) mode) <br>_Register a frame filter on LIN Slave._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_lin\_slave\_init**](#function-syn_lin_slave_init) ([**SYN\_LIN\_Slave**](structSYN__LIN__Slave.md) \* slave, uint8\_t nad) <br>_Initialize LIN Slave engine._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_lin\_slave\_process\_byte**](#function-syn_lin_slave_process_byte) ([**SYN\_LIN\_Slave**](structSYN__LIN__Slave.md) \* slave, uint8\_t byte, [**SYN\_LIN\_Frame**](structSYN__LIN__Frame.md) \* out\_frame) <br>_Process an incoming byte stream byte by byte on LIN Slave._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_lin\_slave\_set\_publish\_data**](#function-syn_lin_slave_set_publish_data) ([**SYN\_LIN\_Slave**](structSYN__LIN__Slave.md) \* slave, uint8\_t id, const uint8\_t \* data, uint8\_t len) <br>_Set published payload data for a specific Frame ID on LIN Slave._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_lin\_verify\_pid**](#function-syn_lin_verify_pid) (uint8\_t pid) <br>_Verify Protected Identifier (PID) parity bits._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_LIN\_DATA\_MAX**](syn__lin_8h.md#define-syn_lin_data_max)  `8`<br> |
| define  | [**SYN\_LIN\_ID\_EXTENDED**](syn__lin_8h.md#define-syn_lin_id_extended)  `0x3E`<br> |
| define  | [**SYN\_LIN\_ID\_MASTER\_REQ**](syn__lin_8h.md#define-syn_lin_id_master_req)  `0x3C`<br> |
| define  | [**SYN\_LIN\_ID\_MAX**](syn__lin_8h.md#define-syn_lin_id_max)  `0x3F`<br> |
| define  | [**SYN\_LIN\_ID\_RESERVED**](syn__lin_8h.md#define-syn_lin_id_reserved)  `0x3F`<br> |
| define  | [**SYN\_LIN\_ID\_SLAVE\_RESP**](syn__lin_8h.md#define-syn_lin_id_slave_resp)  `0x3D`<br> |
| define  | [**SYN\_LIN\_SLAVE\_MAX\_FRAMES**](syn__lin_8h.md#define-syn_lin_slave_max_frames)  `8`<br> |
| define  | [**SYN\_LIN\_SYNC\_BYTE**](syn__lin_8h.md#define-syn_lin_sync_byte)  `0x55`<br> |

## Detailed Description


Provides non-blocking, zero-allocation LIN frame parsing, PID parity logic, classic/enhanced checksum verification, Master schedule table execution, and Slave response publishing/subscription state machines. 


    
## Public Types Documentation




### enum SYN\_LIN\_ChecksumMode 

_LIN Checksum calculation mode._ 
```C++
enum SYN_LIN_ChecksumMode {
    SYN_LIN_CHECKSUM_CLASSIC =
        0,
    SYN_LIN_CHECKSUM_ENHANCED = 1
};
```




<hr>



### enum SYN\_LIN\_SlotDirection 

_Direction of data for a Master schedule table slot._ 
```C++
enum SYN_LIN_SlotDirection {
    SYN_LIN_SLOT_PUBLISH = 0,
    SYN_LIN_SLOT_SUBSCRIBE = 1
};
```




<hr>



### enum SYN\_LIN\_State 

```C++
enum SYN_LIN_State {
    SYN_LIN_STATE_IDLE = 0,
    SYN_LIN_STATE_SYNC,
    SYN_LIN_STATE_PID,
    SYN_LIN_STATE_DATA,
    SYN_LIN_STATE_CHECKSUM
};
```



LIN Bus Receiver State Machine Enum. 


        

<hr>
## Public Functions Documentation




### function syn\_lin\_calc\_checksum 

_Compute LIN Checksum (Classic or Enhanced)._ 
```C++
uint8_t syn_lin_calc_checksum (
    uint8_t pid,
    const uint8_t * data,
    uint8_t len,
    SYN_LIN_ChecksumMode mode
) 
```





**Parameters:**


* `pid` 8-bit Protected Identifier (used only in Enhanced mode). 
* `data` Pointer to payload data bytes. 
* `len` Data length (1..8 bytes). 
* `mode` Classic (data only) or Enhanced (PID + data). 



**Returns:**

Inverted modulo-255 sum byte. 





        

<hr>



### function syn\_lin\_calc\_pid 

_Compute LIN Protected Identifier (PID) from a 6-bit Frame ID._ 
```C++
uint8_t syn_lin_calc_pid (
    uint8_t id
) 
```





**Parameters:**


* `id` 6-bit LIN Frame ID (0..63). 



**Returns:**

8-bit Protected Identifier byte with P0/P1 parity bits set. 





        

<hr>



### function syn\_lin\_master\_init 

_Initialize LIN Master engine with a schedule table._ 
```C++
SYN_Status syn_lin_master_init (
    SYN_LIN_Master * master,
    const SYN_LIN_ScheduleSlot * slots,
    size_t count
) 
```





**Parameters:**


* `master` Pointer to LIN Master handle. 
* `slots` Pointer to array of schedule table slots. 
* `count` Number of slots in table. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid inputs. 





        

<hr>



### function syn\_lin\_master\_step 

_Step LIN Master schedule table timer._ 
```C++
bool syn_lin_master_step (
    SYN_LIN_Master * master,
    uint32_t dt_ms,
    const SYN_LIN_ScheduleSlot ** active_slot
) 
```





**Parameters:**


* `master` Pointer to LIN Master handle. 
* `dt_ms` Elapsed time since last call in milliseconds. 
* `active_slot` Pointer to store currently executed slot reference (can be NULL). 



**Returns:**

True if a new schedule slot trigger occurred. 





        

<hr>



### function syn\_lin\_slave\_add\_frame 

_Register a frame filter on LIN Slave._ 
```C++
SYN_Status syn_lin_slave_add_frame (
    SYN_LIN_Slave * slave,
    uint8_t id,
    uint8_t len,
    SYN_LIN_SlotDirection dir,
    SYN_LIN_ChecksumMode mode
) 
```





**Parameters:**


* `slave` Pointer to LIN Slave handle. 
* `id` Frame ID (0..63). 
* `len` Payload length (1..8 bytes). 
* `dir` SUBSCRIBE (slave receives) or PUBLISH (slave transmits). 
* `mode` Classic or Enhanced checksum mode. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if slave table is full. 





        

<hr>



### function syn\_lin\_slave\_init 

_Initialize LIN Slave engine._ 
```C++
SYN_Status syn_lin_slave_init (
    SYN_LIN_Slave * slave,
    uint8_t nad
) 
```





**Parameters:**


* `slave` Pointer to LIN Slave handle. 
* `nad` Node Address (0..255, 0x7F = wildcard/broadcast). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid inputs. 





        

<hr>



### function syn\_lin\_slave\_process\_byte 

_Process an incoming byte stream byte by byte on LIN Slave._ 
```C++
bool syn_lin_slave_process_byte (
    SYN_LIN_Slave * slave,
    uint8_t byte,
    SYN_LIN_Frame * out_frame
) 
```





**Parameters:**


* `slave` Pointer to LIN Slave handle. 
* `byte` Incoming UART byte. 
* `out_frame` Pointer to output structure populated when a complete frame is received. 



**Returns:**

True if a complete valid frame was processed. 





        

<hr>



### function syn\_lin\_slave\_set\_publish\_data 

_Set published payload data for a specific Frame ID on LIN Slave._ 
```C++
SYN_Status syn_lin_slave_set_publish_data (
    SYN_LIN_Slave * slave,
    uint8_t id,
    const uint8_t * data,
    uint8_t len
) 
```





**Parameters:**


* `slave` Pointer to LIN Slave handle. 
* `id` Frame ID (0..63). 
* `data` Pointer to payload data bytes. 
* `len` Payload data length. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if ID not registered as PUBLISH. 





        

<hr>



### function syn\_lin\_verify\_pid 

_Verify Protected Identifier (PID) parity bits._ 
```C++
bool syn_lin_verify_pid (
    uint8_t pid
) 
```





**Parameters:**


* `pid` 8-bit PID byte. 



**Returns:**

True if P0 and P1 parity bits match the 6-bit Frame ID. 





        

<hr>
## Macro Definition Documentation





### define SYN\_LIN\_DATA\_MAX 

```C++
#define SYN_LIN_DATA_MAX `8`
```



Maximum payload bytes per LIN frame 


        

<hr>



### define SYN\_LIN\_ID\_EXTENDED 

```C++
#define SYN_LIN_ID_EXTENDED `0x3E`
```



LIN Extended Frame ID 


        

<hr>



### define SYN\_LIN\_ID\_MASTER\_REQ 

```C++
#define SYN_LIN_ID_MASTER_REQ `0x3C`
```



LIN Diagnostic Master Request ID 


        

<hr>



### define SYN\_LIN\_ID\_MAX 

```C++
#define SYN_LIN_ID_MAX `0x3F`
```



Maximum 6-bit LIN Frame ID (0..63) 


        

<hr>



### define SYN\_LIN\_ID\_RESERVED 

```C++
#define SYN_LIN_ID_RESERVED `0x3F`
```



LIN Reserved Frame ID 


        

<hr>



### define SYN\_LIN\_ID\_SLAVE\_RESP 

```C++
#define SYN_LIN_ID_SLAVE_RESP `0x3D`
```



LIN Diagnostic Slave Response ID 


        

<hr>



### define SYN\_LIN\_SLAVE\_MAX\_FRAMES 

```C++
#define SYN_LIN_SLAVE_MAX_FRAMES `8`
```



Maximum configured frame slots per LIN Slave 


        

<hr>



### define SYN\_LIN\_SYNC\_BYTE 

```C++
#define SYN_LIN_SYNC_BYTE `0x55`
```



LIN Sync Byte 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lin.h`

