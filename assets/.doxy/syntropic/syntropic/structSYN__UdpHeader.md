

# Struct SYN\_UdpHeader



[**ClassList**](annotated.md) **>** [**SYN\_UdpHeader**](structSYN__UdpHeader.md)



_UDP packet header structure (8 bytes)._ 

* `#include <syn_udp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**checksum**](#variable-checksum)  <br> |
|  uint16\_t | [**dst\_port**](#variable-dst_port)  <br> |
|  uint16\_t | [**length**](#variable-length)  <br> |
|  uint16\_t | [**src\_port**](#variable-src_port)  <br> |












































## Public Attributes Documentation




### variable checksum 

```C++
uint16_t SYN_UdpHeader::checksum;
```



Checksum over pseudo-header and payload. 


        

<hr>



### variable dst\_port 

```C++
uint16_t SYN_UdpHeader::dst_port;
```



Destination port number. 


        

<hr>



### variable length 

```C++
uint16_t SYN_UdpHeader::length;
```



Length of UDP header + payload in bytes. 


        

<hr>



### variable src\_port 

```C++
uint16_t SYN_UdpHeader::src_port;
```



Source port number. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_udp.h`

