

# File syn\_sbus.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_sbus.h**](syn__sbus_8h.md)

[Go to the source code of this file](syn__sbus_8h_source.md)

_Zero-Heap SBUS (Futaba/FrSky) 16-Channel Receiver Decoder._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_SBUS\_Frame**](structSYN__SBUS__Frame.md) <br> |
| struct | [**SYN\_SBUS\_Parser**](structSYN__SBUS__Parser.md) <br> |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sbus\_decode\_buffer**](#function-syn_sbus_decode_buffer) (const uint8\_t buf, [**SYN\_SBUS\_Frame**](structSYN__SBUS__Frame.md) \* frame) <br>_Decode raw 25-byte SBUS frame buffer directly._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sbus\_init**](#function-syn_sbus_init) ([**SYN\_SBUS\_Parser**](structSYN__SBUS__Parser.md) \* parser) <br>_Initialize SBUS parser state machine._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sbus\_parse\_byte**](#function-syn_sbus_parse_byte) ([**SYN\_SBUS\_Parser**](structSYN__SBUS__Parser.md) \* parser, uint8\_t byte, [**SYN\_SBUS\_Frame**](structSYN__SBUS__Frame.md) \* frame) <br>_Process single incoming byte from UART._  |
|  uint16\_t | [**syn\_sbus\_raw\_to\_us**](#function-syn_sbus_raw_to_us) (uint16\_t raw\_val) <br>_Convert raw 11-bit SBUS channel value to pulse width in microseconds (us)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_SBUS\_FRAME\_SIZE**](syn__sbus_8h.md#define-syn_sbus_frame_size)  `25`<br> |
| define  | [**SYN\_SBUS\_HEADER**](syn__sbus_8h.md#define-syn_sbus_header)  `0x0FU`<br> |
| define  | [**SYN\_SBUS\_NUM\_CHANNELS**](syn__sbus_8h.md#define-syn_sbus_num_channels)  `16`<br> |

## Detailed Description


SBUS Protocol Specifications:
* Baud rate: 100,000 bps, 8 data bits, Even parity, 2 stop bits (8E2), Inverted logic.
* Frame length: 25 bytes.
* Header byte: 0x0F
* Channels: 16 11-bit channels (packed bitstream across bytes 1..22, raw range 172..1811, 1000..2000 us).
* Flags (byte 23): Channel 17 (bit 0), Channel 18 (bit 1), Frame Loss (bit 2), Failsafe (bit 3).
* Footer byte: 0x00 (or 0x04 for endbyte flags). 




    
## Public Functions Documentation




### function syn\_sbus\_decode\_buffer 

_Decode raw 25-byte SBUS frame buffer directly._ 
```C++
SYN_Status syn_sbus_decode_buffer (
    const uint8_t buf,
    SYN_SBUS_Frame * frame
) 
```





**Parameters:**


* `buf` Pointer to 25-byte buffer. 
* `frame` Pointer to destination frame struct. 



**Returns:**

SYN\_OK on successful decode, SYN\_INVALID\_PARAM or SYN\_ERROR on bad header/footer.


Decode raw 25-byte SBUS frame buffer directly.




**Parameters:**


* `buf` Pointer to 25-byte raw SBUS frame buffer. 
* `frame` Pointer to output SBUS decoded frame structure. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on header mismatch. 





        

<hr>



### function syn\_sbus\_init 

_Initialize SBUS parser state machine._ 
```C++
SYN_Status syn_sbus_init (
    SYN_SBUS_Parser * parser
) 
```





**Parameters:**


* `parser` Pointer to parser struct. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_sbus\_parse\_byte 

_Process single incoming byte from UART._ 
```C++
SYN_Status syn_sbus_parse_byte (
    SYN_SBUS_Parser * parser,
    uint8_t byte,
    SYN_SBUS_Frame * frame
) 
```





**Parameters:**


* `parser` Pointer to parser struct. 
* `byte` Incoming serial byte. 
* `frame` Pointer to output frame struct (populated when a complete valid 25-byte frame is decoded). 



**Returns:**

SYN\_OK if complete frame decoded, SYN\_BUSY if byte accepted but frame incomplete, SYN\_ERROR on header mismatch. 





        

<hr>



### function syn\_sbus\_raw\_to\_us 

_Convert raw 11-bit SBUS channel value to pulse width in microseconds (us)._ 
```C++
uint16_t syn_sbus_raw_to_us (
    uint16_t raw_val
) 
```



Standard mapping:
* 172 raw -&gt; 1000 us
* 992 raw -&gt; 1500 us
* 1811 raw -&gt; 2000 us






**Parameters:**


* `raw_val` 11-bit raw SBUS channel value (172..1811). 



**Returns:**

Pulse width in microseconds (clamped 1000..2000 us). 





        

<hr>
## Macro Definition Documentation





### define SYN\_SBUS\_FRAME\_SIZE 

```C++
#define SYN_SBUS_FRAME_SIZE `25`
```



Total byte length of standard SBUS frame (25) 


        

<hr>



### define SYN\_SBUS\_HEADER 

```C++
#define SYN_SBUS_HEADER `0x0FU`
```



SBUS frame header marker byte (0x0F) 


        

<hr>



### define SYN\_SBUS\_NUM\_CHANNELS 

```C++
#define SYN_SBUS_NUM_CHANNELS `16`
```



Number of analog channels in SBUS frame (16) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_sbus.h`

