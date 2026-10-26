

# File syn\_udp.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_udp.h**](syn__udp_8h.md)

[Go to the source code of this file](syn__udp_8h_source.md)

_Zero-Alloc Native UDP Demuxing Engine & Task-Blocking Layer._ 

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/net/syn_eth.h"`
* `#include "syntropic/pt/syn_pt.h"`
* `#include "syntropic/sched/syn_sched.h"`
* `#include "syntropic/sched/syn_task.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_UDP**](structSYN__UDP.md) <br>_UDP protocol stack container instance._  |
| struct | [**SYN\_UdpHeader**](structSYN__UdpHeader.md) <br>_UDP packet header structure (8 bytes)._  |
| struct | [**SYN\_UdpSocket**](structSYN__UdpSocket.md) <br>_UDP socket slot control block._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_UdpSocket**](structSYN__UdpSocket.md) \* | [**syn\_udp\_bind**](#function-syn_udp_bind) ([**SYN\_UDP**](structSYN__UDP.md) \* udp, uint16\_t local\_port) <br>_Bind UDP socket to local port._  |
|  uint16\_t | [**syn\_udp\_checksum**](#function-syn_udp_checksum) (uint32\_t src\_ip, uint32\_t dst\_ip, const uint8\_t \* udp\_seg, size\_t len) <br>_Compute UDP 16-bit 1's complement checksum over pseudo-header and segment._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_udp\_init**](#function-syn_udp_init) ([**SYN\_UDP**](structSYN__UDP.md) \* udp, [**SYN\_ETH**](structSYN__ETH.md) \* eth) <br>_Initialize UDP Stack._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_udp\_process\_packet**](#function-syn_udp_process_packet) ([**SYN\_UDP**](structSYN__UDP.md) \* udp, const uint8\_t \* ip\_packet, size\_t len) <br>_Process incoming UDP IPv4 packet._  |
|  int | [**syn\_udp\_sendto**](#function-syn_udp_sendto) ([**SYN\_UDP**](structSYN__UDP.md) \* udp, uint16\_t src\_port, uint32\_t dst\_ip, uint16\_t dst\_port, const uint8\_t \* data, size\_t data\_len, uint8\_t \* tx\_out, size\_t \* tx\_len) <br>_Construct outbound UDP packet into frame output buffer._  |
|  void | [**syn\_udp\_unbind**](#function-syn_udp_unbind) ([**SYN\_UdpSocket**](structSYN__UdpSocket.md) \* sock) <br>_Unbind and release a UDP socket slot._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**PT\_UDP\_BLOCK\_READ**](syn__udp_8h.md#define-pt_udp_block_read) (pt, task, sock, buf, max\_len, bytes\_read, src\_ip, src\_port) `/* multi line expression */`<br>_Protothread helper macro to block task until UDP packet arrives._  |
| define  | [**SYN\_UDP\_BUF\_SIZE**](syn__udp_8h.md#define-syn_udp_buf_size)  `256`<br> |
| define  | [**SYN\_UDP\_MAX\_SOCKETS**](syn__udp_8h.md#define-syn_udp_max_sockets)  `4`<br> |

## Public Functions Documentation




### function syn\_udp\_bind 

_Bind UDP socket to local port._ 
```C++
SYN_UdpSocket * syn_udp_bind (
    SYN_UDP * udp,
    uint16_t local_port
) 
```





**Parameters:**


* `udp` Pointer to UDP stack instance. 
* `local_port` Port number to bind. 



**Returns:**

Pointer to allocated UDP socket slot, or NULL if capacity exceeded. 





        

<hr>



### function syn\_udp\_checksum 

_Compute UDP 16-bit 1's complement checksum over pseudo-header and segment._ 
```C++
uint16_t syn_udp_checksum (
    uint32_t src_ip,
    uint32_t dst_ip,
    const uint8_t * udp_seg,
    size_t len
) 
```





**Parameters:**


* `src_ip` Source 32-bit IPv4 address. 
* `dst_ip` Destination 32-bit IPv4 address. 
* `udp_seg` Pointer to raw UDP segment bytes. 
* `len` UDP segment length in bytes. 



**Returns:**

Computed 16-bit UDP checksum. 





        

<hr>



### function syn\_udp\_init 

_Initialize UDP Stack._ 
```C++
SYN_Status syn_udp_init (
    SYN_UDP * udp,
    SYN_ETH * eth
) 
```





**Parameters:**


* `udp` Pointer to UDP stack instance. 
* `eth` Pointer to Ethernet interface instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_udp\_process\_packet 

_Process incoming UDP IPv4 packet._ 
```C++
SYN_Status syn_udp_process_packet (
    SYN_UDP * udp,
    const uint8_t * ip_packet,
    size_t len
) 
```





**Parameters:**


* `udp` Pointer to UDP stack instance. 
* `ip_packet` Pointer to raw IPv4 packet. 
* `len` IPv4 packet byte length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_udp\_sendto 

_Construct outbound UDP packet into frame output buffer._ 
```C++
int syn_udp_sendto (
    SYN_UDP * udp,
    uint16_t src_port,
    uint32_t dst_ip,
    uint16_t dst_port,
    const uint8_t * data,
    size_t data_len,
    uint8_t * tx_out,
    size_t * tx_len
) 
```





**Parameters:**


* `udp` Pointer to UDP stack. 
* `src_port` Source UDP port. 
* `dst_ip` Destination IPv4 address. 
* `dst_port` Destination UDP port. 
* `data` Pointer to payload data buffer. 
* `data_len` Payload byte length. 
* `tx_out` Output Ethernet frame buffer. 
* `tx_len` Pointer to receive final Ethernet frame length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_udp\_unbind 

_Unbind and release a UDP socket slot._ 
```C++
void syn_udp_unbind (
    SYN_UdpSocket * sock
) 
```





**Parameters:**


* `sock` Pointer to UDP socket slot. 




        

<hr>
## Macro Definition Documentation





### define PT\_UDP\_BLOCK\_READ 

_Protothread helper macro to block task until UDP packet arrives._ 
```C++
#define PT_UDP_BLOCK_READ (
    pt,
    task,
    sock,
    buf,
    max_len,
    bytes_read,
    src_ip,
    src_port
) `/* multi line expression */`
```





**Parameters:**


* `pt` Protothread state. 
* `task` Task instance. 
* `sock` UDP socket pointer. 
* `buf` Destination read buffer. 
* `max_len` Maximum bytes to read. 
* `bytes_read` Pointer to output size\_t receiving count of read bytes. 
* `src_ip` Pointer to uint32\_t receiving sender IP address. 
* `src_port` Pointer to uint16\_t receiving sender port. 




        

<hr>



### define SYN\_UDP\_BUF\_SIZE 

```C++
#define SYN_UDP_BUF_SIZE `256`
```



UDP socket internal RX buffer size in bytes 


        

<hr>



### define SYN\_UDP\_MAX\_SOCKETS 

```C++
#define SYN_UDP_MAX_SOCKETS `4`
```



Maximum concurrent UDP bound sockets 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_udp.h`

