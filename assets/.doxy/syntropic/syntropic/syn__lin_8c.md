

# File syn\_lin.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_lin.c**](syn__lin_8c.md)

[Go to the source code of this file](syn__lin_8c_source.md)

_LIN Protocol Engine implementation._ 

* `#include "syn_lin.h"`





































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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lin.c`

