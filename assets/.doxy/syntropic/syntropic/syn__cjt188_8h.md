

# File syn\_cjt188.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cjt188.h**](syn__cjt188_8h.md)

[Go to the source code of this file](syn__cjt188_8h_source.md)

_CJ/T 188-2004 / CJ/T 188-2018 Smart Metering Protocol._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_CJT188\_Decoder**](structSYN__CJT188__Decoder.md) <br>_Streaming Decoder State Machine for CJ/T 188._  |
| struct | [**SYN\_CJT188\_Frame**](structSYN__CJT188__Frame.md) <br>_Decoded CJ/T 188 Protocol Frame._  |
| struct | [**SYN\_CJT188\_MeterData**](structSYN__CJT188__MeterData.md) <br>_Parsed CJ/T 188 Meter Response Data._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**syn\_cjt188\_checksum**](#function-syn_cjt188_checksum) (const uint8\_t \* buf, size\_t len) <br>_Calculate CJ/T 188 8-bit checksum._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cjt188\_decoder\_feed**](#function-syn_cjt188_decoder_feed) ([**SYN\_CJT188\_Decoder**](structSYN__CJT188__Decoder.md) \* decoder, uint8\_t byte, [**SYN\_CJT188\_Frame**](structSYN__CJT188__Frame.md) \* out\_frame) <br>_Feed a single byte into the streaming decoder._  |
|  void | [**syn\_cjt188\_decoder\_init**](#function-syn_cjt188_decoder_init) ([**SYN\_CJT188\_Decoder**](structSYN__CJT188__Decoder.md) \* decoder) <br>_Initialize a CJ/T 188 streaming decoder instance._  |
|  size\_t | [**syn\_cjt188\_encode\_read\_req**](#function-syn_cjt188_encode_read_req) (uint8\_t \* out\_buf, size\_t buf\_size, uint8\_t meter\_type, const uint8\_t meter\_id, const uint8\_t vendor\_id, uint16\_t data\_id, uint8\_t seq) <br>_Encode a CJ/T 188 Read Meter Data Request (_ `0x01` _)._ |
|  size\_t | [**syn\_cjt188\_encode\_valve\_ctrl**](#function-syn_cjt188_encode_valve_ctrl) (uint8\_t \* out\_buf, size\_t buf\_size, uint8\_t meter\_type, const uint8\_t meter\_id, const uint8\_t vendor\_id, [**bool**](syn__defs_8h.md#enum-bool) open\_valve, uint8\_t seq) <br>_Encode a CJ/T 188 Valve Control Request (_ `0x04` _)._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cjt188\_parse\_frame**](#function-syn_cjt188_parse_frame) (const uint8\_t \* buf, size\_t len, [**SYN\_CJT188\_Frame**](structSYN__CJT188__Frame.md) \* out\_frame) <br>_Parse a raw CJ/T 188 binary frame._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_CJT188\_CTRL\_READ\_DATA**](syn__cjt188_8h.md#define-syn_cjt188_ctrl_read_data)  `0x01U`<br> |
| define  | [**SYN\_CJT188\_CTRL\_READ\_DATA\_RESP**](syn__cjt188_8h.md#define-syn_cjt188_ctrl_read_data_resp)  `0x81U`<br> |
| define  | [**SYN\_CJT188\_CTRL\_WRITE\_ADDR**](syn__cjt188_8h.md#define-syn_cjt188_ctrl_write_addr)  `0x15U`<br> |
| define  | [**SYN\_CJT188\_CTRL\_WRITE\_ADDR\_RESP**](syn__cjt188_8h.md#define-syn_cjt188_ctrl_write_addr_resp)  `0x95U`<br> |
| define  | [**SYN\_CJT188\_CTRL\_WRITE\_DATA**](syn__cjt188_8h.md#define-syn_cjt188_ctrl_write_data)  `0x04U`<br> |
| define  | [**SYN\_CJT188\_CTRL\_WRITE\_DATA\_RESP**](syn__cjt188_8h.md#define-syn_cjt188_ctrl_write_data_resp)  `0x84U`<br> |
| define  | [**SYN\_CJT188\_DI\_READ\_HIST\_DATA**](syn__cjt188_8h.md#define-syn_cjt188_di_read_hist_data)  `0xD120U`<br> |
| define  | [**SYN\_CJT188\_DI\_READ\_METER\_DATA**](syn__cjt188_8h.md#define-syn_cjt188_di_read_meter_data)  `0x901FU`<br> |
| define  | [**SYN\_CJT188\_DI\_VALVE\_CONTROL**](syn__cjt188_8h.md#define-syn_cjt188_di_valve_control)  `0xA017U`<br> |
| define  | [**SYN\_CJT188\_END\_BYTE**](syn__cjt188_8h.md#define-syn_cjt188_end_byte)  `0x16U`<br> |
| define  | [**SYN\_CJT188\_MAX\_FRAME\_SIZE**](syn__cjt188_8h.md#define-syn_cjt188_max_frame_size)  `128U`<br> |
| define  | [**SYN\_CJT188\_METER\_COLD\_WATER**](syn__cjt188_8h.md#define-syn_cjt188_meter_cold_water)  `0x10U`<br> |
| define  | [**SYN\_CJT188\_METER\_COOLING**](syn__cjt188_8h.md#define-syn_cjt188_meter_cooling)  `0x21U`<br> |
| define  | [**SYN\_CJT188\_METER\_DRINK\_WATER**](syn__cjt188_8h.md#define-syn_cjt188_meter_drink_water)  `0x12U`<br> |
| define  | [**SYN\_CJT188\_METER\_GAS**](syn__cjt188_8h.md#define-syn_cjt188_meter_gas)  `0x30U`<br> |
| define  | [**SYN\_CJT188\_METER\_HEAT**](syn__cjt188_8h.md#define-syn_cjt188_meter_heat)  `0x20U`<br> |
| define  | [**SYN\_CJT188\_METER\_HOT\_WATER**](syn__cjt188_8h.md#define-syn_cjt188_meter_hot_water)  `0x11U`<br> |
| define  | [**SYN\_CJT188\_METER\_POWER**](syn__cjt188_8h.md#define-syn_cjt188_meter_power)  `0x40U`<br> |
| define  | [**SYN\_CJT188\_METER\_RECLAIMED\_WATER**](syn__cjt188_8h.md#define-syn_cjt188_meter_reclaimed_water)  `0x13U`<br> |
| define  | [**SYN\_CJT188\_MIN\_FRAME\_SIZE**](syn__cjt188_8h.md#define-syn_cjt188_min_frame_size)  `13U`<br> |
| define  | [**SYN\_CJT188\_PREAMBLE\_BYTE**](syn__cjt188_8h.md#define-syn_cjt188_preamble_byte)  `0xFEU`<br> |
| define  | [**SYN\_CJT188\_START\_BYTE**](syn__cjt188_8h.md#define-syn_cjt188_start_byte)  `0x68U`<br> |
| define  | [**SYN\_CJT188\_VALVE\_CLOSE**](syn__cjt188_8h.md#define-syn_cjt188_valve_close)  `0x99U`<br> |
| define  | [**SYN\_CJT188\_VALVE\_OPEN**](syn__cjt188_8h.md#define-syn_cjt188_valve_open)  `0x55U`<br> |

## Detailed Description


Implements non-blocking, zero-malloc CJ/T 188 protocol framing, data encoding, valve control commands, 8-bit checksum verification, and streaming UART decoding for cold water, hot water, heat, and gas meters. 


    
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
## Macro Definition Documentation





### define SYN\_CJT188\_CTRL\_READ\_DATA 

```C++
#define SYN_CJT188_CTRL_READ_DATA `0x01U`
```



Read meter data request 
 


        

<hr>



### define SYN\_CJT188\_CTRL\_READ\_DATA\_RESP 

```C++
#define SYN_CJT188_CTRL_READ_DATA_RESP `0x81U`
```



Read meter data response 
 


        

<hr>



### define SYN\_CJT188\_CTRL\_WRITE\_ADDR 

```C++
#define SYN_CJT188_CTRL_WRITE_ADDR `0x15U`
```



Write meter address request 


        

<hr>



### define SYN\_CJT188\_CTRL\_WRITE\_ADDR\_RESP 

```C++
#define SYN_CJT188_CTRL_WRITE_ADDR_RESP `0x95U`
```



Write meter address resp 
 


        

<hr>



### define SYN\_CJT188\_CTRL\_WRITE\_DATA 

```C++
#define SYN_CJT188_CTRL_WRITE_DATA `0x04U`
```



Write data / control req 
 


        

<hr>



### define SYN\_CJT188\_CTRL\_WRITE\_DATA\_RESP 

```C++
#define SYN_CJT188_CTRL_WRITE_DATA_RESP `0x84U`
```



Write data / control resp 
 


        

<hr>



### define SYN\_CJT188\_DI\_READ\_HIST\_DATA 

```C++
#define SYN_CJT188_DI_READ_HIST_DATA `0xD120U`
```



Read historical data 
 


        

<hr>



### define SYN\_CJT188\_DI\_READ\_METER\_DATA 

```C++
#define SYN_CJT188_DI_READ_METER_DATA `0x901FU`
```



Read meter current data 
 


        

<hr>



### define SYN\_CJT188\_DI\_VALVE\_CONTROL 

```C++
#define SYN_CJT188_DI_VALVE_CONTROL `0xA017U`
```



Valve control command 
 


        

<hr>



### define SYN\_CJT188\_END\_BYTE 

```C++
#define SYN_CJT188_END_BYTE `0x16U`
```



Frame end delimiter 
 


        

<hr>



### define SYN\_CJT188\_MAX\_FRAME\_SIZE 

```C++
#define SYN_CJT188_MAX_FRAME_SIZE `128U`
```



Max supported frame length 
 


        

<hr>



### define SYN\_CJT188\_METER\_COLD\_WATER 

```C++
#define SYN_CJT188_METER_COLD_WATER `0x10U`
```



Cold water meter 
 


        

<hr>



### define SYN\_CJT188\_METER\_COOLING 

```C++
#define SYN_CJT188_METER_COOLING `0x21U`
```



Heat meter (cooling) 
 


        

<hr>



### define SYN\_CJT188\_METER\_DRINK\_WATER 

```C++
#define SYN_CJT188_METER_DRINK_WATER `0x12U`
```



Direct drinking water meter 


        

<hr>



### define SYN\_CJT188\_METER\_GAS 

```C++
#define SYN_CJT188_METER_GAS `0x30U`
```



Gas meter 
 


        

<hr>



### define SYN\_CJT188\_METER\_HEAT 

```C++
#define SYN_CJT188_METER_HEAT `0x20U`
```



Heat / caloric meter (heat) 


        

<hr>



### define SYN\_CJT188\_METER\_HOT\_WATER 

```C++
#define SYN_CJT188_METER_HOT_WATER `0x11U`
```



Domestic hot water meter 
 


        

<hr>



### define SYN\_CJT188\_METER\_POWER 

```C++
#define SYN_CJT188_METER_POWER `0x40U`
```



Power meter 
 


        

<hr>



### define SYN\_CJT188\_METER\_RECLAIMED\_WATER 

```C++
#define SYN_CJT188_METER_RECLAIMED_WATER `0x13U`
```



Reclaimed water meter 
 


        

<hr>



### define SYN\_CJT188\_MIN\_FRAME\_SIZE 

```C++
#define SYN_CJT188_MIN_FRAME_SIZE `13U`
```



Min frame without preamble 
 


        

<hr>



### define SYN\_CJT188\_PREAMBLE\_BYTE 

```C++
#define SYN_CJT188_PREAMBLE_BYTE `0xFEU`
```



Leading preamble byte 
 


        

<hr>



### define SYN\_CJT188\_START\_BYTE 

```C++
#define SYN_CJT188_START_BYTE `0x68U`
```



Frame start delimiter 
 


        

<hr>



### define SYN\_CJT188\_VALVE\_CLOSE 

```C++
#define SYN_CJT188_VALVE_CLOSE `0x99U`
```



Close valve command 
 


        

<hr>



### define SYN\_CJT188\_VALVE\_OPEN 

```C++
#define SYN_CJT188_VALVE_OPEN `0x55U`
```



Open valve command 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cjt188.h`

