

# Struct SYN\_MAVLINK\_Parser



[**ClassList**](annotated.md) **>** [**SYN\_MAVLINK\_Parser**](structSYN__MAVLINK__Parser.md)



[More...](#detailed-description)

* `#include <syn_mavlink.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**comp\_id**](#variable-comp_id)  <br> |
|  uint8\_t | [**compat\_flags**](#variable-compat_flags)  <br> |
|  uint16\_t | [**crc**](#variable-crc)  <br> |
|  uint32\_t | [**crc\_errors**](#variable-crc_errors)  <br> |
|  uint8\_t | [**incompat\_flags**](#variable-incompat_flags)  <br> |
|  [**SYN\_MAVLINK\_Frame**](structSYN__MAVLINK__Frame.md) | [**last\_frame**](#variable-last_frame)  <br> |
|  uint32\_t | [**msg\_id**](#variable-msg_id)  <br> |
|  uint32\_t | [**packets\_received**](#variable-packets_received)  <br> |
|  uint8\_t | [**payload**](#variable-payload)  <br> |
|  uint8\_t | [**payload\_idx**](#variable-payload_idx)  <br> |
|  uint8\_t | [**payload\_len**](#variable-payload_len)  <br> |
|  uint8\_t | [**seq**](#variable-seq)  <br> |
|  uint8\_t | [**state**](#variable-state)  <br> |
|  uint8\_t | [**sys\_id**](#variable-sys_id)  <br> |












































## Detailed Description


MAVLink v2 Streaming Parser Instance. 


    
## Public Attributes Documentation




### variable comp\_id 

```C++
uint8_t SYN_MAVLINK_Parser::comp_id;
```



Received component ID 


        

<hr>



### variable compat\_flags 

```C++
uint8_t SYN_MAVLINK_Parser::compat_flags;
```



Received compatibility flags 


        

<hr>



### variable crc 

```C++
uint16_t SYN_MAVLINK_Parser::crc;
```



Running CRC accumulator 


        

<hr>



### variable crc\_errors 

```C++
uint32_t SYN_MAVLINK_Parser::crc_errors;
```



Count of CRC mismatch errors 


        

<hr>



### variable incompat\_flags 

```C++
uint8_t SYN_MAVLINK_Parser::incompat_flags;
```



Received incompatibility flags 


        

<hr>



### variable last\_frame 

```C++
SYN_MAVLINK_Frame SYN_MAVLINK_Parser::last_frame;
```



Last successfully parsed frame 


        

<hr>



### variable msg\_id 

```C++
uint32_t SYN_MAVLINK_Parser::msg_id;
```



Decoded message ID 


        

<hr>



### variable packets\_received 

```C++
uint32_t SYN_MAVLINK_Parser::packets_received;
```



Total valid packets parsed 


        

<hr>



### variable payload 

```C++
uint8_t SYN_MAVLINK_Parser::payload[SYN_MAVLINK_MAX_PAYLOAD_LEN];
```



Internal payload buffer 


        

<hr>



### variable payload\_idx 

```C++
uint8_t SYN_MAVLINK_Parser::payload_idx;
```



Current payload write index 


        

<hr>



### variable payload\_len 

```C++
uint8_t SYN_MAVLINK_Parser::payload_len;
```



Expected payload length 


        

<hr>



### variable seq 

```C++
uint8_t SYN_MAVLINK_Parser::seq;
```



Received sequence number 


        

<hr>



### variable state 

```C++
uint8_t SYN_MAVLINK_Parser::state;
```



Internal parser state machine step 


        

<hr>



### variable sys\_id 

```C++
uint8_t SYN_MAVLINK_Parser::sys_id;
```



Received system ID 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_mavlink.h`

