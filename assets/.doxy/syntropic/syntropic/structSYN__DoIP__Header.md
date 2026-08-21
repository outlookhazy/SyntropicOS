

# Struct SYN\_DoIP\_Header



[**ClassList**](annotated.md) **>** [**SYN\_DoIP\_Header**](structSYN__DoIP__Header.md)



_ISO 13400-2 DoIP Header Structure (8 bytes)_ 

* `#include <syn_doip.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**inverse\_protocol\_version**](#variable-inverse_protocol_version)  <br> |
|  uint32\_t | [**payload\_length**](#variable-payload_length)  <br> |
|  uint16\_t | [**payload\_type**](#variable-payload_type)  <br> |
|  uint8\_t | [**protocol\_version**](#variable-protocol_version)  <br> |












































## Public Attributes Documentation




### variable inverse\_protocol\_version 

```C++
uint8_t SYN_DoIP_Header::inverse_protocol_version;
```



Must be ~protocol\_version (0xFD) 


        

<hr>



### variable payload\_length 

```C++
uint32_t SYN_DoIP_Header::payload_length;
```



Big-endian 4-byte payload length 


        

<hr>



### variable payload\_type 

```C++
uint16_t SYN_DoIP_Header::payload_type;
```



Big-endian 2-byte payload type 


        

<hr>



### variable protocol\_version 

```C++
uint8_t SYN_DoIP_Header::protocol_version;
```



Must be 0x02 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_doip.h`

