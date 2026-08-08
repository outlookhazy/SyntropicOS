

# Struct SYN\_TCP



[**ClassList**](annotated.md) **>** [**SYN\_TCP**](structSYN__TCP.md)



_Top-level TCP stack instance container._ 

* `#include <syn_tcp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_TcpConn**](structSYN__TcpConn.md) | [**conns**](#variable-conns)  <br> |
|  [**SYN\_ETH**](structSYN__ETH.md) \* | [**eth**](#variable-eth)  <br> |












































## Public Attributes Documentation




### variable conns 

```C++
SYN_TcpConn SYN_TCP::conns[SYN_TCP_MAX_CONNS];
```



Array of managed TCP connections. 


        

<hr>



### variable eth 

```C++
SYN_ETH* SYN_TCP::eth;
```



Associated Ethernet interface instance. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_tcp.h`

