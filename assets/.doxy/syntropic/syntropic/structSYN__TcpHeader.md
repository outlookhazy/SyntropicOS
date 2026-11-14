

# Struct SYN\_TcpHeader



[**ClassList**](annotated.md) **>** [**SYN\_TcpHeader**](structSYN__TcpHeader.md)



_TCP packet header structure._ 

* `#include <syn_tcp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**ack\_num**](#variable-ack_num)  <br> |
|  uint16\_t | [**checksum**](#variable-checksum)  <br> |
|  uint8\_t | [**data\_offset**](#variable-data_offset)  <br> |
|  uint16\_t | [**dst\_port**](#variable-dst_port)  <br> |
|  uint8\_t | [**flags**](#variable-flags)  <br> |
|  uint32\_t | [**seq\_num**](#variable-seq_num)  <br> |
|  uint16\_t | [**src\_port**](#variable-src_port)  <br> |
|  uint16\_t | [**urgent\_ptr**](#variable-urgent_ptr)  <br> |
|  uint16\_t | [**window**](#variable-window)  <br> |












































## Public Attributes Documentation




### variable ack\_num 

```C++
uint32_t SYN_TcpHeader::ack_num;
```



Acknowledgment number. 


        

<hr>



### variable checksum 

```C++
uint16_t SYN_TcpHeader::checksum;
```



Checksum value over pseudo-header and payload. 


        

<hr>



### variable data\_offset 

```C++
uint8_t SYN_TcpHeader::data_offset;
```



Header length in 32-bit words (top 4 bits). 


        

<hr>



### variable dst\_port 

```C++
uint16_t SYN_TcpHeader::dst_port;
```



Destination port number. 


        

<hr>



### variable flags 

```C++
uint8_t SYN_TcpHeader::flags;
```



Control flags (SYN, ACK, FIN, RST, PSH, URG). 


        

<hr>



### variable seq\_num 

```C++
uint32_t SYN_TcpHeader::seq_num;
```



Sequence number. 


        

<hr>



### variable src\_port 

```C++
uint16_t SYN_TcpHeader::src_port;
```



Source port number. 


        

<hr>



### variable urgent\_ptr 

```C++
uint16_t SYN_TcpHeader::urgent_ptr;
```



Urgent pointer. 


        

<hr>



### variable window 

```C++
uint16_t SYN_TcpHeader::window;
```



Receive window size. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_tcp.h`

