

# File syn\_igmp.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_igmp.h**](syn__igmp_8h.md)

[Go to the source code of this file](syn__igmp_8h_source.md)

_Zero-Heap Native IGMPv2 Protocol Engine._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/net/syn_eth.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_IGMP**](structSYN__IGMP.md) <br> |
| struct | [**SYN\_IGMP\_Group**](structSYN__IGMP__Group.md) <br> |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_igmp\_build\_report**](#function-syn_igmp_build_report) ([**SYN\_IGMP**](structSYN__IGMP.md) \* igmp, [**SYN\_ETH**](structSYN__ETH.md) \* eth, uint8\_t type, uint32\_t group\_ip, uint8\_t \* frame\_out, size\_t \* frame\_len) <br>_Build an IGMPv2 Membership Report (Join Group) or Leave frame over Ethernet._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_igmp\_init**](#function-syn_igmp_init) ([**SYN\_IGMP**](structSYN__IGMP.md) \* igmp) <br>_Initialize IGMP Engine Context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_igmp\_join\_group**](#function-syn_igmp_join_group) ([**SYN\_IGMP**](structSYN__IGMP.md) \* igmp, [**SYN\_ETH**](structSYN__ETH.md) \* eth, uint32\_t group\_ip, uint8\_t \* frame\_out, size\_t \* frame\_len) <br>_Join an IPv4 Multicast Group and construct IGMPv2 Join Frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_igmp\_leave\_group**](#function-syn_igmp_leave_group) ([**SYN\_IGMP**](structSYN__IGMP.md) \* igmp, [**SYN\_ETH**](structSYN__ETH.md) \* eth, uint32\_t group\_ip, uint8\_t \* frame\_out, size\_t \* frame\_len) <br>_Leave an IPv4 Multicast Group and construct IGMPv2 Leave Frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_igmp\_process\_packet**](#function-syn_igmp_process_packet) ([**SYN\_IGMP**](structSYN__IGMP.md) \* igmp, [**SYN\_ETH**](structSYN__ETH.md) \* eth, const uint8\_t \* ip\_pkt, size\_t len, uint8\_t \* frame\_tx, size\_t \* tx\_len) <br>_Process incoming IGMP packet (e.g. Membership Query from switch)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_IGMP\_MAX\_GROUPS**](syn__igmp_8h.md#define-syn_igmp_max_groups)  `4U`<br> |
| define  | [**SYN\_IGMP\_TYPE\_MEMBERSHIP\_QUERY**](syn__igmp_8h.md#define-syn_igmp_type_membership_query)  `0x11U`<br> |
| define  | [**SYN\_IGMP\_TYPE\_V2\_LEAVE**](syn__igmp_8h.md#define-syn_igmp_type_v2_leave)  `0x17U`<br> |
| define  | [**SYN\_IGMP\_TYPE\_V2\_REPORT**](syn__igmp_8h.md#define-syn_igmp_type_v2_report)  `0x16U`<br> |

## Detailed Description


Specifications:
* RFC 2236 Internet Group Management Protocol, Version 2
* IP Protocol Number: 2
* Zero dynamic memory allocation (0 bytes heap)
* Types:
  * 0x11: Membership Query
  * 0x16: IGMPv2 Membership Report (Join)
  * 0x17: IGMPv2 Leave Group 






    
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
## Macro Definition Documentation





### define SYN\_IGMP\_MAX\_GROUPS 

```C++
#define SYN_IGMP_MAX_GROUPS `4U`
```



Maximum concurrent multicast groups tracked per engine 


        

<hr>



### define SYN\_IGMP\_TYPE\_MEMBERSHIP\_QUERY 

```C++
#define SYN_IGMP_TYPE_MEMBERSHIP_QUERY `0x11U`
```



IGMP Membership Query message type (0x11) 


        

<hr>



### define SYN\_IGMP\_TYPE\_V2\_LEAVE 

```C++
#define SYN_IGMP_TYPE_V2_LEAVE `0x17U`
```



IGMPv2 Leave Group message type (0x17) 


        

<hr>



### define SYN\_IGMP\_TYPE\_V2\_REPORT 

```C++
#define SYN_IGMP_TYPE_V2_REPORT `0x16U`
```



IGMPv2 Membership Report message type (0x16) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_igmp.h`

