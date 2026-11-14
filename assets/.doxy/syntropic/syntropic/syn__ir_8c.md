

# File syn\_ir.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ir.c**](syn__ir_8c.md)

[Go to the source code of this file](syn__ir_8c_source.md)

_Infrared (IR) Remote Control Protocol Engine implementation._ 

* `#include "../util/syn_assert.h"`
* `#include "syn_ir.h"`
* `#include <string.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_IR\_ProtoDesc**](structSYN__IR__ProtoDesc.md) <br>_Internal infrared protocol timing descriptor._  |








## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const [**SYN\_IR\_ProtoDesc**](structSYN__IR__ProtoDesc.md) | [**proto\_table**](#variable-proto_table)  <br> |














## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ir\_decode\_pulse**](#function-syn_ir_decode_pulse) ([**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md) \* decoder, uint16\_t duration\_us, [**bool**](syn__defs_8h.md#enum-bool) is\_mark, [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame\_out) <br>_Process a single pulse duration (mark or space) in microseconds._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ir\_decode\_timeout**](#function-syn_ir_decode_timeout) ([**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md) \* decoder, [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame\_out) <br>_Signal a gap/timeout (&gt;10ms silence) to finalize protocols._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ir\_decoder\_init**](#function-syn_ir_decoder_init) ([**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md) \* decoder) <br>_Initialize or reset an IR decoder instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ir\_encode\_frame**](#function-syn_ir_encode_frame) (const [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame, [**SYN\_IR\_Pulse**](structSYN__IR__Pulse.md) \* pulse\_buf, size\_t buf\_len, size\_t \* count\_out) <br>_Encode a frame into a sequence of pulse timing pairs._  |
|  const char \* | [**syn\_ir\_protocol\_name**](#function-syn_ir_protocol_name) ([**SYN\_IR\_Protocol**](syn__ir_8h.md#enum-syn_ir_protocol) proto) <br>_Get human-readable protocol string name._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ir\_decode\_idle\_space**](#function-ir_decode_idle_space) ([**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md) \* decoder, uint16\_t mark\_us, uint16\_t space\_us, [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame\_out) <br>_Detect protocol preamble, repeat code, or leaderless start from idle mark/space pair._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ir\_decode\_manchester**](#function-ir_decode_manchester) ([**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md) \* decoder, uint16\_t duration\_us, [**bool**](syn__defs_8h.md#enum-bool) is\_mark, [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame\_out) <br>_Quantize pulse duration into Manchester half-bit units and advance RC5/RC6 decoding._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ir\_decode\_pdm\_space**](#function-ir_decode_pdm_space) ([**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md) \* decoder, const [**SYN\_IR\_ProtoDesc**](structSYN__IR__ProtoDesc.md) \* desc, uint16\_t mark\_us, uint16\_t space\_us, [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame\_out) <br>_Decode one mark/space pair in pulse-distance modulation (PDM) data state._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ir\_decode\_sony\_pulse**](#function-ir_decode_sony_pulse) ([**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md) \* decoder, const [**SYN\_IR\_ProtoDesc**](structSYN__IR__ProtoDesc.md) \* desc, uint16\_t duration\_us, [**bool**](syn__defs_8h.md#enum-bool) is\_mark, [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame\_out) <br>_Decode one mark or space pulse in Sony SIRC pulse-width modulation mode._  |
|  void | [**ir\_emit\_pulse**](#function-ir_emit_pulse) ([**SYN\_IR\_Pulse**](structSYN__IR__Pulse.md) \* buf, size\_t \* idx, uint16\_t dur\_us, [**bool**](syn__defs_8h.md#enum-bool) is\_mark) <br>_Append or coalesce a mark/space pulse entry in the output pulse buffer._  |
|  void | [**ir\_encode\_manchester**](#function-ir_encode_manchester) (const [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame, const [**SYN\_IR\_ProtoDesc**](structSYN__IR__ProtoDesc.md) \* desc, uint64\_t raw\_bits, [**SYN\_IR\_Pulse**](structSYN__IR__Pulse.md) \* pulse\_buf, size\_t \* idx) <br>_Encode Manchester (RC5/RC6) bit pattern into mark/space pulses._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ir\_finish\_frame**](#function-ir_finish_frame) ([**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md) \* decoder, [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame\_out) <br>_Unpack completed bit buffer, cache repeat frame, and reset decoder state._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ir\_is\_nec\_family**](#function-ir_is_nec_family) ([**SYN\_IR\_Protocol**](syn__ir_8h.md#enum-syn_ir_protocol) proto) <br>_Check whether protocol uses NEC-compatible 9ms+2.25ms repeat pulses._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ir\_manchester\_push\_half**](#function-ir_manchester_push_half) ([**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md) \* decoder, [**bool**](syn__defs_8h.md#enum-bool) is\_mark, [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame\_out) <br>_Feed one half-bit level into the Manchester (RC5/RC6) decoder state machine._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ir\_manchester\_push\_quantum**](#function-ir_manchester_push_quantum) ([**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md) \* decoder, [**bool**](syn__defs_8h.md#enum-bool) is\_mark, [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame\_out) <br>_Feed one base timing quantum into the Manchester decoder, coalescing RC6 2T trailer bits._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ir\_match\_leader**](#function-ir_match_leader) ([**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md) \* decoder, uint16\_t mark\_us, uint16\_t space\_us) <br>_Match leader mark/space against protocol descriptor table._  |
|  uint64\_t | [**ir\_pack\_raw\_bits**](#function-ir_pack_raw_bits) (const [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame) <br>_Pack address, command, and protocol-specific control bits into a 64-bit word._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ir\_try\_nec\_repeat**](#function-ir_try_nec_repeat) (const [**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md) \* decoder, uint16\_t mark\_us, uint16\_t space\_us, [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame\_out) <br>_Check whether mark/space matches an NEC-family repeat frame._  |
|  void | [**reset\_decoder\_state**](#function-reset_decoder_state) ([**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md) \* decoder) <br>_Reset IR decoder state machine._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**timing\_match**](#function-timing_match) (uint16\_t actual, uint16\_t expected, uint16\_t tolerance) <br>_Check if measured pulse timing matches expected duration within tolerance._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**unpack\_frame**](#function-unpack_frame) (const [**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md) \* decoder, [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame\_out) <br>_Unpack raw 64-bit frame buffer into decoded_ [_**SYN\_IR\_Frame**_](structSYN__IR__Frame.md) _fields._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**unpack\_nec\_family**](#function-unpack_nec_family) (uint8\_t bit\_idx, uint64\_t bits, [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame\_out) <br>_Unpack 32-bit NEC, NEC-Extended, or Apple IR bit stream into a decoded frame._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**unpack\_other\_pdm**](#function-unpack_other_pdm) ([**SYN\_IR\_Protocol**](syn__ir_8h.md#enum-syn_ir_protocol) proto, uint8\_t bit\_idx, uint64\_t bits, [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame\_out) <br>_Unpack Samsung, Kaseikyo, or Denon PDM bit stream into a decoded frame._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**unpack\_rc5**](#function-unpack_rc5) (uint8\_t bit\_idx, uint64\_t bits, [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame\_out) <br>_Unpack 14-bit Philips RC5 bit stream into a decoded frame._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**unpack\_rc6**](#function-unpack_rc6) (uint8\_t bit\_idx, uint64\_t bits, [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame\_out) <br>_Unpack 21-bit Philips RC6 Mode 0 bit stream into a decoded frame._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**unpack\_sony\_family**](#function-unpack_sony_family) (uint8\_t bit\_idx, uint64\_t bits, [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame\_out) <br>_Unpack 12-bit, 15-bit, or 20-bit Sony SIRC bit stream into a decoded frame._  |


























## Public Static Attributes Documentation




### variable proto\_table 

```C++
const SYN_IR_ProtoDesc proto_table[SYN_IR_PROTO_COUNT];
```



Protocol timing lookup table. 


        

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
## Public Static Functions Documentation




### function ir\_decode\_idle\_space 

_Detect protocol preamble, repeat code, or leaderless start from idle mark/space pair._ 
```C++
static bool ir_decode_idle_space (
    SYN_IR_Decoder * decoder,
    uint16_t mark_us,
    uint16_t space_us,
    SYN_IR_Frame * frame_out
) 
```





**Parameters:**


* `decoder` IR decoder instance. 
* `mark_us` Preceding mark duration in microseconds. 
* `space_us` Measured space duration in microseconds. 
* `frame_out` Output decoded IR frame on repeat match or complete frame. 



**Returns:**

true if a frame (such as NEC repeat) is ready immediately, false otherwise. 





        

<hr>



### function ir\_decode\_manchester 

_Quantize pulse duration into Manchester half-bit units and advance RC5/RC6 decoding._ 
```C++
static bool ir_decode_manchester (
    SYN_IR_Decoder * decoder,
    uint16_t duration_us,
    bool is_mark,
    SYN_IR_Frame * frame_out
) 
```





**Parameters:**


* `decoder` IR decoder instance. 
* `duration_us` Measured pulse or space duration in microseconds. 
* `is_mark` true for mark pulse, false for space. 
* `frame_out` Output decoded IR frame when complete. 



**Returns:**

true if a complete frame was decoded, false otherwise. 





        

<hr>



### function ir\_decode\_pdm\_space 

_Decode one mark/space pair in pulse-distance modulation (PDM) data state._ 
```C++
static bool ir_decode_pdm_space (
    SYN_IR_Decoder * decoder,
    const SYN_IR_ProtoDesc * desc,
    uint16_t mark_us,
    uint16_t space_us,
    SYN_IR_Frame * frame_out
) 
```





**Parameters:**


* `decoder` IR decoder instance. 
* `desc` Protocol timing descriptor. 
* `mark_us` Preceding mark duration in microseconds. 
* `space_us` Measured space duration in microseconds. 
* `frame_out` Output decoded IR frame when complete. 



**Returns:**

true if a complete frame was decoded, false otherwise. 





        

<hr>



### function ir\_decode\_sony\_pulse 

_Decode one mark or space pulse in Sony SIRC pulse-width modulation mode._ 
```C++
static bool ir_decode_sony_pulse (
    SYN_IR_Decoder * decoder,
    const SYN_IR_ProtoDesc * desc,
    uint16_t duration_us,
    bool is_mark,
    SYN_IR_Frame * frame_out
) 
```





**Parameters:**


* `decoder` IR decoder instance. 
* `desc` Protocol timing descriptor. 
* `duration_us` Measured pulse duration in microseconds. 
* `is_mark` true for mark pulse, false for space. 
* `frame_out` Output decoded IR frame when complete. 



**Returns:**

true if a complete frame was decoded, false otherwise. 





        

<hr>



### function ir\_emit\_pulse 

_Append or coalesce a mark/space pulse entry in the output pulse buffer._ 
```C++
static void ir_emit_pulse (
    SYN_IR_Pulse * buf,
    size_t * idx,
    uint16_t dur_us,
    bool is_mark
) 
```





**Parameters:**


* `buf` Output pulse array. 
* `idx` Current pulse write index, updated in-place. 
* `dur_us` Pulse duration in microseconds. 
* `is_mark` true for carrier mark, false for space. 




        

<hr>



### function ir\_encode\_manchester 

_Encode Manchester (RC5/RC6) bit pattern into mark/space pulses._ 
```C++
static void ir_encode_manchester (
    const SYN_IR_Frame * frame,
    const SYN_IR_ProtoDesc * desc,
    uint64_t raw_bits,
    SYN_IR_Pulse * pulse_buf,
    size_t * idx
) 
```





**Parameters:**


* `frame` Input IR frame descriptor. 
* `desc` Protocol timing descriptor. 
* `raw_bits` Packed MSB-first raw bit word. 
* `pulse_buf` Output pulse array. 
* `idx` Current pulse write index, updated in-place. 




        

<hr>



### function ir\_finish\_frame 

_Unpack completed bit buffer, cache repeat frame, and reset decoder state._ 
```C++
static bool ir_finish_frame (
    SYN_IR_Decoder * decoder,
    SYN_IR_Frame * frame_out
) 
```





**Parameters:**


* `decoder` IR decoder instance. 
* `frame_out` Output decoded IR frame. 



**Returns:**

true if a valid frame was unpacked, false otherwise. 





        

<hr>



### function ir\_is\_nec\_family 

_Check whether protocol uses NEC-compatible 9ms+2.25ms repeat pulses._ 
```C++
static bool ir_is_nec_family (
    SYN_IR_Protocol proto
) 
```





**Parameters:**


* `proto` IR protocol identifier. 



**Returns:**

true if NEC, NEC\_EXTENDED, or APPLE; false otherwise. 





        

<hr>



### function ir\_manchester\_push\_half 

_Feed one half-bit level into the Manchester (RC5/RC6) decoder state machine._ 
```C++
static bool ir_manchester_push_half (
    SYN_IR_Decoder * decoder,
    bool is_mark,
    SYN_IR_Frame * frame_out
) 
```





**Parameters:**


* `decoder` IR decoder instance. 
* `is_mark` true if half-bit is carrier mark, false if space. 
* `frame_out` Output decoded IR frame when final bit completes. 



**Returns:**

true if a complete frame was decoded, false otherwise. 





        

<hr>



### function ir\_manchester\_push\_quantum 

_Feed one base timing quantum into the Manchester decoder, coalescing RC6 2T trailer bits._ 
```C++
static bool ir_manchester_push_quantum (
    SYN_IR_Decoder * decoder,
    bool is_mark,
    SYN_IR_Frame * frame_out
) 
```





**Parameters:**


* `decoder` IR decoder instance. 
* `is_mark` true if quantum is carrier mark, false if space. 
* `frame_out` Output decoded IR frame when final bit completes. 



**Returns:**

true if a complete frame was decoded, false otherwise. 





        

<hr>



### function ir\_match\_leader 

_Match leader mark/space against protocol descriptor table._ 
```C++
static bool ir_match_leader (
    SYN_IR_Decoder * decoder,
    uint16_t mark_us,
    uint16_t space_us
) 
```





**Parameters:**


* `decoder` IR decoder instance. 
* `mark_us` Preceding mark duration in microseconds. 
* `space_us` Current space duration in microseconds. 



**Returns:**

true if a protocol leader matched and transitioned state, false otherwise. 





        

<hr>



### function ir\_pack\_raw\_bits 

_Pack address, command, and protocol-specific control bits into a 64-bit word._ 
```C++
static uint64_t ir_pack_raw_bits (
    const SYN_IR_Frame * frame
) 
```





**Parameters:**


* `frame` Input IR frame descriptor. 



**Returns:**

Packed raw bit pattern for protocol transmission. 





        

<hr>



### function ir\_try\_nec\_repeat 

_Check whether mark/space matches an NEC-family repeat frame._ 
```C++
static bool ir_try_nec_repeat (
    const SYN_IR_Decoder * decoder,
    uint16_t mark_us,
    uint16_t space_us,
    SYN_IR_Frame * frame_out
) 
```





**Parameters:**


* `decoder` IR decoder instance. 
* `mark_us` Preceding mark duration in microseconds. 
* `space_us` Current space duration in microseconds. 
* `frame_out` Output decoded IR frame (populated on repeat). 



**Returns:**

true if repeat frame emitted, false otherwise. 





        

<hr>



### function reset\_decoder\_state 

_Reset IR decoder state machine._ 
```C++
static void reset_decoder_state (
    SYN_IR_Decoder * decoder
) 
```





**Parameters:**


* `decoder` Decoder instance pointer. 




        

<hr>



### function timing\_match 

_Check if measured pulse timing matches expected duration within tolerance._ 
```C++
static bool timing_match (
    uint16_t actual,
    uint16_t expected,
    uint16_t tolerance
) 
```





**Parameters:**


* `actual` Measured pulse duration in us. 
* `expected` Nominal target duration in us. 
* `tolerance` Allowable +/- error window in us. 



**Returns:**

true if within tolerance, false otherwise. 





        

<hr>



### function unpack\_frame 

_Unpack raw 64-bit frame buffer into decoded_ [_**SYN\_IR\_Frame**_](structSYN__IR__Frame.md) _fields._
```C++
static bool unpack_frame (
    const SYN_IR_Decoder * decoder,
    SYN_IR_Frame * frame_out
) 
```





**Parameters:**


* `decoder` Pointer to IR decoder instance. 
* `frame_out` Pointer to output frame structure. 



**Returns:**

True on successful frame unpacking, false otherwise. 





        

<hr>



### function unpack\_nec\_family 

_Unpack 32-bit NEC, NEC-Extended, or Apple IR bit stream into a decoded frame._ 
```C++
static bool unpack_nec_family (
    uint8_t bit_idx,
    uint64_t bits,
    SYN_IR_Frame * frame_out
) 
```





**Parameters:**


* `bit_idx` Total number of decoded bits. 
* `bits` Raw 32-bit LSB-first received bit pattern. 
* `frame_out` Output decoded IR frame. 



**Returns:**

true if bit count and checksum/signature are valid, false otherwise. 





        

<hr>



### function unpack\_other\_pdm 

_Unpack Samsung, Kaseikyo, or Denon PDM bit stream into a decoded frame._ 
```C++
static bool unpack_other_pdm (
    SYN_IR_Protocol proto,
    uint8_t bit_idx,
    uint64_t bits,
    SYN_IR_Frame * frame_out
) 
```





**Parameters:**


* `proto` Active IR protocol identifier. 
* `bit_idx` Total number of decoded bits. 
* `bits` Raw LSB-first received bit pattern. 
* `frame_out` Output decoded IR frame. 



**Returns:**

true if bit count and protocol checks pass, false otherwise. 





        

<hr>



### function unpack\_rc5 

_Unpack 14-bit Philips RC5 bit stream into a decoded frame._ 
```C++
static bool unpack_rc5 (
    uint8_t bit_idx,
    uint64_t bits,
    SYN_IR_Frame * frame_out
) 
```





**Parameters:**


* `bit_idx` Total number of decoded bits. 
* `bits` Raw MSB-first received bit pattern. 
* `frame_out` Output decoded IR frame. 



**Returns:**

true if valid 14-bit RC5 frame, false otherwise. 





        

<hr>



### function unpack\_rc6 

_Unpack 21-bit Philips RC6 Mode 0 bit stream into a decoded frame._ 
```C++
static bool unpack_rc6 (
    uint8_t bit_idx,
    uint64_t bits,
    SYN_IR_Frame * frame_out
) 
```





**Parameters:**


* `bit_idx` Total number of decoded bits. 
* `bits` Raw MSB-first received bit pattern. 
* `frame_out` Output decoded IR frame. 



**Returns:**

true if valid 21-bit RC6 Mode 0 frame (start bit == 1, mode == 0), false otherwise. 





        

<hr>



### function unpack\_sony\_family 

_Unpack 12-bit, 15-bit, or 20-bit Sony SIRC bit stream into a decoded frame._ 
```C++
static bool unpack_sony_family (
    uint8_t bit_idx,
    uint64_t bits,
    SYN_IR_Frame * frame_out
) 
```





**Parameters:**


* `bit_idx` Total number of decoded bits. 
* `bits` Raw LSB-first received bit pattern. 
* `frame_out` Output decoded IR frame. 



**Returns:**

true if bit count matches a valid Sony variant, false otherwise. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ir.c`

