

# File syn\_dhcp.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_dhcp.h**](syn__dhcp_8h.md)

[Go to the source code of this file](syn__dhcp_8h_source.md)

_Zero-Heap Native DHCP Client Protocol Engine._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/net/syn_eth.h"`
* `#include "syntropic/pt/syn_pt.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_DHCP**](structSYN__DHCP.md) <br> |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_DHCP\_State**](#enum-syn_dhcp_state)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dhcp\_build\_discover**](#function-syn_dhcp_build_discover) ([**SYN\_DHCP**](structSYN__DHCP.md) \* dhcp, const uint8\_t mac\_addr, uint8\_t \* buf\_out, size\_t buf\_size, size\_t \* len\_out) <br>_Build a DHCPDISCOVER UDP packet payload._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dhcp\_build\_request**](#function-syn_dhcp_build_request) ([**SYN\_DHCP**](structSYN__DHCP.md) \* dhcp, const uint8\_t mac\_addr, uint8\_t \* buf\_out, size\_t buf\_size, size\_t \* len\_out) <br>_Build a DHCPREQUEST UDP packet payload._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dhcp\_init**](#function-syn_dhcp_init) ([**SYN\_DHCP**](structSYN__DHCP.md) \* dhcp, uint32\_t xid) <br>_Initialize DHCP Client Context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dhcp\_process\_packet**](#function-syn_dhcp_process_packet) ([**SYN\_DHCP**](structSYN__DHCP.md) \* dhcp, [**SYN\_ETH**](structSYN__ETH.md) \* eth, const uint8\_t \* dhcp\_pkt, size\_t len) <br>_Process incoming DHCP response packet payload (DHCPOFFER or DHCPACK)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**PT\_DHCP\_BLOCK\_BOUND**](syn__dhcp_8h.md#define-pt_dhcp_block_bound) (pt, task, dhcp) `[**PT\_BLOCK\_CONDITION**](syn__pt_8h.md#define-pt_block_condition)(pt, task, (dhcp)-&gt;state == SYN\_DHCP\_STATE\_BOUND)`<br>_Block task execution (SYN\_TASK\_BLOCKED) until DHCP IP lease binding completes._  |
| define  | [**PT\_DHCP\_WAIT\_BOUND**](syn__dhcp_8h.md#define-pt_dhcp_wait_bound) (pt, dhcp) `[**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until)(pt, (dhcp)-&gt;state == SYN\_DHCP\_STATE\_BOUND)`<br>_Block a protothread coroutine until DHCP IP lease binding completes._  |
| define  | [**SYN\_DHCP\_ACK**](syn__dhcp_8h.md#define-syn_dhcp_ack)  `5U`<br> |
| define  | [**SYN\_DHCP\_CLIENT\_PORT**](syn__dhcp_8h.md#define-syn_dhcp_client_port)  `68U`<br> |
| define  | [**SYN\_DHCP\_DECLINE**](syn__dhcp_8h.md#define-syn_dhcp_decline)  `4U`<br> |
| define  | [**SYN\_DHCP\_DISCOVER**](syn__dhcp_8h.md#define-syn_dhcp_discover)  `1U`<br> |
| define  | [**SYN\_DHCP\_MAGIC\_COOKIE**](syn__dhcp_8h.md#define-syn_dhcp_magic_cookie)  `0x63825363UL`<br> |
| define  | [**SYN\_DHCP\_NAK**](syn__dhcp_8h.md#define-syn_dhcp_nak)  `6U`<br> |
| define  | [**SYN\_DHCP\_OFFER**](syn__dhcp_8h.md#define-syn_dhcp_offer)  `2U`<br> |
| define  | [**SYN\_DHCP\_OPT\_END**](syn__dhcp_8h.md#define-syn_dhcp_opt_end)  `255U`<br> |
| define  | [**SYN\_DHCP\_OPT\_LEASE\_TIME**](syn__dhcp_8h.md#define-syn_dhcp_opt_lease_time)  `51U`<br> |
| define  | [**SYN\_DHCP\_OPT\_MSG\_TYPE**](syn__dhcp_8h.md#define-syn_dhcp_opt_msg_type)  `53U`<br> |
| define  | [**SYN\_DHCP\_OPT\_ROUTER**](syn__dhcp_8h.md#define-syn_dhcp_opt_router)  `3U`<br> |
| define  | [**SYN\_DHCP\_OPT\_SUBNET\_MASK**](syn__dhcp_8h.md#define-syn_dhcp_opt_subnet_mask)  `1U`<br> |
| define  | [**SYN\_DHCP\_REQUEST**](syn__dhcp_8h.md#define-syn_dhcp_request)  `3U`<br> |
| define  | [**SYN\_DHCP\_SERVER\_PORT**](syn__dhcp_8h.md#define-syn_dhcp_server_port)  `67U`<br> |

## Detailed Description


Specifications:
* RFC 2131 Dynamic Host Configuration Protocol
* UDP Port 68 (Client) &lt;-&gt; Port 67 (Server)
* Zero dynamic memory allocation (0 bytes heap)
* DHCP Option parsing: Subnet Mask (1), Gateway (3), Lease Time (51), Msg Type (53) 




    
## Public Types Documentation




### enum SYN\_DHCP\_State 

```C++
enum SYN_DHCP_State {
    SYN_DHCP_STATE_INIT = 0,
    SYN_DHCP_STATE_DISCOVER,
    SYN_DHCP_STATE_OFFER,
    SYN_DHCP_STATE_REQUEST,
    SYN_DHCP_STATE_BOUND
};
```



DHCP Client State Machine Enum. 


        

<hr>
## Public Functions Documentation




### function syn\_dhcp\_build\_discover 

_Build a DHCPDISCOVER UDP packet payload._ 
```C++
SYN_Status syn_dhcp_build_discover (
    SYN_DHCP * dhcp,
    const uint8_t mac_addr,
    uint8_t * buf_out,
    size_t buf_size,
    size_t * len_out
) 
```





**Parameters:**


* `dhcp` Pointer to DHCP context. 
* `mac_addr` 6-byte client MAC address. 
* `buf_out` Output buffer (must hold at least 250 bytes). 
* `buf_size` Output buffer capacity. 
* `len_out` Pointer to receive byte length of generated payload. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_dhcp\_build\_request 

_Build a DHCPREQUEST UDP packet payload._ 
```C++
SYN_Status syn_dhcp_build_request (
    SYN_DHCP * dhcp,
    const uint8_t mac_addr,
    uint8_t * buf_out,
    size_t buf_size,
    size_t * len_out
) 
```





**Parameters:**


* `dhcp` Pointer to DHCP context. 
* `mac_addr` 6-byte client MAC address. 
* `buf_out` Output buffer (must hold at least 260 bytes). 
* `buf_size` Output buffer capacity. 
* `len_out` Pointer to receive byte length of generated payload. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_dhcp\_init 

_Initialize DHCP Client Context._ 
```C++
SYN_Status syn_dhcp_init (
    SYN_DHCP * dhcp,
    uint32_t xid
) 
```





**Parameters:**


* `dhcp` Pointer to DHCP context. 
* `xid` 32-bit transaction ID (e.g. random value or tick count). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_dhcp\_process\_packet 

_Process incoming DHCP response packet payload (DHCPOFFER or DHCPACK)._ 
```C++
SYN_Status syn_dhcp_process_packet (
    SYN_DHCP * dhcp,
    SYN_ETH * eth,
    const uint8_t * dhcp_pkt,
    size_t len
) 
```





**Parameters:**


* `dhcp` Pointer to DHCP context. 
* `eth` Pointer to Ethernet context (updated automatically when BOUND). 
* `dhcp_pkt` Pointer to incoming DHCP UDP payload. 
* `len` Payload byte length. 



**Returns:**

SYN\_OK on success, SYN\_BUSY if still waiting/in-progress. 





        

<hr>
## Macro Definition Documentation





### define PT\_DHCP\_BLOCK\_BOUND 

_Block task execution (SYN\_TASK\_BLOCKED) until DHCP IP lease binding completes._ 
```C++
#define PT_DHCP_BLOCK_BOUND (
    pt,
    task,
    dhcp
) `PT_BLOCK_CONDITION (pt, task, (dhcp)->state == SYN_DHCP_STATE_BOUND)`
```





**Parameters:**


* `pt` Protothread context. 
* `task` Pointer to [**SYN\_Task**](structSYN__Task.md). 
* `dhcp` Pointer to DHCP context. 




        

<hr>



### define PT\_DHCP\_WAIT\_BOUND 

_Block a protothread coroutine until DHCP IP lease binding completes._ 
```C++
#define PT_DHCP_WAIT_BOUND (
    pt,
    dhcp
) `PT_WAIT_UNTIL (pt, (dhcp)->state == SYN_DHCP_STATE_BOUND)`
```





**Parameters:**


* `pt` Protothread context. 
* `dhcp` Pointer to DHCP context. 




        

<hr>



### define SYN\_DHCP\_ACK 

```C++
#define SYN_DHCP_ACK `5U`
```



DHCP Message Type: ACK (5) 


        

<hr>



### define SYN\_DHCP\_CLIENT\_PORT 

```C++
#define SYN_DHCP_CLIENT_PORT `68U`
```



DHCP UDP client port (68) 


        

<hr>



### define SYN\_DHCP\_DECLINE 

```C++
#define SYN_DHCP_DECLINE `4U`
```



DHCP Message Type: DECLINE (4) 


        

<hr>



### define SYN\_DHCP\_DISCOVER 

```C++
#define SYN_DHCP_DISCOVER `1U`
```



DHCP Message Type: DISCOVER (1) 


        

<hr>



### define SYN\_DHCP\_MAGIC\_COOKIE 

```C++
#define SYN_DHCP_MAGIC_COOKIE `0x63825363UL`
```



DHCP options magic cookie (99.130.83.99) 


        

<hr>



### define SYN\_DHCP\_NAK 

```C++
#define SYN_DHCP_NAK `6U`
```



DHCP Message Type: NAK (6) 


        

<hr>



### define SYN\_DHCP\_OFFER 

```C++
#define SYN_DHCP_OFFER `2U`
```



DHCP Message Type: OFFER (2) 


        

<hr>



### define SYN\_DHCP\_OPT\_END 

```C++
#define SYN_DHCP_OPT_END `255U`
```



DHCP Option 255: End Marker 


        

<hr>



### define SYN\_DHCP\_OPT\_LEASE\_TIME 

```C++
#define SYN_DHCP_OPT_LEASE_TIME `51U`
```



DHCP Option 51: IP Address Lease Time 


        

<hr>



### define SYN\_DHCP\_OPT\_MSG\_TYPE 

```C++
#define SYN_DHCP_OPT_MSG_TYPE `53U`
```



DHCP Option 53: DHCP Message Type 


        

<hr>



### define SYN\_DHCP\_OPT\_ROUTER 

```C++
#define SYN_DHCP_OPT_ROUTER `3U`
```



DHCP Option 3: Router/Gateway IP 


        

<hr>



### define SYN\_DHCP\_OPT\_SUBNET\_MASK 

```C++
#define SYN_DHCP_OPT_SUBNET_MASK `1U`
```



DHCP Option 1: Subnet Mask 


        

<hr>



### define SYN\_DHCP\_REQUEST 

```C++
#define SYN_DHCP_REQUEST `3U`
```



DHCP Message Type: REQUEST (3) 


        

<hr>



### define SYN\_DHCP\_SERVER\_PORT 

```C++
#define SYN_DHCP_SERVER_PORT `67U`
```



DHCP UDP server port (67) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dhcp.h`

