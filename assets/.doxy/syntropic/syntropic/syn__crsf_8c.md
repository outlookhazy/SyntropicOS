

# File syn\_crsf.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_crsf.c**](syn__crsf_8c.md)

[Go to the source code of this file](syn__crsf_8c_source.md)

_Zero-Heap CRSF (TBS Crossfire / ExpressLRS) Protocol Parser Implementation._ 

* `#include "syntropic/proto/syn_crsf.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**syn\_crsf\_calc\_crc**](#function-syn_crsf_calc_crc) (const uint8\_t \* buf, size\_t len) <br>_Calculate CRC8 DVB-S2 (poly 0xD5) for CRSF buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_crsf\_init**](#function-syn_crsf_init) ([**SYN\_CRSF\_Parser**](structSYN__CRSF__Parser.md) \* parser) <br>_Initialize CRSF parser._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_crsf\_parse\_byte**](#function-syn_crsf_parse_byte) ([**SYN\_CRSF\_Parser**](structSYN__CRSF__Parser.md) \* parser, uint8\_t byte, [**SYN\_CRSF\_FrameType**](syn__crsf_8h.md#enum-syn_crsf_frametype) \* type\_out) <br>_Process single byte from serial RX stream._  |
|  uint16\_t | [**syn\_crsf\_raw\_to\_us**](#function-syn_crsf_raw_to_us) (uint16\_t raw\_val) <br>_Convert raw 11-bit CRSF channel value to pulse width in microseconds (us)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**parse\_channels**](#function-parse_channels) (const uint8\_t \* payload, [**SYN\_CRSF\_ChannelsFrame**](structSYN__CRSF__ChannelsFrame.md) \* channels) <br>_Unpack 11-bit CRSF RC channels from packed byte stream._  |
|  void | [**parse\_link\_stats**](#function-parse_link_stats) (const uint8\_t \* payload, [**SYN\_CRSF\_LinkStats**](structSYN__CRSF__LinkStats.md) \* stats) <br>_Unpack CRSF link statistics from payload stream._  |


























## Public Functions Documentation




### function syn\_crsf\_calc\_crc 

_Calculate CRC8 DVB-S2 (poly 0xD5) for CRSF buffer._ 
```C++
uint8_t syn_crsf_calc_crc (
    const uint8_t * buf,
    size_t len
) 
```





**Parameters:**


* `buf` Pointer to data. 
* `len` Length in bytes. 



**Returns:**

8-bit CRC value. 





        

<hr>



### function syn\_crsf\_init 

_Initialize CRSF parser._ 
```C++
SYN_Status syn_crsf_init (
    SYN_CRSF_Parser * parser
) 
```





**Parameters:**


* `parser` Pointer to parser struct. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_crsf\_parse\_byte 

_Process single byte from serial RX stream._ 
```C++
SYN_Status syn_crsf_parse_byte (
    SYN_CRSF_Parser * parser,
    uint8_t byte,
    SYN_CRSF_FrameType * type_out
) 
```





**Parameters:**


* `parser` Pointer to parser struct. 
* `byte` Incoming byte. 
* `type_out` Optional pointer to receive frame type when a complete valid packet is parsed. 



**Returns:**

SYN\_OK on complete packet, SYN\_BUSY if incomplete, SYN\_ERROR on CRC failure. 





        

<hr>



### function syn\_crsf\_raw\_to\_us 

_Convert raw 11-bit CRSF channel value to pulse width in microseconds (us)._ 
```C++
uint16_t syn_crsf_raw_to_us (
    uint16_t raw_val
) 
```



Standard scaling:
* 170 raw -&gt; 988 us
* 992 raw -&gt; 1500 us
* 1908 raw -&gt; 2012 us






**Parameters:**


* `raw_val` 11-bit raw value (170..1908). 



**Returns:**

Pulse width in microseconds (clamped 988..2012 us). 





        

<hr>
## Public Static Functions Documentation




### function parse\_channels 

_Unpack 11-bit CRSF RC channels from packed byte stream._ 
```C++
static void parse_channels (
    const uint8_t * payload,
    SYN_CRSF_ChannelsFrame * channels
) 
```





**Parameters:**


* `payload` Pointer to raw payload buffer. 
* `channels` Pointer to channels frame structure. 




        

<hr>



### function parse\_link\_stats 

_Unpack CRSF link statistics from payload stream._ 
```C++
static void parse_link_stats (
    const uint8_t * payload,
    SYN_CRSF_LinkStats * stats
) 
```





**Parameters:**


* `payload` Pointer to raw payload buffer. 
* `stats` Pointer to link stats structure. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_crsf.c`

