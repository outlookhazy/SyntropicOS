

# File syn\_tcp.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_tcp.c**](syn__tcp_8c.md)

[Go to the source code of this file](syn__tcp_8c_source.md)

_Zero-Alloc Native TCP State Machine & Segment Engine Implementation._ 

* `#include "syntropic/net/syn_tcp.h"`
* `#include "syntropic/port/syn_port_system.h"`
* `#include "syntropic/sched/syn_sched.h"`
* `#include "syntropic/util/syn_assert.h"`
* `#include "syntropic/util/syn_pack.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**syn\_tcp\_checksum**](#function-syn_tcp_checksum) (uint32\_t src\_ip, uint32\_t dst\_ip, const uint8\_t \* tcp\_seg, size\_t len) <br>_Compute TCP 16-bit 1's complement checksum over pseudo-header and segment._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_tcp\_init**](#function-syn_tcp_init) ([**SYN\_TCP**](structSYN__TCP.md) \* tcp, [**SYN\_ETH**](structSYN__ETH.md) \* eth) <br>_Initialize TCP Stack._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_tcp\_listen**](#function-syn_tcp_listen) ([**SYN\_TCP**](structSYN__TCP.md) \* tcp, uint16\_t port) <br>_Bind TCP listener to port._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_tcp\_process\_packet**](#function-syn_tcp_process_packet) ([**SYN\_TCP**](structSYN__TCP.md) \* tcp, const uint8\_t \* ip\_packet, size\_t len, uint8\_t \* tx\_out, size\_t \* tx\_len) <br>_Process incoming TCP IPv4 segment._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**build\_tcp\_segment**](#function-build_tcp_segment) ([**SYN\_TCP**](structSYN__TCP.md) \* tcp, [**SYN\_TcpConn**](structSYN__TcpConn.md) \* conn, uint8\_t flags, const uint8\_t \* payload, size\_t payload\_len, uint8\_t \* tx\_out, size\_t \* tx\_len) <br>_Build outbound TCP segment frame into buffer._  |


























## Public Functions Documentation




### function syn\_tcp\_checksum 

_Compute TCP 16-bit 1's complement checksum over pseudo-header and segment._ 
```C++
uint16_t syn_tcp_checksum (
    uint32_t src_ip,
    uint32_t dst_ip,
    const uint8_t * tcp_seg,
    size_t len
) 
```





**Parameters:**


* `src_ip` Source 32-bit IPv4 address. 
* `dst_ip` Destination 32-bit IPv4 address. 
* `tcp_seg` Pointer to raw TCP segment bytes. 
* `len` TCP segment length in bytes. 



**Returns:**

Computed 16-bit TCP checksum. 





        

<hr>



### function syn\_tcp\_init 

_Initialize TCP Stack._ 
```C++
SYN_Status syn_tcp_init (
    SYN_TCP * tcp,
    SYN_ETH * eth
) 
```





**Parameters:**


* `tcp` Pointer to TCP stack container. 
* `eth` Pointer to Ethernet interface engine instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_tcp\_listen 

_Bind TCP listener to port._ 
```C++
SYN_Status syn_tcp_listen (
    SYN_TCP * tcp,
    uint16_t port
) 
```





**Parameters:**


* `tcp` Pointer to TCP stack container. 
* `port` Port number to listen on. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_tcp\_process\_packet 

_Process incoming TCP IPv4 segment._ 
```C++
SYN_Status syn_tcp_process_packet (
    SYN_TCP * tcp,
    const uint8_t * ip_packet,
    size_t len,
    uint8_t * tx_out,
    size_t * tx_len
) 
```





**Parameters:**


* `tcp` Pointer to TCP stack container. 
* `ip_packet` Pointer to raw IPv4 packet. 
* `len` IPv4 packet byte length. 
* `tx_out` Output buffer for immediate TCP reply segment if generated. 
* `tx_len` Pointer to receive output segment byte length. 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Public Static Functions Documentation




### function build\_tcp\_segment 

_Build outbound TCP segment frame into buffer._ 
```C++
static void build_tcp_segment (
    SYN_TCP * tcp,
    SYN_TcpConn * conn,
    uint8_t flags,
    const uint8_t * payload,
    size_t payload_len,
    uint8_t * tx_out,
    size_t * tx_len
) 
```





**Parameters:**


* `tcp` Pointer to TCP stack. 
* `conn` Pointer to TCP connection block. 
* `flags` TCP control flags bitmask. 
* `payload` Pointer to payload buffer. 
* `payload_len` Payload byte length. 
* `tx_out` Output Ethernet frame buffer. 
* `tx_len` Output Ethernet frame length pointer. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_tcp.c`

