

# Struct SYN\_UdpDatagram



[**ClassList**](annotated.md) **>** [**SYN\_UdpDatagram**](structSYN__UdpDatagram.md)



_Queued UDP datagram metadata descriptor preserving packet boundaries._ 

* `#include <syn_udp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**len**](#variable-len)  <br> |
|  uint32\_t | [**src\_ip**](#variable-src_ip)  <br> |
|  uint16\_t | [**src\_port**](#variable-src_port)  <br> |












































## Public Attributes Documentation




### variable len 

```C++
uint16_t SYN_UdpDatagram::len;
```



Datagram payload byte length. 


        

<hr>



### variable src\_ip 

```C++
uint32_t SYN_UdpDatagram::src_ip;
```



Sender IPv4 address. 


        

<hr>



### variable src\_port 

```C++
uint16_t SYN_UdpDatagram::src_port;
```



Sender UDP port. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_udp.h`

