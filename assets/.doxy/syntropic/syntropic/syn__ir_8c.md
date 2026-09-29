

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
|  void | [**reset\_decoder\_state**](#function-reset_decoder_state) ([**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md) \* decoder) <br>_Reset IR decoder state machine._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**timing\_match**](#function-timing_match) (uint16\_t actual, uint16\_t expected, uint16\_t tolerance) <br>_Check if measured pulse timing matches expected duration within tolerance._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**unpack\_frame**](#function-unpack_frame) (const [**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md) \* decoder, [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) \* frame\_out) <br>_Unpack raw 64-bit frame buffer into decoded_ [_**SYN\_IR\_Frame**_](structSYN__IR__Frame.md) _fields._ |


























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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ir.c`

