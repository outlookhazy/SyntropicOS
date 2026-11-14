

# Struct SYN\_TcpCtrlSpec



[**ClassList**](annotated.md) **>** [**SYN\_TcpCtrlSpec**](structSYN__TcpCtrlSpec.md)



_Compact control-segment header parameters for 54-byte TCP frame serialization._ 






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**ack\_num**](#variable-ack_num)  <br> |
|  uint32\_t | [**dst\_ip**](#variable-dst_ip)  <br> |
|  uint8\_t | [**dst\_mac**](#variable-dst_mac)  <br> |
|  uint16\_t | [**dst\_port**](#variable-dst_port)  <br> |
|  uint8\_t | [**flags**](#variable-flags)  <br> |
|  uint32\_t | [**seq\_num**](#variable-seq_num)  <br> |
|  uint32\_t | [**src\_ip**](#variable-src_ip)  <br> |
|  uint16\_t | [**src\_port**](#variable-src_port)  <br> |
|  uint16\_t | [**win**](#variable-win)  <br> |












































## Public Attributes Documentation




### variable ack\_num 

```C++
uint32_t SYN_TcpCtrlSpec::ack_num;
```



Acknowledgment number. 


        

<hr>



### variable dst\_ip 

```C++
uint32_t SYN_TcpCtrlSpec::dst_ip;
```



Destination IPv4 address (host byte order). 


        

<hr>



### variable dst\_mac 

```C++
uint8_t SYN_TcpCtrlSpec::dst_mac[6];
```



Destination Ethernet MAC address. 


        

<hr>



### variable dst\_port 

```C++
uint16_t SYN_TcpCtrlSpec::dst_port;
```



Destination TCP port. 


        

<hr>



### variable flags 

```C++
uint8_t SYN_TcpCtrlSpec::flags;
```



TCP control flags bitmask. 


        

<hr>



### variable seq\_num 

```C++
uint32_t SYN_TcpCtrlSpec::seq_num;
```



Sequence number. 


        

<hr>



### variable src\_ip 

```C++
uint32_t SYN_TcpCtrlSpec::src_ip;
```



Source IPv4 address (host byte order). 


        

<hr>



### variable src\_port 

```C++
uint16_t SYN_TcpCtrlSpec::src_port;
```



Source TCP port. 


        

<hr>



### variable win 

```C++
uint16_t SYN_TcpCtrlSpec::win;
```



Advertised receive window. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_tcp.c`

