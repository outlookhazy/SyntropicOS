

# File syn\_eth.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_eth.h**](syn__eth_8h.md)

[Go to the source code of this file](syn__eth_8h_source.md)

_Zero-Heap Native Ethernet II & ARP Protocol Engine._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`
* `#include "port/syn_port_eth.h"`
* `#include "syntropic/pt/syn_pt.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_ETH**](structSYN__ETH.md) <br> |
| struct | [**SYN\_ETH\_ArpEntry**](structSYN__ETH__ArpEntry.md) <br> |
| struct | [**SYN\_ETH\_Header**](structSYN__ETH__Header.md) <br> |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_eth\_arp\_lookup**](#function-syn_eth_arp_lookup) ([**SYN\_ETH**](structSYN__ETH.md) \* eth, uint32\_t ip, uint8\_t mac\_out) <br>_Lookup MAC address in local ARP cache table._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_eth\_arp\_update**](#function-syn_eth_arp_update) ([**SYN\_ETH**](structSYN__ETH.md) \* eth, uint32\_t ip, const uint8\_t mac) <br>_Update or insert an entry into the local ARP cache table._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_eth\_build\_frame**](#function-syn_eth_build_frame) ([**SYN\_ETH**](structSYN__ETH.md) \* eth, const uint8\_t dst\_mac, uint16\_t ethertype, const uint8\_t \* payload, size\_t payload\_len, uint8\_t \* frame\_out, size\_t \* frame\_len) <br>_Build and transmit an outbound Ethernet II frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_eth\_generate\_mac**](#function-syn_eth_generate_mac) (const void \* uid\_bytes, size\_t uid\_len, uint8\_t mac\_out) <br>_Generate a deterministic Locally Administered MAC address from MCU Unique ID._  |
|  struct [**SYN\_ICMP**](structSYN__ICMP.md) \* | [**syn\_eth\_get\_icmp\_instance**](#function-syn_eth_get_icmp_instance) (void) <br>_Weak hook — override to inject an ICMP engine into the Ethernet dispatcher._  |
|  struct [**SYN\_TCP**](structSYN__TCP.md) \* | [**syn\_eth\_get\_tcp\_instance**](#function-syn_eth_get_tcp_instance) (void) <br>_Weak hook — override to inject a TCP engine into the Ethernet dispatcher._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_eth\_init**](#function-syn_eth_init) ([**SYN\_ETH**](structSYN__ETH.md) \* eth, const uint8\_t mac\_addr, uint32\_t ip\_addr) <br>_Initialize Native Ethernet Engine._  |
|  size\_t | [**syn\_eth\_pack\_header**](#function-syn_eth_pack_header) (uint8\_t \* buf, size\_t buf\_size, const uint8\_t dst\_mac, const uint8\_t src\_mac, uint16\_t ethertype) <br>_Pack Ethernet II Header (14 bytes) into target buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_eth\_process\_frame**](#function-syn_eth_process_frame) ([**SYN\_ETH**](structSYN__ETH.md) \* eth, const uint8\_t \* frame, size\_t len, uint8\_t \* tx\_buf, size\_t \* tx\_len) <br>_Process incoming Ethernet II frame._  |
|  uint16\_t | [**syn\_ip\_checksum**](#function-syn_ip_checksum) (const void \* buf, size\_t len) <br>_Calculate 16-bit Internet Checksum (RFC 1071) over buffer._  |
|  size\_t | [**syn\_ip\_pack\_header**](#function-syn_ip_pack_header) (uint8\_t \* buf, size\_t buf\_size, uint32\_t src\_ip, uint32\_t dst\_ip, uint8\_t proto, uint16\_t payload\_len, uint16\_t id) <br>_Pack standard 20-byte IPv4 Header into target buffer._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**PT\_ETH\_WAIT\_ARP**](syn__eth_8h.md#define-pt_eth_wait_arp) (pt, eth, target\_ip, mac\_out) `[**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until)(pt, [**syn\_eth\_arp\_lookup**](syn__eth_8c.md#function-syn_eth_arp_lookup)((eth), (target\_ip), (mac\_out)) == SYN\_OK)`<br>_Block a protothread coroutine until an IP is resolved in the local ARP cache._  |
| define  | [**PT\_ETH\_WAIT\_FRAME**](syn__eth_8h.md#define-pt_eth_wait_frame) (pt, rx\_buf, max\_len, rx\_len) `[**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until)(pt, [**syn\_port\_eth\_rx**](syn__port__eth_8h.md#function-syn_port_eth_rx)((rx\_buf), (max\_len), (rx\_len)) == SYN\_OK)`<br>_Block a protothread coroutine until a raw Ethernet frame is received from PHY._  |
| define  | [**SYN\_ARP\_OP\_REPLY**](syn__eth_8h.md#define-syn_arp_op_reply)  `2U`<br> |
| define  | [**SYN\_ARP\_OP\_REQUEST**](syn__eth_8h.md#define-syn_arp_op_request)  `1U`<br> |
| define  | [**SYN\_ETHTYPE\_ARP**](syn__eth_8h.md#define-syn_ethtype_arp)  `0x0806U`<br> |
| define  | [**SYN\_ETHTYPE\_ETHERCAT**](syn__eth_8h.md#define-syn_ethtype_ethercat)  `0x88A4U`<br> |
| define  | [**SYN\_ETHTYPE\_IPV4**](syn__eth_8h.md#define-syn_ethtype_ipv4)  `0x0800U`<br> |
| define  | [**SYN\_ETHTYPE\_IPV6**](syn__eth_8h.md#define-syn_ethtype_ipv6)  `0x86DDU`<br> |
| define  | [**SYN\_ETH\_ARP\_CACHE\_SIZE**](syn__eth_8h.md#define-syn_eth_arp_cache_size)  `8U`<br> |
| define  | [**SYN\_ETH\_HEADER\_LEN**](syn__eth_8h.md#define-syn_eth_header_len)  `14U`<br> |
| define  | [**SYN\_ETH\_MAC\_LEN**](syn__eth_8h.md#define-syn_eth_mac_len)  `6U`<br> |
| define  | [**SYN\_ETH\_MAX\_FRAME\_LEN**](syn__eth_8h.md#define-syn_eth_max_frame_len)  `1514U`<br> |
| define  | [**SYN\_ETH\_MIN\_FRAME\_LEN**](syn__eth_8h.md#define-syn_eth_min_frame_len)  `60U`<br> |

## Detailed Description


Specifications:
* Ethernet II Header: [Dst MAC 6B] + [Src MAC 6B] + [EtherType 2B]
* Minimum Frame Size: 60 bytes (padded with zeros if payload &lt; 46B)
* Maximum Frame Size: 1514 bytes (excluding 4B FCS)
* EtherTypes:
  * 0x0806: ARP (Address Resolution Protocol)
  * 0x0800: IPv4 (Internet Protocol v4)
  * 0x86DD: IPv6 (Internet Protocol v6)
  * 0x88A4: EtherCAT Industrial Ethernet 






    
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
struct SYN_ICMP * syn_eth_get_icmp_instance (
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
struct SYN_TCP * syn_eth_get_tcp_instance (
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
## Macro Definition Documentation





### define PT\_ETH\_WAIT\_ARP 

_Block a protothread coroutine until an IP is resolved in the local ARP cache._ 
```C++
#define PT_ETH_WAIT_ARP (
    pt,
    eth,
    target_ip,
    mac_out
) `PT_WAIT_UNTIL (pt, syn_eth_arp_lookup ((eth), (target_ip), (mac_out)) == SYN_OK )`
```





**Parameters:**


* `pt` Protothread context. 
* `eth` Pointer to Ethernet instance. 
* `target_ip` Target IPv4 address. 
* `mac_out` Output buffer (6 bytes). 




        

<hr>



### define PT\_ETH\_WAIT\_FRAME 

_Block a protothread coroutine until a raw Ethernet frame is received from PHY._ 
```C++
#define PT_ETH_WAIT_FRAME (
    pt,
    rx_buf,
    max_len,
    rx_len
) `PT_WAIT_UNTIL (pt, syn_port_eth_rx ((rx_buf), (max_len), (rx_len)) == SYN_OK )`
```





**Parameters:**


* `pt` Protothread context. 
* `rx_buf` Buffer to receive frame. 
* `max_len` Capacity. 
* `rx_len` Pointer to receive length. 




        

<hr>



### define SYN\_ARP\_OP\_REPLY 

```C++
#define SYN_ARP_OP_REPLY `2U`
```



ARP Operation: Reply (2) 


        

<hr>



### define SYN\_ARP\_OP\_REQUEST 

```C++
#define SYN_ARP_OP_REQUEST `1U`
```



ARP Operation: Request (1) 


        

<hr>



### define SYN\_ETHTYPE\_ARP 

```C++
#define SYN_ETHTYPE_ARP `0x0806U`
```



EtherType for ARP packets (0x0806) 


        

<hr>



### define SYN\_ETHTYPE\_ETHERCAT 

```C++
#define SYN_ETHTYPE_ETHERCAT `0x88A4U`
```



EtherType for EtherCAT frames (0x88A4) 


        

<hr>



### define SYN\_ETHTYPE\_IPV4 

```C++
#define SYN_ETHTYPE_IPV4 `0x0800U`
```



EtherType for IPv4 packets (0x0800) 


        

<hr>



### define SYN\_ETHTYPE\_IPV6 

```C++
#define SYN_ETHTYPE_IPV6 `0x86DDU`
```



EtherType for IPv6 packets (0x86DD) 


        

<hr>



### define SYN\_ETH\_ARP\_CACHE\_SIZE 

```C++
#define SYN_ETH_ARP_CACHE_SIZE `8U`
```



Default ARP cache table capacity 


        

<hr>



### define SYN\_ETH\_HEADER\_LEN 

```C++
#define SYN_ETH_HEADER_LEN `14U`
```



Ethernet II header length in bytes 


        

<hr>



### define SYN\_ETH\_MAC\_LEN 

```C++
#define SYN_ETH_MAC_LEN `6U`
```



Ethernet MAC address length in bytes 


        

<hr>



### define SYN\_ETH\_MAX\_FRAME\_LEN 

```C++
#define SYN_ETH_MAX_FRAME_LEN `1514U`
```



Maximum standard Ethernet frame length 


        

<hr>



### define SYN\_ETH\_MIN\_FRAME\_LEN 

```C++
#define SYN_ETH_MIN_FRAME_LEN `60U`
```



Minimum Ethernet frame length with padding 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_eth.h`

