

# Struct SYN\_MAVLINK\_Frame



[**ClassList**](annotated.md) **>** [**SYN\_MAVLINK\_Frame**](structSYN__MAVLINK__Frame.md)



[More...](#detailed-description)

* `#include <syn_mavlink.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**comp\_id**](#variable-comp_id)  <br> |
|  uint8\_t | [**compat\_flags**](#variable-compat_flags)  <br> |
|  uint8\_t | [**incompat\_flags**](#variable-incompat_flags)  <br> |
|  uint32\_t | [**msg\_id**](#variable-msg_id)  <br> |
|  uint8\_t | [**payload**](#variable-payload)  <br> |
|  uint8\_t | [**payload\_len**](#variable-payload_len)  <br> |
|  uint8\_t | [**seq**](#variable-seq)  <br> |
|  uint8\_t | [**sys\_id**](#variable-sys_id)  <br> |












































## Detailed Description


Parsed MAVLink v2 Frame. 


    
## Public Attributes Documentation




### variable comp\_id 

```C++
uint8_t SYN_MAVLINK_Frame::comp_id;
```



Sender component ID 


        

<hr>



### variable compat\_flags 

```C++
uint8_t SYN_MAVLINK_Frame::compat_flags;
```



Compatibility flags byte 


        

<hr>



### variable incompat\_flags 

```C++
uint8_t SYN_MAVLINK_Frame::incompat_flags;
```



Incompatibility flags byte 


        

<hr>



### variable msg\_id 

```C++
uint32_t SYN_MAVLINK_Frame::msg_id;
```



24-bit Message ID 


        

<hr>



### variable payload 

```C++
uint8_t SYN_MAVLINK_Frame::payload[SYN_MAVLINK_MAX_PAYLOAD_LEN];
```



Frame payload bytes 


        

<hr>



### variable payload\_len 

```C++
uint8_t SYN_MAVLINK_Frame::payload_len;
```



Payload length in bytes 


        

<hr>



### variable seq 

```C++
uint8_t SYN_MAVLINK_Frame::seq;
```



Sequence number 


        

<hr>



### variable sys\_id 

```C++
uint8_t SYN_MAVLINK_Frame::sys_id;
```



Sender system ID 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_mavlink.h`

