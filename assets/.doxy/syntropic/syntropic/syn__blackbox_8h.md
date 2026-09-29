

# File syn\_blackbox.h



[**FileList**](files.md) **>** [**log**](dir_1a216ccd2a39b426a7ff78fb20f852eb.md) **>** [**syn\_blackbox.h**](syn__blackbox_8h.md)

[Go to the source code of this file](syn__blackbox_8h_source.md)

_Zero-Heap Flight Telemetry Blackbox Binary Recorder._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Blackbox**](structSYN__Blackbox.md) <br> |
| struct | [**SYN\_Blackbox\_Record**](structSYN__Blackbox__Record.md) <br> |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_blackbox\_encode\_delta**](#function-syn_blackbox_encode_delta) ([**SYN\_Blackbox**](structSYN__Blackbox.md) \* bb, const [**SYN\_Blackbox\_Record**](structSYN__Blackbox__Record.md) \* record, uint8\_t \* buf\_out, size\_t \* out\_len) <br>_Encode a compact Predictive Delta Frame ('P') relative to last frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_blackbox\_encode\_intra**](#function-syn_blackbox_encode_intra) ([**SYN\_Blackbox**](structSYN__Blackbox.md) \* bb, const [**SYN\_Blackbox\_Record**](structSYN__Blackbox__Record.md) \* record, uint8\_t \* buf\_out, size\_t \* out\_len) <br>_Encode a full Intra Frame ('I') into raw binary log stream._  |
|  size\_t | [**syn\_blackbox\_encode\_varint**](#function-syn_blackbox_encode_varint) (int32\_t val, uint8\_t \* buf\_out) <br>_Encode a 32-bit signed integer using ZigZag + LEB128 variable-length format._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_blackbox\_init**](#function-syn_blackbox_init) ([**SYN\_Blackbox**](structSYN__Blackbox.md) \* bb) <br>_Initialize Blackbox recorder._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_BLACKBOX\_FRAME\_DELTA**](syn__blackbox_8h.md#define-syn_blackbox_frame_delta)  `'P'`<br> |
| define  | [**SYN\_BLACKBOX\_FRAME\_INTRA**](syn__blackbox_8h.md#define-syn_blackbox_frame_intra)  `'I'`<br> |
| define  | [**SYN\_BLACKBOX\_FRAME\_SLOW**](syn__blackbox_8h.md#define-syn_blackbox_frame_slow)  `'[**S**](syn__x25519_8c.md#function-s)'`<br> |

## Detailed Description


High-speed binary logger for SPI Flash or SD Card logging. Frame Formats:
* 'H' (Header): ASCII configuration settings and flight controller parameters.
* 'I' (Intra Frame): Full uncompressed 32-bit state vector (Gyro, Accel, Motor outputs, Setpoints).
* 'P' (Predictive Delta Frame): Compact variable-length ZigZag delta encoded frame. 




    
## Public Functions Documentation




### function syn\_blackbox\_encode\_delta 

_Encode a compact Predictive Delta Frame ('P') relative to last frame._ 
```C++
SYN_Status syn_blackbox_encode_delta (
    SYN_Blackbox * bb,
    const SYN_Blackbox_Record * record,
    uint8_t * buf_out,
    size_t * out_len
) 
```





**Parameters:**


* `bb` Pointer to Blackbox instance. 
* `record` Pointer to flight state record. 
* `buf_out` Output buffer (must hold at least 32 bytes). 
* `out_len` Pointer to receive encoded byte length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_blackbox\_encode\_intra 

_Encode a full Intra Frame ('I') into raw binary log stream._ 
```C++
SYN_Status syn_blackbox_encode_intra (
    SYN_Blackbox * bb,
    const SYN_Blackbox_Record * record,
    uint8_t * buf_out,
    size_t * out_len
) 
```





**Parameters:**


* `bb` Pointer to Blackbox instance. 
* `record` Pointer to flight state record. 
* `buf_out` Output buffer (must hold at least 32 bytes). 
* `out_len` Pointer to receive encoded byte length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_blackbox\_encode\_varint 

_Encode a 32-bit signed integer using ZigZag + LEB128 variable-length format._ 
```C++
size_t syn_blackbox_encode_varint (
    int32_t val,
    uint8_t * buf_out
) 
```





**Parameters:**


* `val` Signed integer. 
* `buf_out` Output buffer (must hold at least 5 bytes). 



**Returns:**

Number of bytes written to buffer (1..5). 





        

<hr>



### function syn\_blackbox\_init 

_Initialize Blackbox recorder._ 
```C++
SYN_Status syn_blackbox_init (
    SYN_Blackbox * bb
) 
```





**Parameters:**


* `bb` Pointer to Blackbox instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Macro Definition Documentation





### define SYN\_BLACKBOX\_FRAME\_DELTA 

```C++
#define SYN_BLACKBOX_FRAME_DELTA `'P'`
```



Delta frame identifier 'P' (predictive delta-encoded) 


        

<hr>



### define SYN\_BLACKBOX\_FRAME\_INTRA 

```C++
#define SYN_BLACKBOX_FRAME_INTRA `'I'`
```



Intra frame identifier 'I' (full uncompressed frame) 


        

<hr>



### define SYN\_BLACKBOX\_FRAME\_SLOW 

```C++
#define SYN_BLACKBOX_FRAME_SLOW `' S '`
```



Slow frame identifier 'S' (low-rate status fields) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/log/syn_blackbox.h`

