

# Struct SYN\_DLT645\_Frame



[**ClassList**](annotated.md) **>** [**SYN\_DLT645\_Frame**](structSYN__DLT645__Frame.md)



_Decoded DL/T 645 frame representation._ 

* `#include <syn_dlt645.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**address**](#variable-address)  <br> |
|  uint8\_t | [**control**](#variable-control)  <br> |
|  uint32\_t | [**data\_id**](#variable-data_id)  <br> |
|  uint8\_t | [**payload**](#variable-payload)  <br> |
|  uint8\_t | [**payload\_len**](#variable-payload_len)  <br> |
|  [**SYN\_DLT645\_Ver**](syn__dlt645_8h.md#enum-syn_dlt645_ver) | [**version**](#variable-version)  <br> |












































## Public Attributes Documentation




### variable address 

```C++
uint8_t SYN_DLT645_Frame::address[SYN_DLT645_ADDR_LEN];
```



6-byte BCD meter address 
 


        

<hr>



### variable control 

```C++
uint8_t SYN_DLT645_Frame::control;
```



Control code byte 
 


        

<hr>



### variable data\_id 

```C++
uint32_t SYN_DLT645_Frame::data_id;
```



Data Identifier (DI) 
 


        

<hr>



### variable payload 

```C++
uint8_t SYN_DLT645_Frame::payload[64];
```



Raw un-offset data payload bytes 
 


        

<hr>



### variable payload\_len 

```C++
uint8_t SYN_DLT645_Frame::payload_len;
```



Payload length (excluding DI) 
 


        

<hr>



### variable version 

```C++
SYN_DLT645_Ver SYN_DLT645_Frame::version;
```



Protocol version (1997 or 2007) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_dlt645.h`

