

# File syn\_crsf.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_crsf.h**](syn__crsf_8h.md)

[Go to the source code of this file](syn__crsf_8h_source.md)

_Zero-Heap CRSF (TBS Crossfire / ExpressLRS) Protocol Parser._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_CRSF\_ChannelsFrame**](structSYN__CRSF__ChannelsFrame.md) <br> |
| struct | [**SYN\_CRSF\_LinkStats**](structSYN__CRSF__LinkStats.md) <br> |
| struct | [**SYN\_CRSF\_Parser**](structSYN__CRSF__Parser.md) <br> |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_CRSF\_FrameType**](#enum-syn_crsf_frametype)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**syn\_crsf\_calc\_crc**](#function-syn_crsf_calc_crc) (const uint8\_t \* buf, size\_t len) <br>_Calculate CRC8 DVB-S2 (poly 0xD5) for CRSF buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_crsf\_init**](#function-syn_crsf_init) ([**SYN\_CRSF\_Parser**](structSYN__CRSF__Parser.md) \* parser) <br>_Initialize CRSF parser._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_crsf\_parse\_byte**](#function-syn_crsf_parse_byte) ([**SYN\_CRSF\_Parser**](structSYN__CRSF__Parser.md) \* parser, uint8\_t byte, [**SYN\_CRSF\_FrameType**](syn__crsf_8h.md#enum-syn_crsf_frametype) \* type\_out) <br>_Process single byte from serial RX stream._  |
|  uint16\_t | [**syn\_crsf\_raw\_to\_us**](#function-syn_crsf_raw_to_us) (uint16\_t raw\_val) <br>_Convert raw 11-bit CRSF channel value to pulse width in microseconds (us)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_CRSF\_ADDR\_FC**](syn__crsf_8h.md#define-syn_crsf_addr_fc)  `0xC8U`<br> |
| define  | [**SYN\_CRSF\_MAX\_PACKET\_LEN**](syn__crsf_8h.md#define-syn_crsf_max_packet_len)  `64U`<br> |
| define  | [**SYN\_CRSF\_NUM\_CHANNELS**](syn__crsf_8h.md#define-syn_crsf_num_channels)  `16U`<br> |

## Detailed Description


CRSF Specifications:
* Baud rate: 420,000 / 921,600 / 1,875,000 bps, 8N1.
* Frame Format: [Device Addr] [Payload Length] [Frame Type] [Payload...] [CRC8]
* Address: 0xC8 (Flight Controller), 0xEA (Radio Transmitter).
* Types:
  * 0x16: RC Channels Frame (16 11-bit channels packed in 22 bytes, raw 170..1908 -&gt; 988..2012 us).
  * 0x14: Link Statistics (RSSI 1 & 2, Link Quality, SNR).
  * 0x08: Battery Telemetry (Voltage, Current, Capacity, Remaining %).
  * 0x02: GPS Telemetry (Lat, Lon, Groundspeed, Heading, Altitude, Satellites).


* CRC: CRC8 DVB-S2 (poly 0xD5) over Frame Type + Payload. 




    
## Public Types Documentation




### enum SYN\_CRSF\_FrameType 

```C++
enum SYN_CRSF_FrameType {
    SYN_CRSF_TYPE_GPS = 0x02U,
    SYN_CRSF_TYPE_BATTERY = 0x08U,
    SYN_CRSF_TYPE_LINK_STATISTICS = 0x14U,
    SYN_CRSF_TYPE_RC_CHANNELS = 0x16U
};
```



CRSF Frame Types. 


        

<hr>
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
## Macro Definition Documentation





### define SYN\_CRSF\_ADDR\_FC 

```C++
#define SYN_CRSF_ADDR_FC `0xC8U`
```



Flight Controller sync address (0xC8) 


        

<hr>



### define SYN\_CRSF\_MAX\_PACKET\_LEN 

```C++
#define SYN_CRSF_MAX_PACKET_LEN `64U`
```



Maximum CRSF frame byte length 


        

<hr>



### define SYN\_CRSF\_NUM\_CHANNELS 

```C++
#define SYN_CRSF_NUM_CHANNELS `16U`
```



Number of RC channels in CRSF frame (16) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_crsf.h`

