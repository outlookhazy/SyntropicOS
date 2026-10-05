

# Struct SYN\_TcpConn



[**ClassList**](annotated.md) **>** [**SYN\_TcpConn**](structSYN__TcpConn.md)



_Single TCP socket connection control block._ 

* `#include <syn_tcp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**ack\_nxt**](#variable-ack_nxt)  <br> |
|  [**SYN\_Task**](structSYN__Task.md) \* | [**blocked\_task**](#variable-blocked_task)  <br> |
|  uint32\_t | [**local\_ip**](#variable-local_ip)  <br> |
|  uint16\_t | [**local\_port**](#variable-local_port)  <br> |
|  uint32\_t | [**remote\_ip**](#variable-remote_ip)  <br> |
|  uint8\_t | [**remote\_mac**](#variable-remote_mac)  <br> |
|  uint16\_t | [**remote\_port**](#variable-remote_port)  <br> |
|  uint8\_t | [**rx\_buf**](#variable-rx_buf)  <br> |
|  uint16\_t | [**rx\_len**](#variable-rx_len)  <br> |
|  uint32\_t | [**seq\_nxt**](#variable-seq_nxt)  <br> |
|  [**SYN\_TcpState**](syn__tcp_8h.md#enum-syn_tcpstate) | [**state**](#variable-state)  <br> |
|  uint8\_t | [**tx\_buf**](#variable-tx_buf)  <br> |
|  uint16\_t | [**tx\_len**](#variable-tx_len)  <br> |












































## Public Attributes Documentation




### variable ack\_nxt 

```C++
uint32_t SYN_TcpConn::ack_nxt;
```



Expected next sequence number to receive. 


        

<hr>



### variable blocked\_task 

```C++
SYN_Task* SYN_TcpConn::blocked_task;
```



Task currently blocked waiting on TCP data. 


        

<hr>



### variable local\_ip 

```C++
uint32_t SYN_TcpConn::local_ip;
```



Local IPv4 address. 


        

<hr>



### variable local\_port 

```C++
uint16_t SYN_TcpConn::local_port;
```



Local port. 


        

<hr>



### variable remote\_ip 

```C++
uint32_t SYN_TcpConn::remote_ip;
```



Remote peer IPv4 address. 


        

<hr>



### variable remote\_mac 

```C++
uint8_t SYN_TcpConn::remote_mac[6];
```



Remote peer Ethernet MAC address. 


        

<hr>



### variable remote\_port 

```C++
uint16_t SYN_TcpConn::remote_port;
```



Remote peer port. 


        

<hr>



### variable rx\_buf 

```C++
uint8_t SYN_TcpConn::rx_buf[SYN_TCP_BUF_SIZE];
```



Receive data buffer. 


        

<hr>



### variable rx\_len 

```C++
uint16_t SYN_TcpConn::rx_len;
```



Length of buffered receive bytes. 


        

<hr>



### variable seq\_nxt 

```C++
uint32_t SYN_TcpConn::seq_nxt;
```



Next sequence number to send. 


        

<hr>



### variable state 

```C++
SYN_TcpState SYN_TcpConn::state;
```



Connection state machine state. 


        

<hr>



### variable tx\_buf 

```C++
uint8_t SYN_TcpConn::tx_buf[SYN_TCP_BUF_SIZE];
```



Transmit data buffer. 


        

<hr>



### variable tx\_len 

```C++
uint16_t SYN_TcpConn::tx_len;
```



Length of buffered transmit bytes. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_tcp.h`

