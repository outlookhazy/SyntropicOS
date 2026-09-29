

# File syn\_msp.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_msp.c**](syn__msp_8c.md)

[Go to the source code of this file](syn__msp_8c_source.md)

_Zero-Heap MSP (MultiWii Serial Protocol v1/v2) Encoder & Decoder Implementation._ 

* `#include "syntropic/proto/syn_msp.h"`
* `#include <string.h>`

















## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**syn\_\_msp\_8c\_1a726ca809ffd3d67ab4b8476646f26635**](#enum-syn__msp_8c_1a726ca809ffd3d67ab4b8476646f26635)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_msp\_encode\_response**](#function-syn_msp_encode_response) (uint8\_t cmd, const uint8\_t \* payload, uint8\_t len, uint8\_t \* buf\_out, size\_t buf\_size, size\_t \* out\_len) <br>_Encode an MSP response frame into raw serial bytes buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_msp\_init**](#function-syn_msp_init) ([**SYN\_MSP\_Parser**](structSYN__MSP__Parser.md) \* parser) <br>_Initialize MSP parser instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_msp\_parse\_byte**](#function-syn_msp_parse_byte) ([**SYN\_MSP\_Parser**](structSYN__MSP__Parser.md) \* parser, uint8\_t byte, [**SYN\_MSP\_Frame**](structSYN__MSP__Frame.md) \* frame) <br>_Process incoming byte from serial stream._  |




























## Public Types Documentation




### enum syn\_\_msp\_8c\_1a726ca809ffd3d67ab4b8476646f26635 

```C++
enum syn__msp_8c_1a726ca809ffd3d67ab4b8476646f26635 {
    STATE_HEADER_START = 0,
    STATE_HEADER_M,
    STATE_HEADER_DIR,
    STATE_SIZE,
    STATE_CMD,
    STATE_PAYLOAD,
    STATE_CHECKSUM
};
```




<hr>
## Public Functions Documentation




### function syn\_msp\_encode\_response 

_Encode an MSP response frame into raw serial bytes buffer._ 
```C++
SYN_Status syn_msp_encode_response (
    uint8_t cmd,
    const uint8_t * payload,
    uint8_t len,
    uint8_t * buf_out,
    size_t buf_size,
    size_t * out_len
) 
```





**Parameters:**


* `cmd` Command ID. 
* `payload` Pointer to payload buffer (or NULL if 0 length). 
* `len` Payload length in bytes. 
* `buf_out` Output buffer (must hold at least `len + 6` bytes). 
* `buf_size` Capacity of buf\_out in bytes. 
* `out_len` Pointer to receive total generated frame length in bytes. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_msp\_init 

_Initialize MSP parser instance._ 
```C++
SYN_Status syn_msp_init (
    SYN_MSP_Parser * parser
) 
```





**Parameters:**


* `parser` Pointer to parser struct. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_msp\_parse\_byte 

_Process incoming byte from serial stream._ 
```C++
SYN_Status syn_msp_parse_byte (
    SYN_MSP_Parser * parser,
    uint8_t byte,
    SYN_MSP_Frame * frame
) 
```





**Parameters:**


* `parser` Pointer to parser struct. 
* `byte` Incoming serial byte. 
* `frame` Pointer to destination frame (populated on complete valid frame). 



**Returns:**

SYN\_OK on complete frame, SYN\_BUSY if byte accepted but frame incomplete, SYN\_ERROR on checksum failure. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_msp.c`

