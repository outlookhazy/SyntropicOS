

# Struct SYN\_TCP



[**ClassList**](annotated.md) **>** [**SYN\_TCP**](structSYN__TCP.md)



_Top-level TCP stack instance container._ 

* `#include <syn_tcp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_TcpConn**](structSYN__TcpConn.md) | [**conns**](#variable-conns)  <br> |
|  [**SYN\_ETH**](structSYN__ETH.md) \* | [**eth**](#variable-eth)  <br> |
|  uint16\_t | [**listen\_port**](#variable-listen_port)  <br> |
|  uint16\_t | [**listen\_ports**](#variable-listen_ports)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**listening**](#variable-listening)  <br> |












































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



### variable listen\_port 

```C++
uint16_t SYN_TCP::listen_port;
```



Most recently configured passive listener port. 


        

<hr>



### variable listen\_ports 

```C++
uint16_t SYN_TCP::listen_ports[SYN_TCP_MAX_CONNS];
```



Per-slot passive listener ports. 


        

<hr>



### variable listening 

```C++
bool SYN_TCP::listening;
```



True if passive listener is active. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_tcp.h`

