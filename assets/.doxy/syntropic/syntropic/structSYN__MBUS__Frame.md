

# Struct SYN\_MBUS\_Frame



[**ClassList**](annotated.md) **>** [**SYN\_MBUS\_Frame**](structSYN__MBUS__Frame.md)



_M-Bus decoded frame structure._ 

* `#include <syn_mbus.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**a\_field**](#variable-a_field)  <br> |
|  uint8\_t | [**c\_field**](#variable-c_field)  <br> |
|  uint8\_t | [**checksum**](#variable-checksum)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**checksum\_valid**](#variable-checksum_valid)  <br> |
|  uint8\_t | [**ci\_field**](#variable-ci_field)  <br> |
|  uint8\_t | [**payload**](#variable-payload)  <br> |
|  uint8\_t | [**payload\_len**](#variable-payload_len)  <br> |
|  [**SYN\_MBUS\_FrameType**](syn__mbus_8h.md#enum-syn_mbus_frametype) | [**type**](#variable-type)  <br> |












































## Public Attributes Documentation




### variable a\_field 

```C++
uint8_t SYN_MBUS_Frame::a_field;
```



Address field byte (0-250, 0xFE, 0xFF) 


        

<hr>



### variable c\_field 

```C++
uint8_t SYN_MBUS_Frame::c_field;
```



Control field byte 


        

<hr>



### variable checksum 

```C++
uint8_t SYN_MBUS_Frame::checksum;
```



Received checksum byte 


        

<hr>



### variable checksum\_valid 

```C++
bool SYN_MBUS_Frame::checksum_valid;
```



True if calculated checksum matches received 


        

<hr>



### variable ci\_field 

```C++
uint8_t SYN_MBUS_Frame::ci_field;
```



Control Information field (Control & Long frames) 


        

<hr>



### variable payload 

```C++
uint8_t SYN_MBUS_Frame::payload[SYN_MBUS_MAX_PAYLOAD];
```



Payload data buffer 


        

<hr>



### variable payload\_len 

```C++
uint8_t SYN_MBUS_Frame::payload_len;
```



Length of payload data 


        

<hr>



### variable type 

```C++
SYN_MBUS_FrameType SYN_MBUS_Frame::type;
```



Frame format classification 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_mbus.h`

