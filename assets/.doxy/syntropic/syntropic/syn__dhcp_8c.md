

# File syn\_dhcp.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_dhcp.c**](syn__dhcp_8c.md)

[Go to the source code of this file](syn__dhcp_8c_source.md)

_Zero-Heap Native DHCP Client Protocol Engine Implementation._ 

* `#include "syntropic/net/syn_dhcp.h"`
* `#include <string.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_DhcpParsedOpts**](structSYN__DhcpParsedOpts.md) <br>_Parsed DHCP option values extracted from a BOOTP options block._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dhcp\_build\_discover**](#function-syn_dhcp_build_discover) ([**SYN\_DHCP**](structSYN__DHCP.md) \* dhcp, const uint8\_t mac\_addr, uint8\_t \* buf\_out, size\_t buf\_size, size\_t \* len\_out) <br>_Build a DHCPDISCOVER UDP packet payload._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dhcp\_build\_request**](#function-syn_dhcp_build_request) ([**SYN\_DHCP**](structSYN__DHCP.md) \* dhcp, const uint8\_t mac\_addr, uint8\_t \* buf\_out, size\_t buf\_size, size\_t \* len\_out) <br>_Build a DHCPREQUEST UDP packet payload._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dhcp\_init**](#function-syn_dhcp_init) ([**SYN\_DHCP**](structSYN__DHCP.md) \* dhcp, uint32\_t xid) <br>_Initialize DHCP Client Context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dhcp\_process\_packet**](#function-syn_dhcp_process_packet) ([**SYN\_DHCP**](structSYN__DHCP.md) \* dhcp, [**SYN\_ETH**](structSYN__ETH.md) \* eth, const uint8\_t \* dhcp\_pkt, size\_t len) <br>_Process incoming DHCP response packet payload (DHCPOFFER or DHCPACK)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**dhcp\_apply\_message**](#function-dhcp_apply_message) ([**SYN\_DHCP**](structSYN__DHCP.md) \* dhcp, [**SYN\_ETH**](structSYN__ETH.md) \* eth, uint32\_t yiaddr, const [**SYN\_DhcpParsedOpts**](structSYN__DhcpParsedOpts.md) \* opts) <br>_Apply parsed DHCP message semantics (OFFER, ACK, or NAK) to client state._  |
|  void | [**dhcp\_apply\_option**](#function-dhcp_apply_option) ([**SYN\_DhcpParsedOpts**](structSYN__DhcpParsedOpts.md) \* opts, uint8\_t opt, uint8\_t opt\_len, const uint8\_t \* val) <br>_Decode a single TLV DHCP option into_ [_**SYN\_DhcpParsedOpts**_](structSYN__DhcpParsedOpts.md) _._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dhcp\_reply\_valid**](#function-dhcp_reply_valid) (const [**SYN\_DHCP**](structSYN__DHCP.md) \* dhcp, const uint8\_t \* dhcp\_pkt, size\_t len) <br>_Validate fixed BOOTP reply header fields, magic cookie, and transaction ID._  |
|  void | [**pack\_uint32**](#function-pack_uint32) (uint8\_t \* buf, uint32\_t val) <br>_Pack 32-bit integer in big-endian byte order._  |
|  uint32\_t | [**unpack\_uint32**](#function-unpack_uint32) (const uint8\_t \* buf) <br>_Unpack 32-bit integer from big-endian byte order._  |


























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
## Public Static Functions Documentation




### function dhcp\_apply\_message 

_Apply parsed DHCP message semantics (OFFER, ACK, or NAK) to client state._ 
```C++
static SYN_Status dhcp_apply_message (
    SYN_DHCP * dhcp,
    SYN_ETH * eth,
    uint32_t yiaddr,
    const SYN_DhcpParsedOpts * opts
) 
```





**Parameters:**


* `dhcp` DHCP client context. 
* `eth` Optional Ethernet interface context to update or clear. 
* `yiaddr` Offered/assigned IPv4 address from BOOTP yiaddr field. 
* `opts` Decoded DHCP options block. 



**Returns:**

SYN\_OK on BOUND, SYN\_INVALID\_PARAM on zero-IP ACK, or SYN\_BUSY otherwise. 





        

<hr>



### function dhcp\_apply\_option 

_Decode a single TLV DHCP option into_ [_**SYN\_DhcpParsedOpts**_](structSYN__DhcpParsedOpts.md) _._
```C++
static void dhcp_apply_option (
    SYN_DhcpParsedOpts * opts,
    uint8_t opt,
    uint8_t opt_len,
    const uint8_t * val
) 
```





**Parameters:**


* `opts` Parsed options accumulator. 
* `opt` Option code. 
* `opt_len` Option payload length in bytes. 
* `val` Pointer to option payload bytes. 




        

<hr>



### function dhcp\_reply\_valid 

_Validate fixed BOOTP reply header fields, magic cookie, and transaction ID._ 
```C++
static bool dhcp_reply_valid (
    const SYN_DHCP * dhcp,
    const uint8_t * dhcp_pkt,
    size_t len
) 
```





**Parameters:**


* `dhcp` DHCP client state. 
* `dhcp_pkt` Raw DHCP UDP payload. 
* `len` Payload length in bytes. 



**Returns:**

true if the packet is a valid BOOTREPLY for this client transaction. 





        

<hr>



### function pack\_uint32 

_Pack 32-bit integer in big-endian byte order._ 
```C++
static void pack_uint32 (
    uint8_t * buf,
    uint32_t val
) 
```





**Parameters:**


* `buf` Output byte buffer. 
* `val` 32-bit value to pack. 




        

<hr>



### function unpack\_uint32 

_Unpack 32-bit integer from big-endian byte order._ 
```C++
static uint32_t unpack_uint32 (
    const uint8_t * buf
) 
```





**Parameters:**


* `buf` Input byte buffer. 



**Returns:**

Unpacked 32-bit integer. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dhcp.c`

