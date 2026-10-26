

# File syn\_mbus.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_mbus.h**](syn__mbus_8h.md)

[Go to the source code of this file](syn__mbus_8h_source.md)

_M-Bus (Meter-Bus, EN 13757-2 / EN 13757-3) Protocol Engine._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_MBUS\_Decoder**](structSYN__MBUS__Decoder.md) <br>_M-Bus streaming state machine decoder._  |
| struct | [**SYN\_MBUS\_Frame**](structSYN__MBUS__Frame.md) <br>_M-Bus decoded frame structure._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_MBUS\_FrameCallback**](#typedef-syn_mbus_framecallback)  <br>_Callback invoked when a complete, valid M-Bus frame is received._  |
| enum  | [**SYN\_MBUS\_FrameType**](#enum-syn_mbus_frametype)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**syn\_mbus\_calc\_checksum**](#function-syn_mbus_calc_checksum) (const uint8\_t \* data, size\_t len) <br>_Calculate M-Bus 8-bit checksum (modulo-256 sum over specified buffer)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mbus\_decode\_frame**](#function-syn_mbus_decode_frame) (const uint8\_t \* buf, size\_t len, [**SYN\_MBUS\_Frame**](structSYN__MBUS__Frame.md) \* frame) <br>_Decode a raw M-Bus byte buffer into a frame structure._  |
|  void | [**syn\_mbus\_decoder\_feed**](#function-syn_mbus_decoder_feed) ([**SYN\_MBUS\_Decoder**](structSYN__MBUS__Decoder.md) \* dec, uint8\_t byte) <br>_Feed a single byte to the streaming M-Bus decoder._  |
|  void | [**syn\_mbus\_decoder\_init**](#function-syn_mbus_decoder_init) ([**SYN\_MBUS\_Decoder**](structSYN__MBUS__Decoder.md) \* dec, [**SYN\_MBUS\_FrameCallback**](syn__mbus_8h.md#typedef-syn_mbus_framecallback) callback, void \* ctx) <br>_Initialize an M-Bus streaming state machine decoder._  |
|  void | [**syn\_mbus\_decoder\_reset**](#function-syn_mbus_decoder_reset) ([**SYN\_MBUS\_Decoder**](structSYN__MBUS__Decoder.md) \* dec) <br>_Reset the streaming decoder state._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mbus\_encode\_ack**](#function-syn_mbus_encode_ack) (uint8\_t \* buf, size\_t cap, size\_t \* out\_len) <br>_Encode a Single Character ACK frame (0xE5)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mbus\_encode\_control**](#function-syn_mbus_encode_control) (uint8\_t c\_field, uint8\_t a\_field, uint8\_t ci\_field, uint8\_t \* buf, size\_t cap, size\_t \* out\_len) <br>_Encode a Control Frame (9 bytes: 0x68 \| 0x03 \| 0x03 \| 0x68 \| C \| A \| CI \| Checksum \| 0x16)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mbus\_encode\_long**](#function-syn_mbus_encode_long) (uint8\_t c\_field, uint8\_t a\_field, uint8\_t ci\_field, const uint8\_t \* payload, uint8\_t payload\_len, uint8\_t \* buf, size\_t cap, size\_t \* out\_len) <br>_Encode a Long Frame (0x68 \| L \| L \| 0x68 \| C \| A \| CI \| Payload... \| Checksum \| 0x16)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mbus\_encode\_short**](#function-syn_mbus_encode_short) (uint8\_t c\_field, uint8\_t a\_field, uint8\_t \* buf, size\_t cap, size\_t \* out\_len) <br>_Encode a Short Frame (5 bytes: 0x10 \| C \| A \| Checksum \| 0x16)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_MBUS\_ACK\_BYTE**](syn__mbus_8h.md#define-syn_mbus_ack_byte)  `0xE5u`<br> |
| define  | [**SYN\_MBUS\_ADDR\_BROADCAST\_NONE**](syn__mbus_8h.md#define-syn_mbus_addr_broadcast_none)  `0xFFu`<br> |
| define  | [**SYN\_MBUS\_ADDR\_BROADCAST\_REPLY**](syn__mbus_8h.md#define-syn_mbus_addr_broadcast_reply)  `0xFEu`<br> |
| define  | [**SYN\_MBUS\_CI\_RSP\_DATA\_LSB**](syn__mbus_8h.md#define-syn_mbus_ci_rsp_data_lsb)  `0x72u`<br> |
| define  | [**SYN\_MBUS\_CI\_RSP\_DATA\_MSB**](syn__mbus_8h.md#define-syn_mbus_ci_rsp_data_msb)  `0x73u`<br> |
| define  | [**SYN\_MBUS\_CI\_SELECT\_SLAVE**](syn__mbus_8h.md#define-syn_mbus_ci_select_slave)  `0x52u`<br> |
| define  | [**SYN\_MBUS\_CI\_SND\_UD\_LSB**](syn__mbus_8h.md#define-syn_mbus_ci_snd_ud_lsb)  `0x51u`<br> |
| define  | [**SYN\_MBUS\_C\_REQ\_UD1**](syn__mbus_8h.md#define-syn_mbus_c_req_ud1)  `0x5Au`<br> |
| define  | [**SYN\_MBUS\_C\_REQ\_UD2**](syn__mbus_8h.md#define-syn_mbus_c_req_ud2)  `0x5Bu`<br> |
| define  | [**SYN\_MBUS\_C\_REQ\_UD2\_FCB**](syn__mbus_8h.md#define-syn_mbus_c_req_ud2_fcb)  `0x7Bu`<br> |
| define  | [**SYN\_MBUS\_C\_RSP\_UD**](syn__mbus_8h.md#define-syn_mbus_c_rsp_ud)  `0x08u`<br> |
| define  | [**SYN\_MBUS\_C\_SND\_NKE**](syn__mbus_8h.md#define-syn_mbus_c_snd_nke)  `0x40u`<br> |
| define  | [**SYN\_MBUS\_C\_SND\_UD**](syn__mbus_8h.md#define-syn_mbus_c_snd_ud)  `0x53u`<br> |
| define  | [**SYN\_MBUS\_C\_SND\_UD\_FCB**](syn__mbus_8h.md#define-syn_mbus_c_snd_ud_fcb)  `0x73u`<br> |
| define  | [**SYN\_MBUS\_MAX\_FRAME\_LEN**](syn__mbus_8h.md#define-syn_mbus_max_frame_len)  `261u`<br> |
| define  | [**SYN\_MBUS\_MAX\_PAYLOAD**](syn__mbus_8h.md#define-syn_mbus_max_payload)  `252u`<br> |
| define  | [**SYN\_MBUS\_START\_CONTROL**](syn__mbus_8h.md#define-syn_mbus_start_control)  `0x68u`<br> |
| define  | [**SYN\_MBUS\_START\_LONG**](syn__mbus_8h.md#define-syn_mbus_start_long)  `0x68u`<br> |
| define  | [**SYN\_MBUS\_START\_SHORT**](syn__mbus_8h.md#define-syn_mbus_start_short)  `0x10u`<br> |
| define  | [**SYN\_MBUS\_STOP**](syn__mbus_8h.md#define-syn_mbus_stop)  `0x16u`<br> |

## Detailed Description


Implements M-Bus master and slave frame formatting, checksum validation, one-shot encoding/decoding, and streaming byte-by-byte frame reception. Supports all 4 M-Bus EN 13757-2 frame types:
* Single Character ACK (0xE5)
* Short Frame (0x10)
* Control Frame (0x68)
* Long Frame (0x68) 




    
## Public Types Documentation




### typedef SYN\_MBUS\_FrameCallback 

_Callback invoked when a complete, valid M-Bus frame is received._ 
```C++
typedef void(* SYN_MBUS_FrameCallback) (const SYN_MBUS_Frame *frame, void *ctx);
```





**Parameters:**


* `frame` Pointer to decoded frame structure. 
* `ctx` User context. 




        

<hr>



### enum SYN\_MBUS\_FrameType 

```C++
enum SYN_MBUS_FrameType {
    SYN_MBUS_FRAME_TYPE_UNKNOWN = 0,
    SYN_MBUS_FRAME_TYPE_SINGLE_ACK,
    SYN_MBUS_FRAME_TYPE_SHORT,
    SYN_MBUS_FRAME_TYPE_CONTROL,
    SYN_MBUS_FRAME_TYPE_LONG
};
```



M-Bus Frame Format Types Enum. 


        

<hr>
## Public Functions Documentation




### function syn\_mbus\_calc\_checksum 

_Calculate M-Bus 8-bit checksum (modulo-256 sum over specified buffer)._ 
```C++
uint8_t syn_mbus_calc_checksum (
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `data` Pointer to buffer. 
* `len` Length of data in bytes. 



**Returns:**

Calculated checksum byte. 





        

<hr>



### function syn\_mbus\_decode\_frame 

_Decode a raw M-Bus byte buffer into a frame structure._ 
```C++
SYN_Status syn_mbus_decode_frame (
    const uint8_t * buf,
    size_t len,
    SYN_MBUS_Frame * frame
) 
```





**Parameters:**


* `buf` Input byte buffer containing a complete M-Bus frame. 
* `len` Buffer length in bytes. 
* `frame` Destination frame structure. 



**Returns:**

SYN\_OK on success, SYN\_ERR\_INVALID\_CHECKSUM on checksum failure, or error status. 





        

<hr>



### function syn\_mbus\_decoder\_feed 

_Feed a single byte to the streaming M-Bus decoder._ 
```C++
void syn_mbus_decoder_feed (
    SYN_MBUS_Decoder * dec,
    uint8_t byte
) 
```





**Parameters:**


* `dec` Decoder instance. 
* `byte` Byte received from UART/RS232/transceiver. 




        

<hr>



### function syn\_mbus\_decoder\_init 

_Initialize an M-Bus streaming state machine decoder._ 
```C++
void syn_mbus_decoder_init (
    SYN_MBUS_Decoder * dec,
    SYN_MBUS_FrameCallback callback,
    void * ctx
) 
```





**Parameters:**


* `dec` Decoder instance. 
* `callback` Callback called upon successful frame completion. 
* `ctx` User context passed to callback. 




        

<hr>



### function syn\_mbus\_decoder\_reset 

_Reset the streaming decoder state._ 
```C++
void syn_mbus_decoder_reset (
    SYN_MBUS_Decoder * dec
) 
```





**Parameters:**


* `dec` Decoder instance. 




        

<hr>



### function syn\_mbus\_encode\_ack 

_Encode a Single Character ACK frame (0xE5)._ 
```C++
SYN_Status syn_mbus_encode_ack (
    uint8_t * buf,
    size_t cap,
    size_t * out_len
) 
```





**Parameters:**


* `buf` Output buffer. 
* `cap` Buffer capacity (must be &gt;= 1). 
* `out_len` Receives total bytes written (1). 



**Returns:**

SYN\_OK on success, or error status. 





        

<hr>



### function syn\_mbus\_encode\_control 

_Encode a Control Frame (9 bytes: 0x68 \| 0x03 \| 0x03 \| 0x68 \| C \| A \| CI \| Checksum \| 0x16)._ 
```C++
SYN_Status syn_mbus_encode_control (
    uint8_t c_field,
    uint8_t a_field,
    uint8_t ci_field,
    uint8_t * buf,
    size_t cap,
    size_t * out_len
) 
```





**Parameters:**


* `c_field` Control field byte. 
* `a_field` Address field byte. 
* `ci_field` Control Information field byte. 
* `buf` Output buffer. 
* `cap` Buffer capacity (must be &gt;= 9). 
* `out_len` Receives total bytes written (9). 



**Returns:**

SYN\_OK on success, or error status. 





        

<hr>



### function syn\_mbus\_encode\_long 

_Encode a Long Frame (0x68 \| L \| L \| 0x68 \| C \| A \| CI \| Payload... \| Checksum \| 0x16)._ 
```C++
SYN_Status syn_mbus_encode_long (
    uint8_t c_field,
    uint8_t a_field,
    uint8_t ci_field,
    const uint8_t * payload,
    uint8_t payload_len,
    uint8_t * buf,
    size_t cap,
    size_t * out_len
) 
```





**Parameters:**


* `c_field` Control field byte. 
* `a_field` Address field byte. 
* `ci_field` Control Information field byte. 
* `payload` Pointer to payload data buffer. 
* `payload_len` Payload length (max 252 bytes). 
* `buf` Output buffer. 
* `cap` Buffer capacity (must be &gt;= payload\_len + 9). 
* `out_len` Receives total bytes written. 



**Returns:**

SYN\_OK on success, or error status. 





        

<hr>



### function syn\_mbus\_encode\_short 

_Encode a Short Frame (5 bytes: 0x10 \| C \| A \| Checksum \| 0x16)._ 
```C++
SYN_Status syn_mbus_encode_short (
    uint8_t c_field,
    uint8_t a_field,
    uint8_t * buf,
    size_t cap,
    size_t * out_len
) 
```





**Parameters:**


* `c_field` Control field byte. 
* `a_field` Address field byte. 
* `buf` Output buffer. 
* `cap` Buffer capacity (must be &gt;= 5). 
* `out_len` Receives total bytes written (5). 



**Returns:**

SYN\_OK on success, or error status. 





        

<hr>
## Macro Definition Documentation





### define SYN\_MBUS\_ACK\_BYTE 

```C++
#define SYN_MBUS_ACK_BYTE `0xE5u`
```



Single character ACK byte 


        

<hr>



### define SYN\_MBUS\_ADDR\_BROADCAST\_NONE 

```C++
#define SYN_MBUS_ADDR_BROADCAST_NONE `0xFFu`
```



Broadcast without reply 


        

<hr>



### define SYN\_MBUS\_ADDR\_BROADCAST\_REPLY 

```C++
#define SYN_MBUS_ADDR_BROADCAST_REPLY `0xFEu`
```



Broadcast with reply 


        

<hr>



### define SYN\_MBUS\_CI\_RSP\_DATA\_LSB 

```C++
#define SYN_MBUS_CI_RSP_DATA_LSB `0x72u`
```



Response data, 12-byte header (LSB first) 


        

<hr>



### define SYN\_MBUS\_CI\_RSP\_DATA\_MSB 

```C++
#define SYN_MBUS_CI_RSP_DATA_MSB `0x73u`
```



Response data, 12-byte header (MSB first) 


        

<hr>



### define SYN\_MBUS\_CI\_SELECT\_SLAVE 

```C++
#define SYN_MBUS_CI_SELECT_SLAVE `0x52u`
```



Select slave (secondary address) 


        

<hr>



### define SYN\_MBUS\_CI\_SND\_UD\_LSB 

```C++
#define SYN_MBUS_CI_SND_UD_LSB `0x51u`
```



Send data, 12-byte header 


        

<hr>



### define SYN\_MBUS\_C\_REQ\_UD1 

```C++
#define SYN_MBUS_C_REQ_UD1 `0x5Au`
```



Master -&gt; Slave: Request data class 1 (FCB=0) 


        

<hr>



### define SYN\_MBUS\_C\_REQ\_UD2 

```C++
#define SYN_MBUS_C_REQ_UD2 `0x5Bu`
```



Master -&gt; Slave: Request data class 2 (FCB=0) 


        

<hr>



### define SYN\_MBUS\_C\_REQ\_UD2\_FCB 

```C++
#define SYN_MBUS_C_REQ_UD2_FCB `0x7Bu`
```



Master -&gt; Slave: Request data class 2 (FCB=1) 


        

<hr>



### define SYN\_MBUS\_C\_RSP\_UD 

```C++
#define SYN_MBUS_C_RSP_UD `0x08u`
```



Slave -&gt; Master: Response user data 


        

<hr>



### define SYN\_MBUS\_C\_SND\_NKE 

```C++
#define SYN_MBUS_C_SND_NKE `0x40u`
```



Master -&gt; Slave: Link reset (SND\_NKE) 


        

<hr>



### define SYN\_MBUS\_C\_SND\_UD 

```C++
#define SYN_MBUS_C_SND_UD `0x53u`
```



Master -&gt; Slave: Send user data (FCB=0) 


        

<hr>



### define SYN\_MBUS\_C\_SND\_UD\_FCB 

```C++
#define SYN_MBUS_C_SND_UD_FCB `0x73u`
```



Master -&gt; Slave: Send user data (FCB=1) 


        

<hr>



### define SYN\_MBUS\_MAX\_FRAME\_LEN 

```C++
#define SYN_MBUS_MAX_FRAME_LEN `261u`
```



Max total raw frame size (SOF+L+L+SOF+C+A+CI+Data+CHK+EOF) 


        

<hr>



### define SYN\_MBUS\_MAX\_PAYLOAD 

```C++
#define SYN_MBUS_MAX_PAYLOAD `252u`
```



Max user payload size in bytes 


        

<hr>



### define SYN\_MBUS\_START\_CONTROL 

```C++
#define SYN_MBUS_START_CONTROL `0x68u`
```



Start delimiter for Control frame 


        

<hr>



### define SYN\_MBUS\_START\_LONG 

```C++
#define SYN_MBUS_START_LONG `0x68u`
```



Start delimiter for Long frame 


        

<hr>



### define SYN\_MBUS\_START\_SHORT 

```C++
#define SYN_MBUS_START_SHORT `0x10u`
```



Start delimiter for Short frame 


        

<hr>



### define SYN\_MBUS\_STOP 

```C++
#define SYN_MBUS_STOP `0x16u`
```



Stop delimiter for frames 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_mbus.h`

