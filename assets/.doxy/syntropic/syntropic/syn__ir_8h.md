

# File syn\_ir.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ir.h**](syn__ir_8h.md)

[Go to the source code of this file](syn__ir_8h_source.md)

_Infrared (IR) Remote Control Protocol Engine (NEC, Sony, RC5, RC6, Samsung, Panasonic, Denon, Apple)._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md) <br>_Non-blocking IR Decoder Handle._  |
| struct | [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) <br>_Decoded IR Remote Control Frame._  |
| struct | [**SYN\_IR\_Pulse**](structSYN__IR__Pulse.md) <br>_Single pulse duration timing pair for IR transmitter._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_IR\_EncodingType**](#enum-syn_ir_encodingtype)  <br>_IR signal encoding types._  |
| enum  | [**SYN\_IR\_FsmState**](#enum-syn_ir_fsmstate)  <br> |
| enum  | [**SYN\_IR\_Protocol**](#enum-syn_ir_protocol)  <br>_Supported IR remote control protocols._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ir\_decode\_pulse**](#function-syn_ir_decode_pulse) ([**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md) \* decoder, uint16\_t duration\_us, [**bool**](syn__defs_8h.md#enum-bool) is\_mark, [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame\_out) <br>_Process a single pulse duration (mark or space) in microseconds._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ir\_decode\_timeout**](#function-syn_ir_decode_timeout) ([**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md) \* decoder, [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame\_out) <br>_Signal a gap/timeout (&gt;10ms silence) to finalize protocols._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ir\_decoder\_init**](#function-syn_ir_decoder_init) ([**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md) \* decoder) <br>_Initialize or reset an IR decoder instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ir\_encode\_frame**](#function-syn_ir_encode_frame) (const [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame, [**SYN\_IR\_Pulse**](structSYN__IR__Pulse.md) \* pulse\_buf, size\_t buf\_len, size\_t \* count\_out) <br>_Encode a frame into a sequence of pulse timing pairs._  |
|  const char \* | [**syn\_ir\_protocol\_name**](#function-syn_ir_protocol_name) ([**SYN\_IR\_Protocol**](syn__ir_8h.md#enum-syn_ir_protocol) proto) <br>_Get human-readable protocol string name._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_IR\_FLAG\_NONE**](syn__ir_8h.md#define-syn_ir_flag_none)  `0x0000U`<br> |
| define  | [**SYN\_IR\_FLAG\_REPEAT**](syn__ir_8h.md#define-syn_ir_flag_repeat)  `(1U &lt;&lt; 0)`<br> |
| define  | [**SYN\_IR\_FLAG\_TOGGLE**](syn__ir_8h.md#define-syn_ir_flag_toggle)  `(1U &lt;&lt; 1)`<br> |

## Detailed Description


Implements non-blocking, zero-malloc IR pulse decoding and encoding for common consumer remote control protocols using microsecond pulse timing inputs.




**
**


* NEC (Standard & Extended 32-bit)
* Sony SIRCS (12-bit, 15-bit, 20-bit)
* Philips RC5 (14-bit Manchester)
* Philips RC6 Mode 0 (21-bit Manchester)
* Samsung (32-bit PDM)
* Kaseikyo / Panasonic (48-bit PDM)
* Denon / Sharp (15-bit PDM)
* Apple (32-bit NEC variant)






**
**


```C++
static SYN_IR_Decoder decoder;
syn_ir_decoder_init(&decoder);

// In Timer Input Capture / EXTI ISR:
SYN_IR_Frame frame;
if (syn_ir_decode_pulse(&decoder, duration_us, is_mark, &frame)) {
    // Process decoded frame: frame.protocol, frame.address, frame.command
}
```
 





    
## Public Types Documentation




### enum SYN\_IR\_EncodingType 

_IR signal encoding types._ 
```C++
enum SYN_IR_EncodingType {
    SYN_IR_ENC_PDM = 0,
    SYN_IR_ENC_PWM = 1,
    SYN_IR_ENC_MANCHESTER = 2,
    SYN_IR_ENC_PPM = 3
};
```




<hr>



### enum SYN\_IR\_FsmState 

```C++
enum SYN_IR_FsmState {
    SYN_IR_STATE_IDLE = 0,
    SYN_IR_STATE_LEADER,
    SYN_IR_STATE_DATA,
    SYN_IR_STATE_TRAILER
};
```



IR Decoder Finite State Machine States. 


        

<hr>



### enum SYN\_IR\_Protocol 

_Supported IR remote control protocols._ 
```C++
enum SYN_IR_Protocol {
    SYN_IR_PROTO_UNKNOWN = 0,
    SYN_IR_PROTO_NEC,
    SYN_IR_PROTO_NEC_EXTENDED,
    SYN_IR_PROTO_SONY_12,
    SYN_IR_PROTO_SONY_15,
    SYN_IR_PROTO_SONY_20,
    SYN_IR_PROTO_RC5,
    SYN_IR_PROTO_RC6,
    SYN_IR_PROTO_SAMSUNG,
    SYN_IR_PROTO_KASEIKYO,
    SYN_IR_PROTO_DENON,
    SYN_IR_PROTO_APPLE,
    SYN_IR_PROTO_COUNT
};
```




<hr>
## Public Functions Documentation




### function syn\_ir\_decode\_pulse 

_Process a single pulse duration (mark or space) in microseconds._ 
```C++
bool syn_ir_decode_pulse (
    SYN_IR_Decoder * decoder,
    uint16_t duration_us,
    bool is_mark,
    SYN_IR_Frame * frame_out
) 
```





**Parameters:**


* `decoder` Pointer to IR decoder handle. 
* `duration_us` Pulse duration in microseconds. 
* `is_mark` true if pulse is active IR carrier (mark), false if space. 
* `frame_out` [out] Populated with decoded frame when function returns true. 



**Returns:**

true if a complete, valid frame was decoded, false otherwise. 





        

<hr>



### function syn\_ir\_decode\_timeout 

_Signal a gap/timeout (&gt;10ms silence) to finalize protocols._ 
```C++
bool syn_ir_decode_timeout (
    SYN_IR_Decoder * decoder,
    SYN_IR_Frame * frame_out
) 
```





**Parameters:**


* `decoder` Pointer to IR decoder handle. 
* `frame_out` [out] Populated with decoded frame if timeout completed frame. 



**Returns:**

true if a complete valid frame was finalized by timeout, false otherwise. 





        

<hr>



### function syn\_ir\_decoder\_init 

_Initialize or reset an IR decoder instance._ 
```C++
SYN_Status syn_ir_decoder_init (
    SYN_IR_Decoder * decoder
) 
```





**Parameters:**


* `decoder` Pointer to IR decoder handle. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on NULL. 





        

<hr>



### function syn\_ir\_encode\_frame 

_Encode a frame into a sequence of pulse timing pairs._ 
```C++
SYN_Status syn_ir_encode_frame (
    const SYN_IR_Frame * frame,
    SYN_IR_Pulse * pulse_buf,
    size_t buf_len,
    size_t * count_out
) 
```





**Parameters:**


* `frame` Pointer to frame to encode. 
* `pulse_buf` Output array for pulse timings. 
* `buf_len` Capacity of pulse\_buf. 
* `count_out` [out] Number of pulse timing pairs written. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on failure. 





        

<hr>



### function syn\_ir\_protocol\_name 

_Get human-readable protocol string name._ 
```C++
const char * syn_ir_protocol_name (
    SYN_IR_Protocol proto
) 
```





**Parameters:**


* `proto` Protocol enum value. 



**Returns:**

Pointer to const string. 





        

<hr>
## Macro Definition Documentation





### define SYN\_IR\_FLAG\_NONE 

```C++
#define SYN_IR_FLAG_NONE `0x0000U`
```



No frame flags active 


        

<hr>



### define SYN\_IR\_FLAG\_REPEAT 

```C++
#define SYN_IR_FLAG_REPEAT `(1U << 0)`
```



Set if this frame is a repeat code / held key 


        

<hr>



### define SYN\_IR\_FLAG\_TOGGLE 

```C++
#define SYN_IR_FLAG_TOGGLE `(1U << 1)`
```



Toggle bit active (RC5 / RC6) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ir.h`

