

# File syn\_mavlink.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_mavlink.h**](syn__mavlink_8h.md)

[Go to the source code of this file](syn__mavlink_8h_source.md)

_Zero-Heap MAVLink v2 Protocol Encoder & Streaming Decoder._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_MAVLINK\_Frame**](structSYN__MAVLINK__Frame.md) <br> |
| struct | [**SYN\_MAVLINK\_Parser**](structSYN__MAVLINK__Parser.md) <br> |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_MAVLINK\_MsgId**](#enum-syn_mavlink_msgid)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**syn\_mavlink\_crc\_accumulate**](#function-syn_mavlink_crc_accumulate) (uint8\_t byte, uint16\_t crc) <br>_Accumulate single byte into MAVLink X25 CRC._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mavlink\_encode\_msg**](#function-syn_mavlink_encode_msg) (uint8\_t sys\_id, uint8\_t comp\_id, uint8\_t seq, uint32\_t msg\_id, uint8\_t crc\_extra, const uint8\_t \* payload, uint8\_t payload\_len, uint8\_t \* buf\_out, size\_t \* out\_len) <br>_Encode a MAVLink v2 packet into raw byte buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mavlink\_init**](#function-syn_mavlink_init) ([**SYN\_MAVLINK\_Parser**](structSYN__MAVLINK__Parser.md) \* parser) <br>_Initialize MAVLink v2 parser instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mavlink\_parse\_byte**](#function-syn_mavlink_parse_byte) ([**SYN\_MAVLINK\_Parser**](structSYN__MAVLINK__Parser.md) \* parser, uint8\_t byte, [**SYN\_MAVLINK\_Frame**](structSYN__MAVLINK__Frame.md) \* frame) <br>_Process incoming byte from serial stream._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_MAVLINK\_HEADER\_LEN**](syn__mavlink_8h.md#define-syn_mavlink_header_len)  `10U`<br> |
| define  | [**SYN\_MAVLINK\_MAX\_PAYLOAD\_LEN**](syn__mavlink_8h.md#define-syn_mavlink_max_payload_len)  `255U`<br> |
| define  | [**SYN\_MAVLINK\_STX\_V2**](syn__mavlink_8h.md#define-syn_mavlink_stx_v2)  `0xFD`<br> |

## Detailed Description


MAVLink v2 Frame Specification: Header: [0xFD] + [Payload Len 1B] + [Incompat Flags 1B] + [Compat Flags 1B] + [Seq 1B] + [Sys ID 1B] + [Comp ID 1B] + [Msg ID 3B (Little-Endian)] Payload: [Payload N B] Checksum: [16-bit CRC-X25 over (Header\_without\_STX + Payload + CRC\_Extra)] 


    
## Public Types Documentation




### enum SYN\_MAVLINK\_MsgId 

```C++
enum SYN_MAVLINK_MsgId {
    SYN_MAVLINK_MSG_HEARTBEAT = 0U,
    SYN_MAVLINK_MSG_SYS_STATUS = 1U,
    SYN_MAVLINK_MSG_ATTITUDE = 30U,
    SYN_MAVLINK_MSG_GLOBAL_POSITION_INT = 33U,
    SYN_MAVLINK_MSG_VFR_HUD = 74U
};
```



MAVLink Message IDs. 


        

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
## Macro Definition Documentation





### define SYN\_MAVLINK\_HEADER\_LEN 

```C++
#define SYN_MAVLINK_HEADER_LEN `10U`
```



MAVLink v2 header length in bytes 


        

<hr>



### define SYN\_MAVLINK\_MAX\_PAYLOAD\_LEN 

```C++
#define SYN_MAVLINK_MAX_PAYLOAD_LEN `255U`
```



Maximum MAVLink payload length in bytes 


        

<hr>



### define SYN\_MAVLINK\_STX\_V2 

```C++
#define SYN_MAVLINK_STX_V2 `0xFD`
```



MAVLink v2 Start of Frame marker byte (0xFD) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_mavlink.h`

