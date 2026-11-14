

# File syn\_dali.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_dali.c**](syn__dali_8c.md)

[Go to the source code of this file](syn__dali_8c_source.md)

_Digital Addressable Lighting Interface (DALI / IEC 62386) Protocol Stack implementation._ 

* `#include "../util/syn_assert.h"`
* `#include "syn_dali.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_dali\_decode\_backward**](#function-syn_dali_decode_backward) (uint8\_t raw\_8, [**SYN\_DALI\_BackwardFrame**](structSYN__DALI__BackwardFrame.md) \* frame) <br>_Decode an 8-bit Slave Backward Frame._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_dali\_decode\_forward**](#function-syn_dali_decode_forward) (uint16\_t raw\_16, [**SYN\_DALI\_ForwardFrame**](structSYN__DALI__ForwardFrame.md) \* frame) <br>_Decode a 16-bit Master Forward Frame._  |
|  uint8\_t | [**syn\_dali\_encode\_backward**](#function-syn_dali_encode_backward) (uint8\_t data) <br>_Encode an 8-bit Slave Backward Frame._  |
|  uint16\_t | [**syn\_dali\_encode\_forward**](#function-syn_dali_encode_forward) (uint8\_t addr, uint8\_t data\_cmd) <br>_Encode a 16-bit Master Forward Frame._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_dali\_manchester\_decode\_byte**](#function-syn_dali_manchester_decode_byte) (const uint8\_t \* bit\_in, uint8\_t \* val\_out) <br>_Decode Manchester bi-phase bit pairs into a byte value._  |
|  size\_t | [**syn\_dali\_manchester\_encode\_byte**](#function-syn_dali_manchester_encode_byte) (uint8\_t val, uint8\_t \* bit\_out) <br>_Encode a byte sequence into Manchester bi-phase bits._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dali\_slave\_init**](#function-syn_dali_slave_init) ([**SYN\_DALI\_SlaveState**](structSYN__DALI__SlaveState.md) \* slave, const [**SYN\_DALI\_SlaveConfig**](structSYN__DALI__SlaveConfig.md) \* cfg) <br>_Initialize a DALI Slave Control Gear instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dali\_slave\_process**](#function-syn_dali_slave_process) ([**SYN\_DALI\_SlaveState**](structSYN__DALI__SlaveState.md) \* slave, const [**SYN\_DALI\_ForwardFrame**](structSYN__DALI__ForwardFrame.md) \* req, uint8\_t \* resp\_data, [**bool**](syn__defs_8h.md#enum-bool) \* has\_resp) <br>_Process an incoming DALI Forward Frame on a Slave Control Gear node._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**check\_address\_match**](#function-check_address_match) (const [**SYN\_DALI\_SlaveState**](structSYN__DALI__SlaveState.md) \* slave, const [**SYN\_DALI\_ForwardFrame**](structSYN__DALI__ForwardFrame.md) \* req) <br>_Check if incoming DALI frame address matches slave configuration._  |
|  uint8\_t | [**dali\_clamp\_u8**](#function-dali_clamp_u8) (uint8\_t val, uint8\_t lo, uint8\_t hi) <br>_Clamp an 8-bit level into [lo, hi]._  |
|  uint8\_t | [**dali\_decode\_short\_addr\_arg**](#function-dali_decode_short_addr_arg) (uint8\_t raw) <br>_Decode DALI short-address argument byte (0xFF = unassigned, else bits 6..1)._  |
|  void | [**dali\_exec\_direct\_arc**](#function-dali_exec_direct_arc) ([**SYN\_DALI\_SlaveState**](structSYN__DALI__SlaveState.md) \* slave, uint8\_t target\_level) <br>_Apply a Direct Arc Power Control (DAPC) level command._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dali\_exec\_dtr\_store\_cmd**](#function-dali_exec_dtr_store_cmd) ([**SYN\_DALI\_SlaveState**](structSYN__DALI__SlaveState.md) \* slave, uint8\_t cmd) <br>_Execute DTR0 configuration store and write-enable commands._  |
|  void | [**dali\_exec\_indirect\_cmd**](#function-dali_exec_indirect_cmd) ([**SYN\_DALI\_SlaveState**](structSYN__DALI__SlaveState.md) \* slave, uint8\_t cmd, uint8\_t \* resp\_data, [**bool**](syn__defs_8h.md#enum-bool) \* has\_resp) <br>_Dispatch an addressed indirect arc, configuration, or query command._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dali\_exec\_preset\_cmd**](#function-dali_exec_preset_cmd) ([**SYN\_DALI\_SlaveState**](structSYN__DALI__SlaveState.md) \* slave, uint8\_t cmd) <br>_Execute RECALL\_MAX, RECALL\_MIN, ON\_AND\_STEP\_UP, DAPC, or RESET command._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dali\_exec\_query\_addr\_levels**](#function-dali_exec_query_addr_levels) (const [**SYN\_DALI\_SlaveState**](structSYN__DALI__SlaveState.md) \* slave, uint8\_t cmd, uint8\_t \* resp\_data) <br>_Execute random address and operating level query commands._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dali\_exec\_query\_basic**](#function-dali_exec_query_basic) (const [**SYN\_DALI\_SlaveState**](structSYN__DALI__SlaveState.md) \* slave, uint8\_t cmd, uint8\_t \* resp\_data) <br>_Execute status, DTR content, and group membership query commands._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dali\_exec\_query\_capabilities**](#function-dali_exec_query_capabilities) (const [**SYN\_DALI\_SlaveState**](structSYN__DALI__SlaveState.md) \* slave, uint8\_t cmd, uint8\_t \* resp\_data, [**bool**](syn__defs_8h.md#enum-bool) \* has\_resp) <br>_Execute YES/NO status queries and device capability queries (0x91..0x9A)._  |
|  void | [**dali\_exec\_scene\_group\_cmd**](#function-dali_exec_scene_group_cmd) ([**SYN\_DALI\_SlaveState**](structSYN__DALI__SlaveState.md) \* slave, uint8\_t cmd) <br>_Execute scene recall/configuration and group add/remove commands._  |
|  void | [**dali\_exec\_special\_addr**](#function-dali_exec_special_addr) ([**SYN\_DALI\_SlaveState**](structSYN__DALI__SlaveState.md) \* slave, uint8\_t spec\_code, uint8\_t spec\_data, uint8\_t \* resp\_data, [**bool**](syn__defs_8h.md#enum-bool) \* has\_resp) <br>_Execute compare, withdraw, and short-address programming/verification special commands._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dali\_exec\_special\_search**](#function-dali_exec_special_search) ([**SYN\_DALI\_SlaveState**](structSYN__DALI__SlaveState.md) \* slave, uint8\_t spec\_code, uint8\_t spec\_data) <br>_Execute commissioning termination, DTR load, initialise, randomise, and search commands._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dali\_exec\_step\_cmd**](#function-dali_exec_step_cmd) ([**SYN\_DALI\_SlaveState**](structSYN__DALI__SlaveState.md) \* slave, uint8\_t cmd) <br>_Execute OFF, UP, DOWN, and STEP arc power commands._  |
|  void | [**dali\_reset\_state**](#function-dali_reset_state) ([**SYN\_DALI\_SlaveState**](structSYN__DALI__SlaveState.md) \* slave) <br>_Reset DALI slave operating parameters and registers to IEC 62386 defaults._  |


























## Public Functions Documentation




### function syn\_dali\_decode\_backward 

_Decode an 8-bit Slave Backward Frame._ 
```C++
bool syn_dali_decode_backward (
    uint8_t raw_8,
    SYN_DALI_BackwardFrame * frame
) 
```





**Parameters:**


* `raw_8` Raw 8-bit frame integer. 
* `frame` [out] Decoded frame structure. 



**Returns:**

true on valid frame decoding. 





        

<hr>



### function syn\_dali\_decode\_forward 

_Decode a 16-bit Master Forward Frame._ 
```C++
bool syn_dali_decode_forward (
    uint16_t raw_16,
    SYN_DALI_ForwardFrame * frame
) 
```





**Parameters:**


* `raw_16` Raw 16-bit frame integer. 
* `frame` [out] Decoded frame structure. 



**Returns:**

true on valid frame decoding. 





        

<hr>



### function syn\_dali\_encode\_backward 

_Encode an 8-bit Slave Backward Frame._ 
```C++
uint8_t syn_dali_encode_backward (
    uint8_t data
) 
```





**Parameters:**


* `data` 8-bit response payload. 



**Returns:**

Raw 8-bit frame integer. 





        

<hr>



### function syn\_dali\_encode\_forward 

_Encode a 16-bit Master Forward Frame._ 
```C++
uint16_t syn_dali_encode_forward (
    uint8_t addr,
    uint8_t data_cmd
) 
```





**Parameters:**


* `addr` Address byte (Short, Group, Broadcast). 
* `data_cmd` Data / Command byte. 



**Returns:**

Raw 16-bit frame integer. 





        

<hr>



### function syn\_dali\_manchester\_decode\_byte 

_Decode Manchester bi-phase bit pairs into a byte value._ 
```C++
bool syn_dali_manchester_decode_byte (
    const uint8_t * bit_in,
    uint8_t * val_out
) 
```





**Parameters:**


* `bit_in` Input uint8\_t array containing bi-phase pairs. 
* `val_out` [out] Decoded byte. 



**Returns:**

true on valid Manchester alignment. 





        

<hr>



### function syn\_dali\_manchester\_encode\_byte 

_Encode a byte sequence into Manchester bi-phase bits._ 
```C++
size_t syn_dali_manchester_encode_byte (
    uint8_t val,
    uint8_t * bit_out
) 
```





**Parameters:**


* `val` Input byte value. 
* `bit_out` Output uint8\_t array (minimum 16 elements for bi-phase pairs). 



**Returns:**

Number of bi-phase transitions written. 





        

<hr>



### function syn\_dali\_slave\_init 

_Initialize a DALI Slave Control Gear instance._ 
```C++
SYN_Status syn_dali_slave_init (
    SYN_DALI_SlaveState * slave,
    const SYN_DALI_SlaveConfig * cfg
) 
```





**Parameters:**


* `slave` Slave state handle. 
* `cfg` Slave configuration pointer. 



**Returns:**

SYN\_OK on success, error code otherwise. 





        

<hr>



### function syn\_dali\_slave\_process 

_Process an incoming DALI Forward Frame on a Slave Control Gear node._ 
```C++
SYN_Status syn_dali_slave_process (
    SYN_DALI_SlaveState * slave,
    const SYN_DALI_ForwardFrame * req,
    uint8_t * resp_data,
    bool * has_resp
) 
```





**Parameters:**


* `slave` Slave state handle. 
* `req` Incoming forward frame request. 
* `resp_data` [out] 8-bit response payload (if has\_resp is set to true). 
* `has_resp` [out] Set to true if a backward response frame must be transmitted. 



**Returns:**

SYN\_OK on success, error code on invalid parameters. 





        

<hr>
## Public Static Functions Documentation




### function check\_address\_match 

_Check if incoming DALI frame address matches slave configuration._ 
```C++
static bool check_address_match (
    const SYN_DALI_SlaveState * slave,
    const SYN_DALI_ForwardFrame * req
) 
```





**Parameters:**


* `slave` Pointer to DALI slave state. 
* `req` Pointer to forward frame request. 



**Returns:**

True if address matches, false otherwise. 





        

<hr>



### function dali\_clamp\_u8 

_Clamp an 8-bit level into [lo, hi]._ 
```C++
static inline uint8_t dali_clamp_u8 (
    uint8_t val,
    uint8_t lo,
    uint8_t hi
) 
```





**Parameters:**


* `val` Input value. 
* `lo` Lower bound (inclusive). 
* `hi` Upper bound (inclusive). 



**Returns:**

Clamped 8-bit value. 





        

<hr>



### function dali\_decode\_short\_addr\_arg 

_Decode DALI short-address argument byte (0xFF = unassigned, else bits 6..1)._ 
```C++
static inline uint8_t dali_decode_short_addr_arg (
    uint8_t raw
) 
```





**Parameters:**


* `raw` Encoded short-address byte from DTR0 or PROGRAM\_SHORT\_ADDR. 



**Returns:**

Short address 0..63 or SYN\_DALI\_SHORT\_ADDR\_UNASSIGNED (0xFF). 





        

<hr>



### function dali\_exec\_direct\_arc 

_Apply a Direct Arc Power Control (DAPC) level command._ 
```C++
static void dali_exec_direct_arc (
    SYN_DALI_SlaveState * slave,
    uint8_t target_level
) 
```





**Parameters:**


* `slave` Pointer to DALI slave state. 
* `target_level` Requested 8-bit arc power level. 




        

<hr>



### function dali\_exec\_dtr\_store\_cmd 

_Execute DTR0 configuration store and write-enable commands._ 
```C++
static bool dali_exec_dtr_store_cmd (
    SYN_DALI_SlaveState * slave,
    uint8_t cmd
) 
```





**Parameters:**


* `slave` Pointer to DALI slave state. 
* `cmd` DALI command opcode. 



**Returns:**

true if command was handled, false otherwise. 





        

<hr>



### function dali\_exec\_indirect\_cmd 

_Dispatch an addressed indirect arc, configuration, or query command._ 
```C++
static void dali_exec_indirect_cmd (
    SYN_DALI_SlaveState * slave,
    uint8_t cmd,
    uint8_t * resp_data,
    bool * has_resp
) 
```





**Parameters:**


* `slave` Pointer to DALI slave state. 
* `cmd` DALI command opcode. 
* `resp_data` Output backward frame byte. 
* `has_resp` Set to true if backward frame response is generated. 




        

<hr>



### function dali\_exec\_preset\_cmd 

_Execute RECALL\_MAX, RECALL\_MIN, ON\_AND\_STEP\_UP, DAPC, or RESET command._ 
```C++
static bool dali_exec_preset_cmd (
    SYN_DALI_SlaveState * slave,
    uint8_t cmd
) 
```





**Parameters:**


* `slave` Pointer to DALI slave state. 
* `cmd` DALI command opcode. 



**Returns:**

true if command was handled, false otherwise. 





        

<hr>



### function dali\_exec\_query\_addr\_levels 

_Execute random address and operating level query commands._ 
```C++
static bool dali_exec_query_addr_levels (
    const SYN_DALI_SlaveState * slave,
    uint8_t cmd,
    uint8_t * resp_data
) 
```





**Parameters:**


* `slave` Pointer to DALI slave state. 
* `cmd` DALI query opcode. 
* `resp_data` Output backward frame byte. 



**Returns:**

true if query matched and populated resp\_data, false otherwise. 





        

<hr>



### function dali\_exec\_query\_basic 

_Execute status, DTR content, and group membership query commands._ 
```C++
static bool dali_exec_query_basic (
    const SYN_DALI_SlaveState * slave,
    uint8_t cmd,
    uint8_t * resp_data
) 
```





**Parameters:**


* `slave` Pointer to DALI slave state. 
* `cmd` DALI query opcode. 
* `resp_data` Output backward frame byte. 



**Returns:**

true if query matched and populated resp\_data, false otherwise. 





        

<hr>



### function dali\_exec\_query\_capabilities 

_Execute YES/NO status queries and device capability queries (0x91..0x9A)._ 
```C++
static bool dali_exec_query_capabilities (
    const SYN_DALI_SlaveState * slave,
    uint8_t cmd,
    uint8_t * resp_data,
    bool * has_resp
) 
```





**Parameters:**


* `slave` Pointer to DALI slave state. 
* `cmd` DALI query opcode. 
* `resp_data` Output backward frame byte. 
* `has_resp` Set to true if backward frame response is generated. 



**Returns:**

true if query opcode matched, false otherwise. 





        

<hr>



### function dali\_exec\_scene\_group\_cmd 

_Execute scene recall/configuration and group add/remove commands._ 
```C++
static void dali_exec_scene_group_cmd (
    SYN_DALI_SlaveState * slave,
    uint8_t cmd
) 
```





**Parameters:**


* `slave` Pointer to DALI slave state. 
* `cmd` DALI command opcode. 




        

<hr>



### function dali\_exec\_special\_addr 

_Execute compare, withdraw, and short-address programming/verification special commands._ 
```C++
static void dali_exec_special_addr (
    SYN_DALI_SlaveState * slave,
    uint8_t spec_code,
    uint8_t spec_data,
    uint8_t * resp_data,
    bool * has_resp
) 
```





**Parameters:**


* `slave` Pointer to DALI slave state. 
* `spec_code` Special command address byte. 
* `spec_data` Special command data byte. 
* `resp_data` Output backward frame byte. 
* `has_resp` Set to true if backward frame response is generated. 




        

<hr>



### function dali\_exec\_special\_search 

_Execute commissioning termination, DTR load, initialise, randomise, and search commands._ 
```C++
static bool dali_exec_special_search (
    SYN_DALI_SlaveState * slave,
    uint8_t spec_code,
    uint8_t spec_data
) 
```





**Parameters:**


* `slave` Pointer to DALI slave state. 
* `spec_code` Special command address byte. 
* `spec_data` Special command data byte. 



**Returns:**

true if special command was handled, false otherwise. 





        

<hr>



### function dali\_exec\_step\_cmd 

_Execute OFF, UP, DOWN, and STEP arc power commands._ 
```C++
static bool dali_exec_step_cmd (
    SYN_DALI_SlaveState * slave,
    uint8_t cmd
) 
```





**Parameters:**


* `slave` Pointer to DALI slave state. 
* `cmd` DALI command opcode. 



**Returns:**

true if command was handled, false otherwise. 





        

<hr>



### function dali\_reset\_state 

_Reset DALI slave operating parameters and registers to IEC 62386 defaults._ 
```C++
static void dali_reset_state (
    SYN_DALI_SlaveState * slave
) 
```





**Parameters:**


* `slave` Pointer to DALI slave state. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_dali.c`

