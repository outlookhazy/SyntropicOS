

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
* `frame_out` Output frame buffer (must hold at least 60 bytes). 
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
* `ip_pkt` Pointer to incoming IPv4 packet (starting at IP header). 
* `len` IP packet byte length. 
* `frame_tx` Output buffer to receive constructed Ethernet II reply frame (min 60 bytes). 
* `tx_len` Pointer to receive byte length of constructed reply frame (0 if none generated). 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_icmp.c`

