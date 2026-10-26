

# File syn\_igmp.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_igmp.c**](syn__igmp_8c.md)

[Go to the source code of this file](syn__igmp_8c_source.md)

_Zero-Heap Native IGMPv2 Protocol Engine Implementation._ 

* `#include "syntropic/net/syn_igmp.h"`
* `#include "syntropic/net/syn_icmp.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_igmp\_build\_report**](#function-syn_igmp_build_report) ([**SYN\_IGMP**](structSYN__IGMP.md) \* igmp, [**SYN\_ETH**](structSYN__ETH.md) \* eth, uint8\_t type, uint32\_t group\_ip, uint8\_t \* frame\_out, size\_t \* frame\_len) <br>_Build an IGMPv2 Membership Report (Join Group) or Leave frame over Ethernet._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_igmp\_init**](#function-syn_igmp_init) ([**SYN\_IGMP**](structSYN__IGMP.md) \* igmp) <br>_Initialize IGMP Engine Context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_igmp\_join\_group**](#function-syn_igmp_join_group) ([**SYN\_IGMP**](structSYN__IGMP.md) \* igmp, [**SYN\_ETH**](structSYN__ETH.md) \* eth, uint32\_t group\_ip, uint8\_t \* frame\_out, size\_t \* frame\_len) <br>_Join an IPv4 Multicast Group and construct IGMPv2 Join Frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_igmp\_leave\_group**](#function-syn_igmp_leave_group) ([**SYN\_IGMP**](structSYN__IGMP.md) \* igmp, [**SYN\_ETH**](structSYN__ETH.md) \* eth, uint32\_t group\_ip, uint8\_t \* frame\_out, size\_t \* frame\_len) <br>_Leave an IPv4 Multicast Group and construct IGMPv2 Leave Frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_igmp\_process\_packet**](#function-syn_igmp_process_packet) ([**SYN\_IGMP**](structSYN__IGMP.md) \* igmp, [**SYN\_ETH**](structSYN__ETH.md) \* eth, const uint8\_t \* ip\_pkt, size\_t len, uint8\_t \* frame\_tx, size\_t \* tx\_len) <br>_Process incoming IGMP packet (e.g. Membership Query from switch)._  |




























## Public Functions Documentation




### function syn\_igmp\_build\_report 

_Build an IGMPv2 Membership Report (Join Group) or Leave frame over Ethernet._ 
```C++
SYN_Status syn_igmp_build_report (
    SYN_IGMP * igmp,
    SYN_ETH * eth,
    uint8_t type,
    uint32_t group_ip,
    uint8_t * frame_out,
    size_t * frame_len
) 
```





**Parameters:**


* `igmp` Pointer to IGMP context. 
* `eth` Pointer to Ethernet context. 
* `type` SYN\_IGMP\_TYPE\_V2\_REPORT or SYN\_IGMP\_TYPE\_V2\_LEAVE. 
* `group_ip` 32-bit IPv4 multicast address. 
* `frame_out` Output frame buffer (must hold at least 60 bytes). 
* `frame_len` Pointer to receive final Ethernet II frame length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_igmp\_init 

_Initialize IGMP Engine Context._ 
```C++
SYN_Status syn_igmp_init (
    SYN_IGMP * igmp
) 
```





**Parameters:**


* `igmp` Pointer to IGMP context. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_igmp\_join\_group 

_Join an IPv4 Multicast Group and construct IGMPv2 Join Frame._ 
```C++
SYN_Status syn_igmp_join_group (
    SYN_IGMP * igmp,
    SYN_ETH * eth,
    uint32_t group_ip,
    uint8_t * frame_out,
    size_t * frame_len
) 
```





**Parameters:**


* `igmp` Pointer to IGMP context. 
* `eth` Pointer to Ethernet context. 
* `group_ip` 32-bit IPv4 multicast address (e.g. 224.0.0.251 for mDNS). 
* `frame_out` Output frame buffer (must hold at least 60 bytes). 
* `frame_len` Pointer to receive final Ethernet II frame length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_igmp\_leave\_group 

_Leave an IPv4 Multicast Group and construct IGMPv2 Leave Frame._ 
```C++
SYN_Status syn_igmp_leave_group (
    SYN_IGMP * igmp,
    SYN_ETH * eth,
    uint32_t group_ip,
    uint8_t * frame_out,
    size_t * frame_len
) 
```





**Parameters:**


* `igmp` Pointer to IGMP context. 
* `eth` Pointer to Ethernet context. 
* `group_ip` 32-bit IPv4 multicast address. 
* `frame_out` Output frame buffer (must hold at least 60 bytes). 
* `frame_len` Pointer to receive final Ethernet II frame length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_igmp\_process\_packet 

_Process incoming IGMP packet (e.g. Membership Query from switch)._ 
```C++
SYN_Status syn_igmp_process_packet (
    SYN_IGMP * igmp,
    SYN_ETH * eth,
    const uint8_t * ip_pkt,
    size_t len,
    uint8_t * frame_tx,
    size_t * tx_len
) 
```





**Parameters:**


* `igmp` Pointer to IGMP context. 
* `eth` Pointer to Ethernet context. 
* `ip_pkt` Pointer to incoming IPv4 packet (starting at Ethernet frame). 
* `len` Length in bytes. 
* `frame_tx` Output frame buffer to receive reply report (if generated). 
* `tx_len` Pointer to receive byte length of generated reply frame. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_igmp.c`

