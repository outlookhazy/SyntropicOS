

# File syn\_ibus.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ibus.h**](syn__ibus_8h.md)

[Go to the source code of this file](syn__ibus_8h_source.md)

_Zero-Heap IBUS (FlySky) 14-Channel Receiver Decoder._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_IBUS\_Frame**](structSYN__IBUS__Frame.md) <br> |
| struct | [**SYN\_IBUS\_Parser**](structSYN__IBUS__Parser.md) <br> |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**syn\_ibus\_calc\_checksum**](#function-syn_ibus_calc_checksum) (const uint8\_t buf) <br>_Calculate IBUS 16-bit checksum._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ibus\_init**](#function-syn_ibus_init) ([**SYN\_IBUS\_Parser**](structSYN__IBUS__Parser.md) \* parser) <br>_Initialize IBUS parser._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ibus\_parse\_byte**](#function-syn_ibus_parse_byte) ([**SYN\_IBUS\_Parser**](structSYN__IBUS__Parser.md) \* parser, uint8\_t byte, [**SYN\_IBUS\_Frame**](structSYN__IBUS__Frame.md) \* frame) <br>_Process single byte from serial RX stream._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_IBUS\_FRAME\_SIZE**](syn__ibus_8h.md#define-syn_ibus_frame_size)  `32U`<br> |
| define  | [**SYN\_IBUS\_HEADER1**](syn__ibus_8h.md#define-syn_ibus_header1)  `0x20U`<br> |
| define  | [**SYN\_IBUS\_HEADER2**](syn__ibus_8h.md#define-syn_ibus_header2)  `0x40U`<br> |
| define  | [**SYN\_IBUS\_NUM\_CHANNELS**](syn__ibus_8h.md#define-syn_ibus_num_channels)  `14U`<br> |

## Detailed Description


IBUS Protocol Specifications:
* Baud rate: 115,200 bps, 8N1.
* Frame length: 32 bytes.
* Header: 0x20 0x40
* Channels: 14 16-bit little-endian channels (bytes 2..29, raw 1000..2000 us).
* Checksum: 16-bit sum checksum (`0xFFFF - sum(bytes[0..29])`). 




    
## Public Functions Documentation




### function syn\_ibus\_calc\_checksum 

_Calculate IBUS 16-bit checksum._ 
```C++
uint16_t syn_ibus_calc_checksum (
    const uint8_t buf
) 
```





**Parameters:**


* `buf` Pointer to 30 data bytes. 



**Returns:**

16-bit expected checksum (`0xFFFF - sum(bytes)`). 





        

<hr>



### function syn\_ibus\_init 

_Initialize IBUS parser._ 
```C++
SYN_Status syn_ibus_init (
    SYN_IBUS_Parser * parser
) 
```





**Parameters:**


* `parser` Pointer to parser struct. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ibus\_parse\_byte 

_Process single byte from serial RX stream._ 
```C++
SYN_Status syn_ibus_parse_byte (
    SYN_IBUS_Parser * parser,
    uint8_t byte,
    SYN_IBUS_Frame * frame
) 
```





**Parameters:**


* `parser` Pointer to parser struct. 
* `byte` Incoming byte. 
* `frame` Pointer to output frame struct (populated when a valid 32-byte frame is decoded). 



**Returns:**

SYN\_OK on complete frame, SYN\_BUSY if byte accepted but incomplete, SYN\_ERROR on bad header/checksum. 





        

<hr>
## Macro Definition Documentation





### define SYN\_IBUS\_FRAME\_SIZE 

```C++
#define SYN_IBUS_FRAME_SIZE `32U`
```



IBUS frame size in bytes (32) 


        

<hr>



### define SYN\_IBUS\_HEADER1 

```C++
#define SYN_IBUS_HEADER1 `0x20U`
```



First header byte of IBUS frame (0x20) 


        

<hr>



### define SYN\_IBUS\_HEADER2 

```C++
#define SYN_IBUS_HEADER2 `0x40U`
```



Second header byte of IBUS frame (0x40) 


        

<hr>



### define SYN\_IBUS\_NUM\_CHANNELS 

```C++
#define SYN_IBUS_NUM_CHANNELS `14U`
```



Number of RC channels in IBUS frame (14) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ibus.h`

