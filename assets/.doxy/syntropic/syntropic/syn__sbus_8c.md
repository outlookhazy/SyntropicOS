

# File syn\_sbus.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_sbus.c**](syn__sbus_8c.md)

[Go to the source code of this file](syn__sbus_8c_source.md)

_Zero-Heap SBUS (Futaba/FrSky) 16-Channel Receiver Decoder Implementation._ 

* `#include "syntropic/proto/syn_sbus.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sbus\_decode\_buffer**](#function-syn_sbus_decode_buffer) (const uint8\_t buf, [**SYN\_SBUS\_Frame**](structSYN__SBUS__Frame.md) \* frame) <br>_Decode raw 25-byte SBUS frame buffer into channel outputs._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sbus\_init**](#function-syn_sbus_init) ([**SYN\_SBUS\_Parser**](structSYN__SBUS__Parser.md) \* parser) <br>_Initialize SBUS parser state machine._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sbus\_parse\_byte**](#function-syn_sbus_parse_byte) ([**SYN\_SBUS\_Parser**](structSYN__SBUS__Parser.md) \* parser, uint8\_t byte, [**SYN\_SBUS\_Frame**](structSYN__SBUS__Frame.md) \* frame) <br>_Process single incoming byte from UART._  |
|  uint16\_t | [**syn\_sbus\_raw\_to\_us**](#function-syn_sbus_raw_to_us) (uint16\_t raw\_val) <br>_Convert raw 11-bit SBUS channel value to pulse width in microseconds (us)._  |




























## Public Functions Documentation




### function syn\_sbus\_decode\_buffer 

_Decode raw 25-byte SBUS frame buffer into channel outputs._ 
```C++
SYN_Status syn_sbus_decode_buffer (
    const uint8_t buf,
    SYN_SBUS_Frame * frame
) 
```



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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_sbus.c`

