

# Struct SYN\_UDP



[**ClassList**](annotated.md) **>** [**SYN\_UDP**](structSYN__UDP.md)



_UDP protocol stack container instance._ 

* `#include <syn_udp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_ETH**](structSYN__ETH.md) \* | [**eth**](#variable-eth)  <br> |
|  [**SYN\_UdpSocket**](structSYN__UdpSocket.md) | [**sockets**](#variable-sockets)  <br> |












































## Public Attributes Documentation




### variable eth 

```C++
SYN_ETH* SYN_UDP::eth;
```



Associated Ethernet interface instance. 


        

<hr>



### variable sockets 

```C++
SYN_UdpSocket SYN_UDP::sockets[SYN_UDP_MAX_SOCKETS];
```



Array of managed UDP sockets. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_udp.h`

