

# Struct SYN\_IR\_Frame



[**ClassList**](annotated.md) **>** [**SYN\_IR\_Frame**](structSYN__IR__Frame.md)



_Decoded IR Remote Control Frame._ 

* `#include <syn_ir.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**address**](#variable-address)  <br> |
|  uint8\_t | [**bit\_count**](#variable-bit_count)  <br> |
|  uint8\_t | [**carrier\_khz**](#variable-carrier_khz)  <br> |
|  uint32\_t | [**command**](#variable-command)  <br> |
|  uint16\_t | [**flags**](#variable-flags)  <br> |
|  [**SYN\_IR\_Protocol**](syn__ir_8h.md#enum-syn_ir_protocol) | [**protocol**](#variable-protocol)  <br> |












































## Public Attributes Documentation




### variable address 

```C++
uint32_t SYN_IR_Frame::address;
```



Decoded address field 


        

<hr>



### variable bit\_count 

```C++
uint8_t SYN_IR_Frame::bit_count;
```



Total bit count 


        

<hr>



### variable carrier\_khz 

```C++
uint8_t SYN_IR_Frame::carrier_khz;
```



Nominal carrier frequency in kHz (36, 38, 40) 


        

<hr>



### variable command 

```C++
uint32_t SYN_IR_Frame::command;
```



Decoded command field 


        

<hr>



### variable flags 

```C++
uint16_t SYN_IR_Frame::flags;
```



Frame flags (SYN\_IR\_FLAG\_REPEAT, SYN\_IR\_FLAG\_TOGGLE) 


        

<hr>



### variable protocol 

```C++
SYN_IR_Protocol SYN_IR_Frame::protocol;
```



Protocol type 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ir.h`

