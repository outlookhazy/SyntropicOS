

# File syn\_dlt645.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_dlt645.c**](syn__dlt645_8c.md)

[Go to the source code of this file](syn__dlt645_8c_source.md)

_DL/T 645 Electricity Meter Communication Protocol Implementation._ 

* `#include "syn_dlt645.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**syn\_dlt645\_calc\_checksum**](#function-syn_dlt645_calc_checksum) (const uint8\_t \* buf, size\_t len) <br>_Compute DL/T 645 arithmetic modulo-256 checksum over buffer._  |
|  void | [**syn\_dlt645\_decoder\_feed**](#function-syn_dlt645_decoder_feed) ([**SYN\_DLT645\_Decoder**](structSYN__DLT645__Decoder.md) \* dec, uint8\_t rx\_byte) <br>_Feed a single byte into the streaming decoder._  |
|  void | [**syn\_dlt645\_decoder\_init**](#function-syn_dlt645_decoder_init) ([**SYN\_DLT645\_Decoder**](structSYN__DLT645__Decoder.md) \* dec, [**SYN\_DLT645\_Ver**](syn__dlt645_8h.md#enum-syn_dlt645_ver) version, [**SYN\_DLT645\_FrameCallback**](syn__dlt645_8h.md#typedef-syn_dlt645_framecallback) cb, void \* ctx) <br>_Initialize a streaming DL/T 645 decoder._  |
|  size\_t | [**syn\_dlt645\_encode**](#function-syn_dlt645_encode) (const [**SYN\_DLT645\_Frame**](structSYN__DLT645__Frame.md) \* frame, uint8\_t \* out\_buf, size\_t out\_capacity) <br>_Encode a DL/T 645 frame into a binary transmission buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dlt645\_parse**](#function-syn_dlt645_parse) (const uint8\_t \* in\_buf, size\_t len, [**SYN\_DLT645\_Ver**](syn__dlt645_8h.md#enum-syn_dlt645_ver) version, [**SYN\_DLT645\_Frame**](structSYN__DLT645__Frame.md) \* out\_frame) <br>_Parse a raw byte buffer into a DL/T 645 frame._  |




























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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_dlt645.c`

