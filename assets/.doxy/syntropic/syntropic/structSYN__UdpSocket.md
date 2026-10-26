

# Struct SYN\_UdpSocket



[**ClassList**](annotated.md) **>** [**SYN\_UdpSocket**](structSYN__UdpSocket.md)



_UDP socket slot control block._ 

* `#include <syn_udp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_Task**](structSYN__Task.md) \* | [**blocked\_task**](#variable-blocked_task)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_bound**](#variable-is_bound)  <br> |
|  uint16\_t | [**local\_port**](#variable-local_port)  <br> |
|  uint32\_t | [**remote\_ip**](#variable-remote_ip)  <br> |
|  uint16\_t | [**remote\_port**](#variable-remote_port)  <br> |
|  uint8\_t | [**rx\_buf**](#variable-rx_buf)  <br> |
|  uint16\_t | [**rx\_len**](#variable-rx_len)  <br> |












































## Public Attributes Documentation




### variable blocked\_task 

```C++
SYN_Task* SYN_UdpSocket::blocked_task;
```



Protothread task sleeping on this UDP socket. 


        

<hr>



### variable is\_bound 

```C++
bool SYN_UdpSocket::is_bound;
```



True if socket is bound to a local port. 


        

<hr>



### variable local\_port 

```C++
uint16_t SYN_UdpSocket::local_port;
```



Bound local port number. 


        

<hr>



### variable remote\_ip 

```C++
uint32_t SYN_UdpSocket::remote_ip;
```



Connected remote IPv4 address. 


        

<hr>



### variable remote\_port 

```C++
uint16_t SYN_UdpSocket::remote_port;
```



Connected remote port number. 


        

<hr>



### variable rx\_buf 

```C++
uint8_t SYN_UdpSocket::rx_buf[SYN_UDP_BUF_SIZE];
```



Receive payload buffer. 


        

<hr>



### variable rx\_len 

```C++
uint16_t SYN_UdpSocket::rx_len;
```



Received payload length. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_udp.h`

