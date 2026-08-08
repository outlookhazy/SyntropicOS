

# Struct SYN\_BACnet\_MSTP\_Frame



[**ClassList**](annotated.md) **>** [**SYN\_BACnet\_MSTP\_Frame**](structSYN__BACnet__MSTP__Frame.md)



_Decoded BACnet MS/TP Frame Structure._ 

* `#include <syn_bacnet.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**data\_len**](#variable-data_len)  <br> |
|  uint8\_t | [**destination\_mac**](#variable-destination_mac)  <br> |
|  uint8\_t | [**frame\_type**](#variable-frame_type)  <br> |
|  uint8\_t | [**payload**](#variable-payload)  <br> |
|  uint8\_t | [**source\_mac**](#variable-source_mac)  <br> |












































## Public Attributes Documentation




### variable data\_len 

```C++
uint16_t SYN_BACnet_MSTP_Frame::data_len;
```



Payload length in bytes 


        

<hr>



### variable destination\_mac 

```C++
uint8_t SYN_BACnet_MSTP_Frame::destination_mac;
```



Destination MAC address (0..254, 255=Broadcast) 


        

<hr>



### variable frame\_type 

```C++
uint8_t SYN_BACnet_MSTP_Frame::frame_type;
```



MS/TP Frame Type (0..7) 


        

<hr>



### variable payload 

```C++
uint8_t SYN_BACnet_MSTP_Frame::payload[501];
```



Payload buffer 


        

<hr>



### variable source\_mac 

```C++
uint8_t SYN_BACnet_MSTP_Frame::source_mac;
```



Source MAC address (0..254) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_bacnet.h`

