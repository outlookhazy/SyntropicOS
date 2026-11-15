

# Struct SYN\_UDP



[**ClassList**](annotated.md) **>** [**SYN\_UDP**](structSYN__UDP.md)



_UDP protocol stack container instance._ 

* `#include <syn_udp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_ETH**](structSYN__ETH.md) \* | [**eth**](#variable-eth)  <br> |
|  uint16\_t | [**next\_ephemeral\_port**](#variable-next_ephemeral_port)  <br> |
|  [**SYN\_UdpSocket**](structSYN__UdpSocket.md) | [**sockets**](#variable-sockets)  <br> |












































## Public Attributes Documentation




### variable eth 

```C++
SYN_ETH* SYN_UDP::eth;
```



Associated Ethernet interface instance. 


        

<hr>



### variable next\_ephemeral\_port 

```C++
uint16_t SYN_UDP::next_ephemeral_port;
```



Ephemeral local port cursor (49152..65535). 


        

<hr>



### variable sockets 

```C++
SYN_UdpSocket SYN_UDP::sockets[SYN_UDP_MAX_SOCKETS];
```



Array of managed UDP sockets. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_udp.h`

