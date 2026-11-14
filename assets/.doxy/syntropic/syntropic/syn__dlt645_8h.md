

# File syn\_dlt645.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_dlt645.h**](syn__dlt645_8h.md)

[Go to the source code of this file](syn__dlt645_8h_source.md)

_DL/T 645 Electricity Meter Communication Protocol (1997 & 2007)._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_DLT645\_Decoder**](structSYN__DLT645__Decoder.md) <br>_Streaming byte-at-a-time decoder state._  |
| struct | [**SYN\_DLT645\_Frame**](structSYN__DLT645__Frame.md) <br>_Decoded DL/T 645 frame representation._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_DLT645\_Cmd**](#enum-syn_dlt645_cmd)  <br> |
| typedef void(\* | [**SYN\_DLT645\_FrameCallback**](#typedef-syn_dlt645_framecallback)  <br>_Frame received callback._  |
| enum  | [**SYN\_DLT645\_Ver**](#enum-syn_dlt645_ver)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**syn\_dlt645\_calc\_checksum**](#function-syn_dlt645_calc_checksum) (const uint8\_t \* buf, size\_t len) <br>_Compute DL/T 645 arithmetic modulo-256 checksum over buffer._  |
|  void | [**syn\_dlt645\_decoder\_feed**](#function-syn_dlt645_decoder_feed) ([**SYN\_DLT645\_Decoder**](structSYN__DLT645__Decoder.md) \* dec, uint8\_t rx\_byte) <br>_Feed a single byte into the streaming decoder._  |
|  void | [**syn\_dlt645\_decoder\_init**](#function-syn_dlt645_decoder_init) ([**SYN\_DLT645\_Decoder**](structSYN__DLT645__Decoder.md) \* dec, [**SYN\_DLT645\_Ver**](syn__dlt645_8h.md#enum-syn_dlt645_ver) version, [**SYN\_DLT645\_FrameCallback**](syn__dlt645_8h.md#typedef-syn_dlt645_framecallback) cb, void \* ctx) <br>_Initialize a streaming DL/T 645 decoder._  |
|  size\_t | [**syn\_dlt645\_encode**](#function-syn_dlt645_encode) (const [**SYN\_DLT645\_Frame**](structSYN__DLT645__Frame.md) \* frame, uint8\_t \* out\_buf, size\_t out\_capacity) <br>_Encode a DL/T 645 frame into a binary transmission buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dlt645\_parse**](#function-syn_dlt645_parse) (const uint8\_t \* in\_buf, size\_t len, [**SYN\_DLT645\_Ver**](syn__dlt645_8h.md#enum-syn_dlt645_ver) version, [**SYN\_DLT645\_Frame**](structSYN__DLT645__Frame.md) \* out\_frame) <br>_Parse a raw byte buffer into a DL/T 645 frame._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_DLT645\_ADDR\_LEN**](syn__dlt645_8h.md#define-syn_dlt645_addr_len)  `6`<br> |
| define  | [**SYN\_DLT645\_EOF**](syn__dlt645_8h.md#define-syn_dlt645_eof)  `0x16`<br> |
| define  | [**SYN\_DLT645\_OFFSET**](syn__dlt645_8h.md#define-syn_dlt645_offset)  `0x33`<br> |
| define  | [**SYN\_DLT645\_PREAMBLE**](syn__dlt645_8h.md#define-syn_dlt645_preamble)  `0xFE`<br> |
| define  | [**SYN\_DLT645\_SOF**](syn__dlt645_8h.md#define-syn_dlt645_sof)  `0x68`<br> |

## Detailed Description


Implements DL/T 645-1997 and DL/T 645-2007 master-slave request/response encoding, decoding, checksum validation, and streaming reception. 


    
## Public Types Documentation




### enum SYN\_DLT645\_Cmd 

```C++
enum SYN_DLT645_Cmd {
    SYN_DLT645_CMD_READ_DATA = 0x11,
    SYN_DLT645_CMD_READ_DATA_RESP = 0x91,
    SYN_DLT645_CMD_WRITE_DATA = 0x14,
    SYN_DLT645_CMD_WRITE_DATA_RESP = 0x94,
    SYN_DLT645_CMD_READ_ADDR = 0x13,
    SYN_DLT645_CMD_READ_ADDR_RESP = 0x93,
    SYN_DLT645_CMD_WRITE_ADDR = 0x15,
    SYN_DLT645_CMD_WRITE_ADDR_RESP = 0x95,
    SYN_DLT645_CMD_CHANGE_BAUD = 0x17,
    SYN_DLT645_CMD_CHANGE_BAUD_RESP = 0x97,
    SYN_DLT645_CMD_ERROR_RESP = 0xD1
};
```



DL/T 645 Control Code Commands Enum. 


        

<hr>



### typedef SYN\_DLT645\_FrameCallback 

_Frame received callback._ 
```C++
typedef void(* SYN_DLT645_FrameCallback) (const SYN_DLT645_Frame *frame, void *ctx);
```




<hr>



### enum SYN\_DLT645\_Ver 

```C++
enum SYN_DLT645_Ver {
    SYN_DLT645_VER_1997 = 0,
    SYN_DLT645_VER_2007 = 1
};
```



DL/T 645 Protocol Version Standard Enum. 


        

<hr>
## Public Functions Documentation




### function syn\_dlt645\_calc\_checksum 

_Compute DL/T 645 arithmetic modulo-256 checksum over buffer._ 
```C++
uint8_t syn_dlt645_calc_checksum (
    const uint8_t * buf,
    size_t len
) 
```





**Parameters:**


* `buf` Buffer starting from SOF (0x68). 
* `len` Length of buffer from SOF to end of data field. 



**Returns:**

Modulo-256 checksum byte. 





        

<hr>



### function syn\_dlt645\_decoder\_feed 

_Feed a single byte into the streaming decoder._ 
```C++
void syn_dlt645_decoder_feed (
    SYN_DLT645_Decoder * dec,
    uint8_t rx_byte
) 
```





**Parameters:**


* `dec` Decoder instance. 
* `rx_byte` Single byte received from UART ISR. 




        

<hr>



### function syn\_dlt645\_decoder\_init 

_Initialize a streaming DL/T 645 decoder._ 
```C++
void syn_dlt645_decoder_init (
    SYN_DLT645_Decoder * dec,
    SYN_DLT645_Ver version,
    SYN_DLT645_FrameCallback cb,
    void * ctx
) 
```





**Parameters:**


* `dec` Decoder instance. 
* `version` Expected protocol version. 
* `cb` Frame received callback. 
* `ctx` User callback context. 




        

<hr>



### function syn\_dlt645\_encode 

_Encode a DL/T 645 frame into a binary transmission buffer._ 
```C++
size_t syn_dlt645_encode (
    const SYN_DLT645_Frame * frame,
    uint8_t * out_buf,
    size_t out_capacity
) 
```





**Parameters:**


* `frame` Input frame data. 
* `out_buf` Destination output buffer. 
* `out_capacity` Output buffer capacity (min 20 bytes). 



**Returns:**

Number of bytes written to out\_buf (0 on error). 





        

<hr>



### function syn\_dlt645\_parse 

_Parse a raw byte buffer into a DL/T 645 frame._ 
```C++
SYN_Status syn_dlt645_parse (
    const uint8_t * in_buf,
    size_t len,
    SYN_DLT645_Ver version,
    SYN_DLT645_Frame * out_frame
) 
```





**Parameters:**


* `in_buf` Input buffer containing the complete frame. 
* `len` Length of input buffer. 
* `version` Protocol version expected (1997 or 2007). 
* `out_frame` Destination frame struct. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on validation failure. 





        

<hr>
## Macro Definition Documentation





### define SYN\_DLT645\_ADDR\_LEN 

```C++
#define SYN_DLT645_ADDR_LEN `6`
```



Meter BCD address length in bytes (6) 


        

<hr>



### define SYN\_DLT645\_EOF 

```C++
#define SYN_DLT645_EOF `0x16`
```



End of frame delimiter byte (0x16) 


        

<hr>



### define SYN\_DLT645\_OFFSET 

```C++
#define SYN_DLT645_OFFSET `0x33`
```



Data byte encoding offset (+0x33) 


        

<hr>



### define SYN\_DLT645\_PREAMBLE 

```C++
#define SYN_DLT645_PREAMBLE `0xFE`
```



Preamble wake-up byte (0xFE) 


        

<hr>



### define SYN\_DLT645\_SOF 

```C++
#define SYN_DLT645_SOF `0x68`
```



Start of frame delimiter byte (0x68) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_dlt645.h`

