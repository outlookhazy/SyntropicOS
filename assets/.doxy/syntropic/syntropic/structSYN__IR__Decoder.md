

# Struct SYN\_IR\_Decoder



[**ClassList**](annotated.md) **>** [**SYN\_IR\_Decoder**](structSYN__IR__Decoder.md)



_Non-blocking IR Decoder Handle._ 

* `#include <syn_ir.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_IR\_Protocol**](syn__ir_8h.md#enum-syn_ir_protocol) | [**active\_proto**](#variable-active_proto)  <br> |
|  uint8\_t | [**bit\_idx**](#variable-bit_idx)  <br> |
|  uint64\_t | [**bits**](#variable-bits)  <br> |
|  uint8\_t | [**expected\_bits**](#variable-expected_bits)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**have\_last**](#variable-have_last)  <br> |
|  [**SYN\_IR\_Frame**](structSYN__IR__Frame.md) | [**last\_frame**](#variable-last_frame)  <br> |
|  uint16\_t | [**last\_mark\_us**](#variable-last_mark_us)  <br> |
|  uint16\_t | [**last\_space\_us**](#variable-last_space_us)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**manchester\_phase**](#variable-manchester_phase)  <br> |
|  [**SYN\_IR\_FsmState**](syn__ir_8h.md#enum-syn_ir_fsmstate) | [**state**](#variable-state)  <br> |












































## Public Attributes Documentation




### variable active\_proto 

```C++
SYN_IR_Protocol SYN_IR_Decoder::active_proto;
```



Protocol currently being decoded 


        

<hr>



### variable bit\_idx 

```C++
uint8_t SYN_IR_Decoder::bit_idx;
```



Number of bits accumulated 


        

<hr>



### variable bits 

```C++
uint64_t SYN_IR_Decoder::bits;
```



Bit shift register 


        

<hr>



### variable expected\_bits 

```C++
uint8_t SYN_IR_Decoder::expected_bits;
```



Total expected bits for active protocol 


        

<hr>



### variable have\_last 

```C++
bool SYN_IR_Decoder::have_last;
```



True if a frame has been decoded 


        

<hr>



### variable last\_frame 

```C++
SYN_IR_Frame SYN_IR_Decoder::last_frame;
```



Last successfully decoded frame 


        

<hr>



### variable last\_mark\_us 

```C++
uint16_t SYN_IR_Decoder::last_mark_us;
```



Last mark pulse duration 


        

<hr>



### variable last\_space\_us 

```C++
uint16_t SYN_IR_Decoder::last_space_us;
```



Last space pulse duration 


        

<hr>



### variable manchester\_phase 

```C++
bool SYN_IR_Decoder::manchester_phase;
```



Manchester phase tracking 


        

<hr>



### variable state 

```C++
SYN_IR_FsmState SYN_IR_Decoder::state;
```



Current FSM state 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ir.h`

