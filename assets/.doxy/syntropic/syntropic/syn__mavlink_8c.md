

# File syn\_mavlink.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_mavlink.c**](syn__mavlink_8c.md)

[Go to the source code of this file](syn__mavlink_8c_source.md)

_Zero-Heap MAVLink v2 Protocol Encoder & Streaming Decoder Implementation._ 

* `#include "syntropic/proto/syn_mavlink.h"`
* `#include <string.h>`

















## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**syn\_\_mavlink\_8c\_1adc29c2ff13d900c2f185ee95427fb06c**](#enum-syn__mavlink_8c_1adc29c2ff13d900c2f185ee95427fb06c)  <br> |




















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


























## Public Types Documentation




### enum syn\_\_mavlink\_8c\_1adc29c2ff13d900c2f185ee95427fb06c 

```C++
enum syn__mavlink_8c_1adc29c2ff13d900c2f185ee95427fb06c {
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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_mavlink.c`

