

# File syn\_udp.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_udp.c**](syn__udp_8c.md)

[Go to the source code of this file](syn__udp_8c_source.md)

_Zero-Alloc Native UDP Demuxing Engine & Task-Blocking Layer Implementation._ 

* `#include "syntropic/net/syn_udp.h"`
* `#include "syntropic/sched/syn_sched.h"`
* `#include "syntropic/sched/syn_task.h"`
* `#include "syntropic/util/syn_pack.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_UdpSocket**](structSYN__UdpSocket.md) \* | [**syn\_udp\_bind**](#function-syn_udp_bind) ([**SYN\_UDP**](structSYN__UDP.md) \* udp, uint16\_t local\_port) <br>_Bind UDP socket to local port._  |
|  uint16\_t | [**syn\_udp\_checksum**](#function-syn_udp_checksum) (uint32\_t src\_ip, uint32\_t dst\_ip, const uint8\_t \* udp\_seg, size\_t len) <br>_Compute UDP 16-bit 1's complement checksum over pseudo-header and segment._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_udp\_init**](#function-syn_udp_init) ([**SYN\_UDP**](structSYN__UDP.md) \* udp, [**SYN\_ETH**](structSYN__ETH.md) \* eth) <br>_Initialize UDP Stack._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_udp\_process\_packet**](#function-syn_udp_process_packet) ([**SYN\_UDP**](structSYN__UDP.md) \* udp, const uint8\_t \* ip\_packet, size\_t len) <br>_Process incoming UDP IPv4 packet._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_udp\_recv**](#function-syn_udp_recv) ([**SYN\_UdpSocket**](structSYN__UdpSocket.md) \* sock, void \* buf, size\_t max\_len, size\_t \* bytes\_read, uint32\_t \* src\_ip, uint16\_t \* src\_port) <br>_Dequeue a single UDP datagram from a bound socket, preserving datagram boundaries._  |
|  int | [**syn\_udp\_sendto**](#function-syn_udp_sendto) ([**SYN\_UDP**](structSYN__UDP.md) \* udp, uint16\_t src\_port, uint32\_t dst\_ip, uint16\_t dst\_port, const uint8\_t \* data, size\_t data\_len, uint8\_t \* tx\_out, size\_t \* tx\_len) <br>_Construct outbound UDP packet into frame output buffer._  |
|  void | [**syn\_udp\_unbind**](#function-syn_udp_unbind) ([**SYN\_UdpSocket**](structSYN__UdpSocket.md) \* sock) <br>_Unbind and release a UDP socket slot._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**udp\_alloc\_ephemeral\_port**](#function-udp_alloc_ephemeral_port) ([**SYN\_UDP**](structSYN__UDP.md) \* udp) <br>_Allocate an unused ephemeral UDP port in the IANA range 49152..65535._  |
|  void | [**udp\_deliver\_to\_socket**](#function-udp_deliver_to_socket) ([**SYN\_UdpSocket**](structSYN__UdpSocket.md) \* sock, uint32\_t src\_ip, uint16\_t src\_port, const uint8\_t \* payload, size\_t payload\_len) <br>_Deliver a validated UDP payload to a bound socket and detach/wake any blocked task._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**udp\_port\_in\_use**](#function-udp_port_in_use) (const [**SYN\_UDP**](structSYN__UDP.md) \* udp, uint16\_t port) <br>_Check whether a UDP port is already bound on the stack._  |
|  void | [**udp\_resolve\_dst\_mac**](#function-udp_resolve_dst_mac) ([**SYN\_ETH**](structSYN__ETH.md) \* eth, uint32\_t dst\_ip, uint8\_t mac\_out) <br>_Resolve Ethernet destination MAC for an outbound IPv4 UDP datagram._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**udp\_rx\_header\_valid**](#function-udp_rx_header_valid) (const uint8\_t \* ip\_packet, size\_t len, size\_t \* udp\_off\_out, uint16\_t \* udp\_len\_out) <br>_Validate inbound IPv4/UDP header framing and optional RFC 768 checksum._  |
|  void | [**udp\_sync\_head\_view**](#function-udp_sync_head_view) ([**SYN\_UdpSocket**](structSYN__UdpSocket.md) \* sock) <br>_Refresh front datagram view fields (rx\_len, remote\_ip, remote\_port) from ring head._  |
|  void | [**udp\_sync\_legacy\_drain**](#function-udp_sync_legacy_drain) ([**SYN\_UdpSocket**](structSYN__UdpSocket.md) \* sock) <br>_Reconcile descriptor ring if direct legacy readers drained sock-&gt;rx\_len to zero._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_UDP\_EPHEMERAL\_MIN**](syn__udp_8c.md#define-syn_udp_ephemeral_min)  `49152U`<br>_Minimum IANA dynamic/ephemeral UDP port number (RFC 6335)._  |

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



### function syn\_udp\_recv 

_Dequeue a single UDP datagram from a bound socket, preserving datagram boundaries._ 
```C++
SYN_Status syn_udp_recv (
    SYN_UdpSocket * sock,
    void * buf,
    size_t max_len,
    size_t * bytes_read,
    uint32_t * src_ip,
    uint16_t * src_port
) 
```





**Parameters:**


* `sock` Pointer to UDP socket slot. 
* `buf` Destination buffer for datagram payload. 
* `max_len` Maximum bytes to copy into buf. 
* `bytes_read` [out] Bytes copied into buf (0 if no datagram queued). 
* `src_ip` [out] Optional pointer receiving sender IPv4 address. 
* `src_port` [out] Optional pointer receiving sender UDP port. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on NULL required pointer. 





        

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
## Public Static Functions Documentation




### function udp\_alloc\_ephemeral\_port 

_Allocate an unused ephemeral UDP port in the IANA range 49152..65535._ 
```C++
static uint16_t udp_alloc_ephemeral_port (
    SYN_UDP * udp
) 
```





**Parameters:**


* `udp` UDP stack instance. 



**Returns:**

Unused ephemeral port number. 





        

<hr>



### function udp\_deliver\_to\_socket 

_Deliver a validated UDP payload to a bound socket and detach/wake any blocked task._ 
```C++
static void udp_deliver_to_socket (
    SYN_UdpSocket * sock,
    uint32_t src_ip,
    uint16_t src_port,
    const uint8_t * payload,
    size_t payload_len
) 
```





**Parameters:**


* `sock` Bound destination UDP socket. 
* `src_ip` Sender IPv4 address. 
* `src_port` Sender UDP port. 
* `payload` UDP payload bytes. 
* `payload_len` UDP payload byte count. 




        

<hr>



### function udp\_port\_in\_use 

_Check whether a UDP port is already bound on the stack._ 
```C++
static bool udp_port_in_use (
    const SYN_UDP * udp,
    uint16_t port
) 
```





**Parameters:**


* `udp` UDP stack instance. 
* `port` Local port number to check. 



**Returns:**

true if bound, false otherwise. 





        

<hr>



### function udp\_resolve\_dst\_mac 

_Resolve Ethernet destination MAC for an outbound IPv4 UDP datagram._ 
```C++
static void udp_resolve_dst_mac (
    SYN_ETH * eth,
    uint32_t dst_ip,
    uint8_t mac_out
) 
```





**Parameters:**


* `eth` Associated Ethernet interface instance (may be NULL). 
* `dst_ip` Destination IPv4 address (host byte order). 
* `mac_out` Output 6-byte destination MAC buffer. 




        

<hr>



### function udp\_rx\_header\_valid 

_Validate inbound IPv4/UDP header framing and optional RFC 768 checksum._ 
```C++
static bool udp_rx_header_valid (
    const uint8_t * ip_packet,
    size_t len,
    size_t * udp_off_out,
    uint16_t * udp_len_out
) 
```





**Parameters:**


* `ip_packet` Raw Ethernet + IPv4 packet. 
* `len` Packet byte length (&gt;= 42). 
* `udp_off_out` Output byte offset of UDP header. 
* `udp_len_out` Output UDP segment byte length. 



**Returns:**

true if UDP packet is valid, false otherwise. 





        

<hr>



### function udp\_sync\_head\_view 

_Refresh front datagram view fields (rx\_len, remote\_ip, remote\_port) from ring head._ 
```C++
static void udp_sync_head_view (
    SYN_UdpSocket * sock
) 
```





**Parameters:**


* `sock` UDP socket slot. 




        

<hr>



### function udp\_sync\_legacy\_drain 

_Reconcile descriptor ring if direct legacy readers drained sock-&gt;rx\_len to zero._ 
```C++
static void udp_sync_legacy_drain (
    SYN_UdpSocket * sock
) 
```





**Parameters:**


* `sock` UDP socket slot. 




        

<hr>
## Macro Definition Documentation





### define SYN\_UDP\_EPHEMERAL\_MIN 

_Minimum IANA dynamic/ephemeral UDP port number (RFC 6335)._ 
```C++
#define SYN_UDP_EPHEMERAL_MIN `49152U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_udp.c`

