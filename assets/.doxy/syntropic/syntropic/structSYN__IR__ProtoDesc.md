

# Struct SYN\_IR\_ProtoDesc



[**ClassList**](annotated.md) **>** [**SYN\_IR\_ProtoDesc**](structSYN__IR__ProtoDesc.md)



_Internal infrared protocol timing descriptor._ 






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**addr\_bits**](#variable-addr_bits)  <br> |
|  uint16\_t | [**bit\_mark\_us**](#variable-bit_mark_us)  <br> |
|  uint8\_t | [**carrier\_khz**](#variable-carrier_khz)  <br> |
|  uint8\_t | [**cmd\_bits**](#variable-cmd_bits)  <br> |
|  [**SYN\_IR\_EncodingType**](syn__ir_8h.md#enum-syn_ir_encodingtype) | [**encoding**](#variable-encoding)  <br> |
|  uint16\_t | [**half\_bit\_us**](#variable-half_bit_us)  <br> |
|  uint16\_t | [**leader\_mark\_us**](#variable-leader_mark_us)  <br> |
|  uint16\_t | [**leader\_space\_us**](#variable-leader_space_us)  <br> |
|  const char \* | [**name**](#variable-name)  <br> |
|  uint16\_t | [**one\_mark\_us**](#variable-one_mark_us)  <br> |
|  uint16\_t | [**one\_space\_us**](#variable-one_space_us)  <br> |
|  [**SYN\_IR\_Protocol**](syn__ir_8h.md#enum-syn_ir_protocol) | [**proto**](#variable-proto)  <br> |
|  uint16\_t | [**repeat\_space\_us**](#variable-repeat_space_us)  <br> |
|  uint16\_t | [**tolerance\_us**](#variable-tolerance_us)  <br> |
|  uint8\_t | [**total\_bits**](#variable-total_bits)  <br> |
|  uint16\_t | [**zero\_space\_us**](#variable-zero_space_us)  <br> |












































## Public Attributes Documentation




### variable addr\_bits 

```C++
uint8_t SYN_IR_ProtoDesc::addr_bits;
```



Number of address bits 


        

<hr>



### variable bit\_mark\_us 

```C++
uint16_t SYN_IR_ProtoDesc::bit_mark_us;
```



PDM mark duration or PWM '0' mark 


        

<hr>



### variable carrier\_khz 

```C++
uint8_t SYN_IR_ProtoDesc::carrier_khz;
```



Modulation carrier frequency in kHz 


        

<hr>



### variable cmd\_bits 

```C++
uint8_t SYN_IR_ProtoDesc::cmd_bits;
```



Number of command bits 


        

<hr>



### variable encoding 

```C++
SYN_IR_EncodingType SYN_IR_ProtoDesc::encoding;
```



Pulse encoding type (PDM, PWM, Manchester) 


        

<hr>



### variable half\_bit\_us 

```C++
uint16_t SYN_IR_ProtoDesc::half_bit_us;
```



Manchester half-bit duration 


        

<hr>



### variable leader\_mark\_us 

```C++
uint16_t SYN_IR_ProtoDesc::leader_mark_us;
```



Preamble header mark duration in us 


        

<hr>



### variable leader\_space\_us 

```C++
uint16_t SYN_IR_ProtoDesc::leader_space_us;
```



Preamble header space duration in us 


        

<hr>



### variable name 

```C++
const char* SYN_IR_ProtoDesc::name;
```



Human readable protocol name 


        

<hr>



### variable one\_mark\_us 

```C++
uint16_t SYN_IR_ProtoDesc::one_mark_us;
```



PWM '1' mark 


        

<hr>



### variable one\_space\_us 

```C++
uint16_t SYN_IR_ProtoDesc::one_space_us;
```



PDM '1' space 


        

<hr>



### variable proto 

```C++
SYN_IR_Protocol SYN_IR_ProtoDesc::proto;
```



Protocol enum identifier 


        

<hr>



### variable repeat\_space\_us 

```C++
uint16_t SYN_IR_ProtoDesc::repeat_space_us;
```



Repeat header space duration in us 


        

<hr>



### variable tolerance\_us 

```C++
uint16_t SYN_IR_ProtoDesc::tolerance_us;
```



Measurement tolerance window in us 


        

<hr>



### variable total\_bits 

```C++
uint8_t SYN_IR_ProtoDesc::total_bits;
```



Total bits in frame 


        

<hr>



### variable zero\_space\_us 

```C++
uint16_t SYN_IR_ProtoDesc::zero_space_us;
```



PDM '0' space or PWM fixed space 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ir.c`

