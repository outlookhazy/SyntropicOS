

# File syn\_mavlink.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_mavlink.c**](syn__mavlink_8c.md)

[Go to the source code of this file](syn__mavlink_8c_source.md)

_Zero-Heap MAVLink v2 Protocol Encoder & Streaming Decoder Implementation._ 

* `#include "syntropic/proto/syn_mavlink.h"`
* `#include <string.h>`

















## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**syn\_\_mavlink\_8c\_1ad4f4fea28b2f1e278ea0da36a0f5e9b7**](#enum-syn__mavlink_8c_1ad4f4fea28b2f1e278ea0da36a0f5e9b7)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**syn\_mavlink\_crc\_accumulate**](#function-syn_mavlink_crc_accumulate) (uint8\_t byte, uint16\_t crc) <br>_Accumulate single byte into MAVLink X25 CRC._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mavlink\_encode\_msg**](#function-syn_mavlink_encode_msg) (uint8\_t sys\_id, uint8\_t comp\_id, uint8\_t seq, uint32\_t msg\_id, uint8\_t crc\_extra, const uint8\_t \* payload, uint8\_t payload\_len, uint8\_t \* buf\_out, size\_t \* out\_len) <br>_Encode a MAVLink v2 packet into raw byte buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mavlink\_init**](#function-syn_mavlink_init) ([**SYN\_MAVLINK\_Parser**](structSYN__MAVLINK__Parser.md) \* parser) <br>_Initialize MAVLink v2 parser instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mavlink\_parse\_byte**](#function-syn_mavlink_parse_byte) ([**SYN\_MAVLINK\_Parser**](structSYN__MAVLINK__Parser.md) \* parser, uint8\_t byte, [**SYN\_MAVLINK\_Frame**](structSYN__MAVLINK__Frame.md) \* frame) <br>_Process incoming byte from serial stream._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**get\_crc\_extra**](#function-get_crc_extra) (uint32\_t msg\_id) <br>_Get CRC extra byte for specified MAVLink message ID._  |
|  size\_t | [**mavlink\_extract\_tail**](#function-mavlink_extract_tail) (const [**SYN\_MAVLINK\_Parser**](structSYN__MAVLINK__Parser.md) \* parser, uint8\_t crc0, uint8\_t crc1, uint8\_t out) <br>_Reconstruct post-STX frame bytes (header, payload, and CRC) after CRC mismatch._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**mavlink\_finish\_frame**](#function-mavlink_finish_frame) ([**SYN\_MAVLINK\_Parser**](structSYN__MAVLINK__Parser.md) \* parser, uint8\_t crc1, [**SYN\_MAVLINK\_Frame**](structSYN__MAVLINK__Frame.md) \* frame, [**bool**](syn__defs_8h.md#enum-bool) \* crc\_failed) <br>_Verify final MAVLink v2 CRC and populate output frame on match._  |
|  void | [**mavlink\_step\_header**](#function-mavlink_step_header) ([**SYN\_MAVLINK\_Parser**](structSYN__MAVLINK__Parser.md) \* parser, uint8\_t byte) <br>_Accumulate one MAVLink v2 header byte and advance header state machine._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**mavlink\_step\_raw**](#function-mavlink_step_raw) ([**SYN\_MAVLINK\_Parser**](structSYN__MAVLINK__Parser.md) \* parser, uint8\_t byte, [**SYN\_MAVLINK\_Frame**](structSYN__MAVLINK__Frame.md) \* frame, uint8\_t \* crc0\_out, [**bool**](syn__defs_8h.md#enum-bool) \* crc\_failed) <br>_Advance raw MAVLink v2 byte parser by one byte without resynchronization replay._  |


























## Public Types Documentation




### enum syn\_\_mavlink\_8c\_1ad4f4fea28b2f1e278ea0da36a0f5e9b7 

```C++
enum syn__mavlink_8c_1ad4f4fea28b2f1e278ea0da36a0f5e9b7 {
    STATE_STX = 0,
    STATE_LEN,
    STATE_INCOMPAT,
    STATE_COMPAT,
    STATE_SEQ,
    STATE_SYS_ID,
    STATE_COMP_ID,
    STATE_MSG_ID0,
    STATE_MSG_ID1,
    STATE_MSG_ID2,
    STATE_PAYLOAD,
    STATE_CRC0,
    STATE_CRC1
};
```




<hr>
## Public Functions Documentation




### function syn\_mavlink\_crc\_accumulate 

_Accumulate single byte into MAVLink X25 CRC._ 
```C++
uint16_t syn_mavlink_crc_accumulate (
    uint8_t byte,
    uint16_t crc
) 
```





**Parameters:**


* `byte` Input byte to accumulate. 
* `crc` Initial CRC accumulator value. 



**Returns:**

Updated 16-bit CRC value. 





        

<hr>



### function syn\_mavlink\_encode\_msg 

_Encode a MAVLink v2 packet into raw byte buffer._ 
```C++
SYN_Status syn_mavlink_encode_msg (
    uint8_t sys_id,
    uint8_t comp_id,
    uint8_t seq,
    uint32_t msg_id,
    uint8_t crc_extra,
    const uint8_t * payload,
    uint8_t payload_len,
    uint8_t * buf_out,
    size_t * out_len
) 
```





**Parameters:**


* `sys_id` System ID (1..255). 
* `comp_id` Component ID (1..255). 
* `seq` Packet sequence number. 
* `msg_id` Message ID. 
* `crc_extra` CRC Extra byte for the specific message ID. 
* `payload` Pointer to payload data buffer (or NULL if 0 len). 
* `payload_len` Payload byte length. 
* `buf_out` Output byte buffer (must hold at least `payload_len + 12` bytes). 
* `out_len` Pointer to receive final encoded frame length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_mavlink\_init 

_Initialize MAVLink v2 parser instance._ 
```C++
SYN_Status syn_mavlink_init (
    SYN_MAVLINK_Parser * parser
) 
```





**Parameters:**


* `parser` Pointer to parser struct. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_mavlink\_parse\_byte 

_Process incoming byte from serial stream._ 
```C++
SYN_Status syn_mavlink_parse_byte (
    SYN_MAVLINK_Parser * parser,
    uint8_t byte,
    SYN_MAVLINK_Frame * frame
) 
```





**Parameters:**


* `parser` Pointer to parser struct. 
* `byte` Incoming byte. 
* `frame` Destination frame struct (populated on valid complete packet). 



**Returns:**

SYN\_OK on complete valid frame, SYN\_BUSY if processing, SYN\_ERROR on CRC mismatch. 





        

<hr>
## Public Static Functions Documentation




### function get\_crc\_extra 

_Get CRC extra byte for specified MAVLink message ID._ 
```C++
static uint8_t get_crc_extra (
    uint32_t msg_id
) 
```





**Parameters:**


* `msg_id` MAVLink message ID. 



**Returns:**

CRC extra seed byte. 





        

<hr>



### function mavlink\_extract\_tail 

_Reconstruct post-STX frame bytes (header, payload, and CRC) after CRC mismatch._ 
```C++
static size_t mavlink_extract_tail (
    const SYN_MAVLINK_Parser * parser,
    uint8_t crc0,
    uint8_t crc1,
    uint8_t out
) 
```





**Parameters:**


* `parser` MAVLink parser state holding header and payload bytes. 
* `crc0` First received CRC byte. 
* `crc1` Second received CRC byte. 
* `out` Destination buffer (up to 266 bytes) for sliding resynchronization replay. 



**Returns:**

Number of bytes written to `out`. 





        

<hr>



### function mavlink\_finish\_frame 

_Verify final MAVLink v2 CRC and populate output frame on match._ 
```C++
static SYN_Status mavlink_finish_frame (
    SYN_MAVLINK_Parser * parser,
    uint8_t crc1,
    SYN_MAVLINK_Frame * frame,
    bool * crc_failed
) 
```





**Parameters:**


* `parser` MAVLink parser instance. 
* `crc1` High byte of received X.25 CRC. 
* `frame` Optional output frame populated when CRC matches. 
* `crc_failed` Set to true when CRC verification fails. 



**Returns:**

SYN\_OK on valid frame, SYN\_ERROR on CRC mismatch. 





        

<hr>



### function mavlink\_step\_header 

_Accumulate one MAVLink v2 header byte and advance header state machine._ 
```C++
static void mavlink_step_header (
    SYN_MAVLINK_Parser * parser,
    uint8_t byte
) 
```





**Parameters:**


* `parser` MAVLink parser instance. 
* `byte` Incoming header byte. 




        

<hr>



### function mavlink\_step\_raw 

_Advance raw MAVLink v2 byte parser by one byte without resynchronization replay._ 
```C++
static SYN_Status mavlink_step_raw (
    SYN_MAVLINK_Parser * parser,
    uint8_t byte,
    SYN_MAVLINK_Frame * frame,
    uint8_t * crc0_out,
    bool * crc_failed
) 
```





**Parameters:**


* `parser` MAVLink parser instance. 
* `byte` Incoming byte. 
* `frame` Optional output frame populated on valid packet completion. 
* `crc0_out` Receives low CRC byte when completing a candidate frame. 
* `crc_failed` Set to true if a complete candidate frame failed CRC check. 



**Returns:**

SYN\_OK on valid frame, SYN\_BUSY while accumulating, SYN\_ERROR on CRC failure. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_mavlink.c`

