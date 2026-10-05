

# File syn\_cjt188.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cjt188.c**](syn__cjt188_8c.md)

[Go to the source code of this file](syn__cjt188_8c_source.md)

_Implementation of CJ/T 188-2004 / 2018 Protocol Driver._ 

* `#include "syn_cjt188.h"`
* `#include "../util/syn_pack.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**syn\_cjt188\_checksum**](#function-syn_cjt188_checksum) (const uint8\_t \* buf, size\_t len) <br>_Calculate CJ/T 188 8-bit checksum._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cjt188\_decoder\_feed**](#function-syn_cjt188_decoder_feed) ([**SYN\_CJT188\_Decoder**](structSYN__CJT188__Decoder.md) \* decoder, uint8\_t byte, [**SYN\_CJT188\_Frame**](structSYN__CJT188__Frame.md) \* out\_frame) <br>_Feed a single byte into the streaming decoder._  |
|  void | [**syn\_cjt188\_decoder\_init**](#function-syn_cjt188_decoder_init) ([**SYN\_CJT188\_Decoder**](structSYN__CJT188__Decoder.md) \* decoder) <br>_Initialize a CJ/T 188 streaming decoder instance._  |
|  size\_t | [**syn\_cjt188\_encode\_read\_req**](#function-syn_cjt188_encode_read_req) (uint8\_t \* out\_buf, size\_t buf\_size, uint8\_t meter\_type, const uint8\_t meter\_id, const uint8\_t vendor\_id, uint16\_t data\_id, uint8\_t seq) <br>_Encode a CJ/T 188 Read Meter Data Request (_ `0x01` _)._ |
|  size\_t | [**syn\_cjt188\_encode\_valve\_ctrl**](#function-syn_cjt188_encode_valve_ctrl) (uint8\_t \* out\_buf, size\_t buf\_size, uint8\_t meter\_type, const uint8\_t meter\_id, const uint8\_t vendor\_id, [**bool**](syn__defs_8h.md#enum-bool) open\_valve, uint8\_t seq) <br>_Encode a CJ/T 188 Valve Control Request (_ `0x04` _)._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cjt188\_parse\_frame**](#function-syn_cjt188_parse_frame) (const uint8\_t \* buf, size\_t len, [**SYN\_CJT188\_Frame**](structSYN__CJT188__Frame.md) \* out\_frame) <br>_Parse a raw CJ/T 188 binary frame._  |




























## Public Functions Documentation




### function syn\_cjt188\_checksum 

_Calculate CJ/T 188 8-bit checksum._ 
```C++
uint8_t syn_cjt188_checksum (
    const uint8_t * buf,
    size_t len
) 
```





**Parameters:**


* `buf` Pointer to frame starting at 0x68. 
* `len` Length of frame from 0x68 up to payload end (excluding CS and 0x16). 



**Returns:**

8-bit sum modulo 256. 





        

<hr>



### function syn\_cjt188\_decoder\_feed 

_Feed a single byte into the streaming decoder._ 
```C++
bool syn_cjt188_decoder_feed (
    SYN_CJT188_Decoder * decoder,
    uint8_t byte,
    SYN_CJT188_Frame * out_frame
) 
```





**Parameters:**


* `decoder` Decoder instance. 
* `byte` Received byte from UART/RS-485. 
* `out_frame` Output frame structure populated when a complete frame arrives. 



**Returns:**

true if a complete valid frame was decoded, false otherwise. 





        

<hr>



### function syn\_cjt188\_decoder\_init 

_Initialize a CJ/T 188 streaming decoder instance._ 
```C++
void syn_cjt188_decoder_init (
    SYN_CJT188_Decoder * decoder
) 
```





**Parameters:**


* `decoder` Pointer to decoder instance. 




        

<hr>



### function syn\_cjt188\_encode\_read\_req 

_Encode a CJ/T 188 Read Meter Data Request (_ `0x01` _)._
```C++
size_t syn_cjt188_encode_read_req (
    uint8_t * out_buf,
    size_t buf_size,
    uint8_t meter_type,
    const uint8_t meter_id,
    const uint8_t vendor_id,
    uint16_t data_id,
    uint8_t seq
) 
```





**Parameters:**


* `out_buf` Buffer to write encoded frame into. 
* `buf_size` Capacity of out\_buf (at least 17 bytes for 4x FE + frame). 
* `meter_type` Meter type (e.g. SYN\_CJT188\_METER\_COLD\_WATER). 
* `meter_id` 5-byte BCD meter ID array. 
* `vendor_id` 2-byte vendor code array. 
* `data_id` Data ID (e.g. SYN\_CJT188\_DI\_READ\_METER\_DATA). 
* `seq` Sequence number. 



**Returns:**

Total bytes written to out\_buf, or 0 if buffer too small. 





        

<hr>



### function syn\_cjt188\_encode\_valve\_ctrl 

_Encode a CJ/T 188 Valve Control Request (_ `0x04` _)._
```C++
size_t syn_cjt188_encode_valve_ctrl (
    uint8_t * out_buf,
    size_t buf_size,
    uint8_t meter_type,
    const uint8_t meter_id,
    const uint8_t vendor_id,
    bool open_valve,
    uint8_t seq
) 
```





**Parameters:**


* `out_buf` Buffer to write encoded frame into. 
* `buf_size` Capacity of out\_buf (at least 18 bytes). 
* `meter_type` Meter type. 
* `meter_id` 5-byte BCD meter ID array. 
* `vendor_id` 2-byte vendor code array. 
* `open_valve` true for OPEN (0x55), false for CLOSE (0x99). 
* `seq` Sequence number. 



**Returns:**

Total bytes written, or 0 if buffer too small. 





        

<hr>



### function syn\_cjt188\_parse\_frame 

_Parse a raw CJ/T 188 binary frame._ 
```C++
bool syn_cjt188_parse_frame (
    const uint8_t * buf,
    size_t len,
    SYN_CJT188_Frame * out_frame
) 
```





**Parameters:**


* `buf` Pointer to frame buffer (may begin with optional preamble `0xFE`). 
* `len` Length of raw buffer. 
* `out_frame` Pointer to output frame structure. 



**Returns:**

true if valid frame parsed, false otherwise. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cjt188.c`

