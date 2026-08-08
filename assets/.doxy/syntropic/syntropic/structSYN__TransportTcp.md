

# Struct SYN\_TransportTcp



[**ClassList**](annotated.md) **>** [**SYN\_TransportTcp**](structSYN__TransportTcp.md)



_TCP bridge transport layer context._ 

* `#include <syn_transport_tcp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**bytes\_read**](#variable-bytes_read)  <br> |
|  uint16\_t | [**payload\_len**](#variable-payload_len)  <br> |
|  uint8\_t | [**rx\_buf**](#variable-rx_buf)  <br> |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**sock**](#variable-sock)  <br> |
|  uint8\_t | [**state**](#variable-state)  <br> |












































## Public Attributes Documentation




### variable bytes\_read 

```C++
uint16_t SYN_TransportTcp::bytes_read;
```



Bytes of current payload read so far 


        

<hr>



### variable payload\_len 

```C++
uint16_t SYN_TransportTcp::payload_len;
```



Expected size of the incoming frame payload 


        

<hr>



### variable rx\_buf 

```C++
uint8_t SYN_TransportTcp::rx_buf[128];
```



Buffer for staging incoming packet payloads 


        

<hr>



### variable sock 

```C++
SYN_Socket SYN_TransportTcp::sock;
```



Embedded network TCP socket handle 


        

<hr>



### variable state 

```C++
uint8_t SYN_TransportTcp::state;
```



Parser state: 0=len MSB, 1=len LSB, 2=payload 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_transport_tcp.h`

