

# File syn\_msp.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_msp.h**](syn__msp_8h.md)

[Go to the source code of this file](syn__msp_8h_source.md)

_Zero-Heap MSP (MultiWii Serial Protocol v1/v2) Encoder & Decoder._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_MSP\_Frame**](structSYN__MSP__Frame.md) <br> |
| struct | [**SYN\_MSP\_Parser**](structSYN__MSP__Parser.md) <br> |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_MSP\_Cmd**](#enum-syn_msp_cmd)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_msp\_encode\_response**](#function-syn_msp_encode_response) (uint8\_t cmd, const uint8\_t \* payload, uint8\_t len, uint8\_t \* buf\_out, size\_t buf\_size, size\_t \* out\_len) <br>_Encode an MSP response frame into raw serial bytes buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_msp\_init**](#function-syn_msp_init) ([**SYN\_MSP\_Parser**](structSYN__MSP__Parser.md) \* parser) <br>_Initialize MSP parser instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_msp\_parse\_byte**](#function-syn_msp_parse_byte) ([**SYN\_MSP\_Parser**](structSYN__MSP__Parser.md) \* parser, uint8\_t byte, [**SYN\_MSP\_Frame**](structSYN__MSP__Frame.md) \* frame) <br>_Process incoming byte from serial stream._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_MSP\_ERR\_CHAR**](syn__msp_8h.md#define-syn_msp_err_char)  `'!'`<br> |
| define  | [**SYN\_MSP\_HEADER\_CHAR**](syn__msp_8h.md#define-syn_msp_header_char)  `'$'`<br> |
| define  | [**SYN\_MSP\_MAX\_PAYLOAD**](syn__msp_8h.md#define-syn_msp_max_payload)  `64U`<br> |
| define  | [**SYN\_MSP\_REQ\_CHAR**](syn__msp_8h.md#define-syn_msp_req_char)  `'&lt;'`<br> |
| define  | [**SYN\_MSP\_RESP\_CHAR**](syn__msp_8h.md#define-syn_msp_resp_char)  `'&gt;'`<br> |
| define  | [**SYN\_MSP\_V1\_CHAR**](syn__msp_8h.md#define-syn_msp_v1_char)  `'[**M**](syn__x25519_8c.md#function-m)'`<br> |

## Detailed Description


MSP Specifications:
* Frame Format: `$M` + [Direction] + [Size] + [Command] + [Payload...] + [Checksum]
* Direction: `<` (Inbound Request), `>` (Outbound Response), `!` (Error Response).
* Checksum: XOR sum over `Size ^ Command ^ Payload[0] ^ ... ^ Payload[N-1]`.
* Common Commands:
  * 101: `MSP_STATUS` (Cycle time, I2C errors, sensors active, flight mode flags).
  * 102: `MSP_RAW_IMU` (Gyro X/Y/Z, Accel X/Y/Z, Mag X/Y/Z).
  * 108: `MSP_ATTITUDE` (Roll 10x deg, Pitch 10x deg, Yaw deg).
  * 109: `MSP_ALTITUDE` (Estimated altitude in cm & variometer rate).
  * 130: `MSP_BATTERY_STATE` (Cell count, capacity mAh, voltage 0.1V, current 0.01A). 






    
## Public Types Documentation




### enum SYN\_MSP\_Cmd 

```C++
enum SYN_MSP_Cmd {
    SYN_MSP_API_VERSION = 1U,
    SYN_MSP_FC_VARIANT = 2U,
    SYN_MSP_STATUS = 101U,
    SYN_MSP_RAW_IMU = 102U,
    SYN_MSP_ATTITUDE = 108U,
    SYN_MSP_ALTITUDE = 109U,
    SYN_MSP_BATTERY_STATE = 130U
};
```



Common MSP Command IDs. 


        

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
## Macro Definition Documentation





### define SYN\_MSP\_ERR\_CHAR 

```C++
#define SYN_MSP_ERR_CHAR `'!'`
```



MSP direction character for error '!' 


        

<hr>



### define SYN\_MSP\_HEADER\_CHAR 

```C++
#define SYN_MSP_HEADER_CHAR `'$'`
```



MSP preamble start character '$' 


        

<hr>



### define SYN\_MSP\_MAX\_PAYLOAD 

```C++
#define SYN_MSP_MAX_PAYLOAD `64U`
```



Maximum MSP v1 payload byte length 


        

<hr>



### define SYN\_MSP\_REQ\_CHAR 

```C++
#define SYN_MSP_REQ_CHAR `'<'`
```



MSP direction character for request '&lt;' 


        

<hr>



### define SYN\_MSP\_RESP\_CHAR 

```C++
#define SYN_MSP_RESP_CHAR `'>'`
```



MSP direction character for response '&gt;' 


        

<hr>



### define SYN\_MSP\_V1\_CHAR 

```C++
#define SYN_MSP_V1_CHAR `' M '`
```



MSP v1 protocol identifier 'M' 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_msp.h`

