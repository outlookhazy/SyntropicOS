

# File syn\_eth.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_eth.c**](syn__eth_8c.md)

[Go to the source code of this file](syn__eth_8c_source.md)

_Zero-Heap Native Ethernet II & ARP Protocol Engine Implementation._ 

* `#include "syntropic/net/syn_eth.h"`
* `#include "syntropic/net/syn_icmp.h"`
* `#include "syntropic/net/syn_tcp.h"`
* `#include "syntropic/net/syn_transport_udp.h"`
* `#include "syntropic/net/syn_udp.h"`
* `#include "syntropic/port/syn_port_system.h"`
* `#include "syntropic/util/syn_crc.h"`
* `#include <string.h>`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const uint8\_t | [**MAC\_BROADCAST**](#variable-mac_broadcast)   = `{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}`<br> |














## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_eth\_arp\_lookup**](#function-syn_eth_arp_lookup) ([**SYN\_ETH**](structSYN__ETH.md) \* eth, uint32\_t ip, uint8\_t mac\_out) <br>_Lookup MAC address in local ARP cache table._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_eth\_arp\_update**](#function-syn_eth_arp_update) ([**SYN\_ETH**](structSYN__ETH.md) \* eth, uint32\_t ip, const uint8\_t mac) <br>_Update or insert an entry into the local ARP cache table._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_eth\_build\_frame**](#function-syn_eth_build_frame) ([**SYN\_ETH**](structSYN__ETH.md) \* eth, const uint8\_t dst\_mac, uint16\_t ethertype, const uint8\_t \* payload, size\_t payload\_len, uint8\_t \* frame\_out, size\_t \* frame\_len) <br>_Build and transmit an outbound Ethernet II frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_eth\_generate\_mac**](#function-syn_eth_generate_mac) (const void \* uid\_bytes, size\_t uid\_len, uint8\_t mac\_out) <br>_Generate a deterministic Locally Administered MAC address from MCU Unique ID._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) struct [**SYN\_ICMP**](structSYN__ICMP.md) \* | [**syn\_eth\_get\_icmp\_instance**](#function-syn_eth_get_icmp_instance) (void) <br>_Weak hook — override to inject an ICMP engine into the Ethernet dispatcher._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) struct [**SYN\_TCP**](structSYN__TCP.md) \* | [**syn\_eth\_get\_tcp\_instance**](#function-syn_eth_get_tcp_instance) (void) <br>_Weak hook — override to inject a TCP engine into the Ethernet dispatcher._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_eth\_init**](#function-syn_eth_init) ([**SYN\_ETH**](structSYN__ETH.md) \* eth, const uint8\_t mac\_addr, uint32\_t ip\_addr) <br>_Initialize Native Ethernet Engine._  |
|  size\_t | [**syn\_eth\_pack\_header**](#function-syn_eth_pack_header) (uint8\_t \* buf, size\_t buf\_size, const uint8\_t dst\_mac, const uint8\_t src\_mac, uint16\_t ethertype) <br>_Pack Ethernet II Header (14 bytes) into target buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_eth\_process\_frame**](#function-syn_eth_process_frame) ([**SYN\_ETH**](structSYN__ETH.md) \* eth, const uint8\_t \* frame, size\_t len, uint8\_t \* tx\_buf, size\_t \* tx\_len) <br>_Process incoming Ethernet II frame._  |
|  uint16\_t | [**syn\_ip\_checksum**](#function-syn_ip_checksum) (const void \* buf, size\_t len) <br>_Calculate 16-bit Internet Checksum (RFC 1071) over buffer._  |
|  size\_t | [**syn\_ip\_pack\_header**](#function-syn_ip_pack_header) (uint8\_t \* buf, size\_t buf\_size, uint32\_t src\_ip, uint32\_t dst\_ip, uint8\_t proto, uint16\_t payload\_len, uint16\_t id) <br>_Pack standard 20-byte IPv4 Header into target buffer._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_UDP**](structSYN__UDP.md) \* | [**syn\_transport\_udp\_get\_instance**](#function-syn_transport_udp_get_instance) (void) <br>_Weak hook to obtain UDP transport instance._  |




























## Public Static Attributes Documentation




### variable MAC\_BROADCAST 

```C++
const uint8_t MAC_BROADCAST[6];
```



Broadcast MAC address (FF:FF:FF:FF:FF:FF) 


        

<hr>
## Public Functions Documentation




### function syn\_eth\_arp\_lookup 

_Lookup MAC address in local ARP cache table._ 
```C++
SYN_Status syn_eth_arp_lookup (
    SYN_ETH * eth,
    uint32_t ip,
    uint8_t mac_out
) 
```





**Parameters:**


* `eth` Pointer to Ethernet instance. 
* `ip` 32-bit IPv4 address to lookup. 
* `mac_out` Output buffer (6 bytes) to receive resolved MAC. 



**Returns:**

SYN\_OK if IP found in cache, SYN\_NOT\_FOUND if absent. 





        

<hr>



### function syn\_eth\_arp\_update 

_Update or insert an entry into the local ARP cache table._ 
```C++
SYN_Status syn_eth_arp_update (
    SYN_ETH * eth,
    uint32_t ip,
    const uint8_t mac
) 
```





**Parameters:**


* `eth` Pointer to Ethernet instance. 
* `ip` 32-bit IPv4 address. 
* `mac` Resolved 6-byte MAC address. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_eth\_build\_frame 

_Build and transmit an outbound Ethernet II frame._ 
```C++
SYN_Status syn_eth_build_frame (
    SYN_ETH * eth,
    const uint8_t dst_mac,
    uint16_t ethertype,
    const uint8_t * payload,
    size_t payload_len,
    uint8_t * frame_out,
    size_t * frame_len
) 
```





**Parameters:**


* `eth` Pointer to Ethernet instance. 
* `dst_mac` Destination MAC address (6 bytes). 
* `ethertype` EtherType value (e.g. SYN\_ETHTYPE\_IPV4). 
* `payload` Pointer to payload buffer. 
* `payload_len` Payload byte length. 
* `frame_out` Output frame buffer (must hold at least 1514 bytes). 
* `frame_len` Pointer to receive final raw Ethernet frame length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_eth\_generate\_mac 

_Generate a deterministic Locally Administered MAC address from MCU Unique ID._ 
```C++
SYN_Status syn_eth_generate_mac (
    const void * uid_bytes,
    size_t uid_len,
    uint8_t mac_out
) 
```





**Parameters:**


* `uid_bytes` Pointer to hardware unique ID bytes (e.g. STM32 96-bit UID). 
* `uid_len` Length of unique ID in bytes. 
* `mac_out` Output buffer to receive generated 6-byte MAC address. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_eth\_get\_icmp\_instance 

_Weak hook — override to inject an ICMP engine into the Ethernet dispatcher._ 
```C++
SYN_WEAK struct SYN_ICMP * syn_eth_get_icmp_instance (
    void
) 
```



When syn\_eth\_process\_frame receives an IPv4/ICMP packet (protocol 1), it calls this to obtain the ICMP instance. Return NULL to silently drop.




**Returns:**

Pointer to [**SYN\_ICMP**](structSYN__ICMP.md) instance, or NULL. 





        

<hr>



### function syn\_eth\_get\_tcp\_instance 

_Weak hook — override to inject a TCP engine into the Ethernet dispatcher._ 
```C++
SYN_WEAK struct SYN_TCP * syn_eth_get_tcp_instance (
    void
) 
```



When syn\_eth\_process\_frame receives an IPv4/TCP packet (protocol 6), it calls this to obtain the TCP instance. Return NULL to silently drop.




**Returns:**

Pointer to [**SYN\_TCP**](structSYN__TCP.md) instance, or NULL. 





        

<hr>



### function syn\_eth\_init 

_Initialize Native Ethernet Engine._ 
```C++
SYN_Status syn_eth_init (
    SYN_ETH * eth,
    const uint8_t mac_addr,
    uint32_t ip_addr
) 
```





**Parameters:**


* `eth` Pointer to Ethernet instance. 
* `mac_addr` 6-byte hardware MAC address. 
* `ip_addr` 32-bit IPv4 address (host byte order). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_eth\_pack\_header 

_Pack Ethernet II Header (14 bytes) into target buffer._ 
```C++
size_t syn_eth_pack_header (
    uint8_t * buf,
    size_t buf_size,
    const uint8_t dst_mac,
    const uint8_t src_mac,
    uint16_t ethertype
) 
```





**Parameters:**


* `buf` Output destination buffer. 
* `buf_size` Destination buffer capacity. 
* `dst_mac` Destination 6-byte MAC address. 
* `src_mac` Source 6-byte MAC address. 
* `ethertype` 16-bit EtherType (host byte order). 



**Returns:**

Bytes written (14) on success, 0 on invalid param / insufficient buffer. 





        

<hr>



### function syn\_eth\_process\_frame 

_Process incoming Ethernet II frame._ 
```C++
SYN_Status syn_eth_process_frame (
    SYN_ETH * eth,
    const uint8_t * frame,
    size_t len,
    uint8_t * tx_buf,
    size_t * tx_len
) 
```





**Parameters:**


* `eth` Pointer to Ethernet instance. 
* `frame` Pointer to raw frame bytes. 
* `len` Frame byte length (60..1514 bytes). 
* `tx_buf` Output buffer to receive immediate ARP reply if generated. 
* `tx_len` Pointer to receive ARP reply byte length (0 if no reply needed). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ip\_checksum 

_Calculate 16-bit Internet Checksum (RFC 1071) over buffer._ 
```C++
uint16_t syn_ip_checksum (
    const void * buf,
    size_t len
) 
```





**Parameters:**


* `buf` Pointer to buffer. 
* `len` Buffer byte length. 



**Returns:**

16-bit one's complement checksum. 





        

<hr>



### function syn\_ip\_pack\_header 

_Pack standard 20-byte IPv4 Header into target buffer._ 
```C++
size_t syn_ip_pack_header (
    uint8_t * buf,
    size_t buf_size,
    uint32_t src_ip,
    uint32_t dst_ip,
    uint8_t proto,
    uint16_t payload_len,
    uint16_t id
) 
```





**Parameters:**


* `buf` Output destination buffer. 
* `buf_size` Destination buffer capacity. 
* `src_ip` Source IPv4 address (host byte order). 
* `dst_ip` Destination IPv4 address (host byte order). 
* `proto` IP Protocol number (1=ICMP, 2=IGMP, 6=TCP, 17=UDP). 
* `payload_len` Payload byte length after IPv4 header. 
* `id` 16-bit IP identification field. 



**Returns:**

Bytes written (20) on success, 0 on invalid param / insufficient buffer. 





        

<hr>



### function syn\_transport\_udp\_get\_instance 

_Weak hook to obtain UDP transport instance._ 
```C++
SYN_WEAK  SYN_UDP * syn_transport_udp_get_instance (
    void
) 
```



Get the active native [**SYN\_UDP**](structSYN__UDP.md) stack instance.




**Returns:**

Pointer to global [**SYN\_UDP**](structSYN__UDP.md) instance, or NULL if unavailable. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_eth.c`

