

# File syn\_icmp.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_icmp.c**](syn__icmp_8c.md)

[Go to the source code of this file](syn__icmp_8c_source.md)

_Zero-Heap Native ICMP Protocol Engine Implementation._ 

* `#include "syntropic/net/syn_icmp.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_icmp\_build\_echo\_request**](#function-syn_icmp_build_echo_request) ([**SYN\_ICMP**](structSYN__ICMP.md) \* icmp, [**SYN\_ETH**](structSYN__ETH.md) \* eth, uint32\_t dst\_ip, const uint8\_t dst\_mac, uint16\_t id, uint16\_t seq, const uint8\_t \* payload, size\_t payload\_len, uint8\_t \* frame\_out, size\_t \* frame\_len) <br>_Construct an outbound ICMP Echo Request frame._  |
|  uint16\_t | [**syn\_icmp\_checksum**](#function-syn_icmp_checksum) (const void \* buf, size\_t len) <br>_Compute RFC 1071 Ones-Complement Internet Checksum._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_icmp\_init**](#function-syn_icmp_init) ([**SYN\_ICMP**](structSYN__ICMP.md) \* icmp) <br>_Initialize ICMP Engine Context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_icmp\_process\_packet**](#function-syn_icmp_process_packet) ([**SYN\_ICMP**](structSYN__ICMP.md) \* icmp, const uint8\_t \* ip\_pkt, size\_t len, uint8\_t \* frame\_tx, size\_t \* tx\_len) <br>_Process incoming ICMP packet inside IPv4 payload._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**icmp\_build\_echo\_reply**](#function-icmp_build_echo_reply) ([**SYN\_ICMP**](structSYN__ICMP.md) \* icmp, const uint8\_t \* ip\_pkt, size\_t pkt\_len, size\_t icmp\_offset, uint8\_t \* frame\_tx, size\_t \* tx\_len) <br>_Build an Ethernet II + IPv4 + ICMP Echo Reply frame from a validated Echo Request._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**icmp\_echo\_dst\_unicast**](#function-icmp_echo_dst_unicast) (const uint8\_t \* ip\_pkt) <br>_Verify Echo Request destination MAC and IPv4 address are not broadcast or multicast._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**icmp\_validate\_bounds\_and\_csum**](#function-icmp_validate_bounds_and_csum) ([**SYN\_ICMP**](structSYN__ICMP.md) \* icmp, const uint8\_t \* ip\_pkt, size\_t len, uint8\_t ip\_hl, size\_t \* pkt\_len\_out, size\_t \* icmp\_off\_out) <br>_Validate IPv4 total length and ICMP checksum, returning unpadded lengths._  |


























## Public Functions Documentation




### function syn\_icmp\_build\_echo\_request 

_Construct an outbound ICMP Echo Request frame._ 
```C++
SYN_Status syn_icmp_build_echo_request (
    SYN_ICMP * icmp,
    SYN_ETH * eth,
    uint32_t dst_ip,
    const uint8_t dst_mac,
    uint16_t id,
    uint16_t seq,
    const uint8_t * payload,
    size_t payload_len,
    uint8_t * frame_out,
    size_t * frame_len
) 
```





**Parameters:**


* `icmp` Pointer to ICMP context. 
* `eth` Pointer to Ethernet context. 
* `dst_ip` 32-bit IPv4 address of destination. 
* `dst_mac` 6-byte MAC address of destination. 
* `id` Identifier. 
* `seq` Sequence number. 
* `payload` Pointer to payload data. 
* `payload_len` Payload length in bytes. 
* `frame_out` Output frame buffer (must hold at least max(SYN\_ETH\_MIN\_FRAME\_LEN, 42 + payload\_len) bytes, up to SYN\_ETH\_MAX\_FRAME\_LEN = 1514 bytes). 
* `frame_len` Pointer to receive final Ethernet II frame length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_icmp\_checksum 

_Compute RFC 1071 Ones-Complement Internet Checksum._ 
```C++
uint16_t syn_icmp_checksum (
    const void * buf,
    size_t len
) 
```





**Parameters:**


* `buf` Pointer to buffer bytes. 
* `len` Length in bytes. 



**Returns:**

16-bit Ones-Complement checksum. 





        

<hr>



### function syn\_icmp\_init 

_Initialize ICMP Engine Context._ 
```C++
SYN_Status syn_icmp_init (
    SYN_ICMP * icmp
) 
```





**Parameters:**


* `icmp` Pointer to ICMP context. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_icmp\_process\_packet 

_Process incoming ICMP packet inside IPv4 payload._ 
```C++
SYN_Status syn_icmp_process_packet (
    SYN_ICMP * icmp,
    const uint8_t * ip_pkt,
    size_t len,
    uint8_t * frame_tx,
    size_t * tx_len
) 
```



Automatically generates a matching Type 0 Echo Reply if a Type 8 Echo Request is received.




**Parameters:**


* `icmp` Pointer to ICMP context. 
* `ip_pkt` Pointer to incoming Ethernet II + IPv4 frame. 
* `len` Ethernet frame byte length. 
* `frame_tx` Output buffer to receive constructed Ethernet II reply frame (must hold at least SYN\_ETH\_MAX\_FRAME\_LEN = 1514 bytes when non-NULL). 
* `tx_len` Pointer to receive byte length of constructed reply frame (0 if none generated). 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Public Static Functions Documentation




### function icmp\_build\_echo\_reply 

_Build an Ethernet II + IPv4 + ICMP Echo Reply frame from a validated Echo Request._ 
```C++
static void icmp_build_echo_reply (
    SYN_ICMP * icmp,
    const uint8_t * ip_pkt,
    size_t pkt_len,
    size_t icmp_offset,
    uint8_t * frame_tx,
    size_t * tx_len
) 
```





**Parameters:**


* `icmp` Pointer to ICMP context. 
* `ip_pkt` Inbound Ethernet + IPv4 packet. 
* `pkt_len` Unpadded Ethernet + IPv4 packet length (14 + ip\_total\_len). 
* `icmp_offset` Offset of ICMP header inside ip\_pkt. 
* `frame_tx` Destination buffer for reply frame. 
* `tx_len` Output wire frame length (padded to 60B minimum). 




        

<hr>



### function icmp\_echo\_dst\_unicast 

_Verify Echo Request destination MAC and IPv4 address are not broadcast or multicast._ 
```C++
static bool icmp_echo_dst_unicast (
    const uint8_t * ip_pkt
) 
```





**Parameters:**


* `ip_pkt` Inbound Ethernet + IPv4 packet. 



**Returns:**

true if destination MAC and IPv4 may be reflected in Echo Reply, false otherwise. 





        

<hr>



### function icmp\_validate\_bounds\_and\_csum 

_Validate IPv4 total length and ICMP checksum, returning unpadded lengths._ 
```C++
static SYN_Status icmp_validate_bounds_and_csum (
    SYN_ICMP * icmp,
    const uint8_t * ip_pkt,
    size_t len,
    uint8_t ip_hl,
    size_t * pkt_len_out,
    size_t * icmp_off_out
) 
```





**Parameters:**


* `icmp` Pointer to ICMP context. 
* `ip_pkt` Inbound Ethernet + IPv4 packet. 
* `len` Raw frame byte length. 
* `ip_hl` Validated IPv4 header byte length (&gt;= 20). 
* `pkt_len_out` Output unpadded frame length (14 + ip\_total\_len). 
* `icmp_off_out` Output ICMP header byte offset. 



**Returns:**

SYN\_OK on valid ICMP packet, SYN\_INVALID\_PARAM or SYN\_ERROR otherwise. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_icmp.c`

