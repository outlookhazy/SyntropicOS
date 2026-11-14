

# File syn\_ibus.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ibus.c**](syn__ibus_8c.md)

[Go to the source code of this file](syn__ibus_8c_source.md)

_Zero-Heap IBUS (FlySky) 14-Channel Receiver Decoder Implementation._ 

* `#include "syntropic/proto/syn_ibus.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**syn\_ibus\_calc\_checksum**](#function-syn_ibus_calc_checksum) (const uint8\_t buf) <br>_Calculate IBUS 16-bit checksum._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ibus\_init**](#function-syn_ibus_init) ([**SYN\_IBUS\_Parser**](structSYN__IBUS__Parser.md) \* parser) <br>_Initialize IBUS parser._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ibus\_parse\_byte**](#function-syn_ibus_parse_byte) ([**SYN\_IBUS\_Parser**](structSYN__IBUS__Parser.md) \* parser, uint8\_t byte, [**SYN\_IBUS\_Frame**](structSYN__IBUS__Frame.md) \* frame) <br>_Process single byte from serial RX stream._  |




























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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ibus.c`

