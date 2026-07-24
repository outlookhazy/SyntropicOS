

# File syn\_dali.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_dali.c**](syn__dali_8c.md)

[Go to the source code of this file](syn__dali_8c_source.md)

_Digital Addressable Lighting Interface (DALI / IEC 62386) Protocol Stack implementation._ 

* `#include "syn_dali.h"`
* `#include "../util/syn_assert.h"`
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
|  [**bool**](syn__defs_8h.md#enum-bool) | [**check\_address\_match**](#function-check_address_match) (const [**SYN\_DALI\_SlaveState**](structSYN__DALI__SlaveState.md) \* slave, const [**SYN\_DALI\_ForwardFrame**](structSYN__DALI__ForwardFrame.md) \* req) <br> |


























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
* `bit_out` Output uint16\_t array (minimum 16 elements for bi-phase pairs). 



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

```C++
static bool check_address_match (
    const SYN_DALI_SlaveState * slave,
    const SYN_DALI_ForwardFrame * req
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_dali.c`

