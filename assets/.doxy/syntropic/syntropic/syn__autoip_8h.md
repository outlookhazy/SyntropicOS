

# File syn\_autoip.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_autoip.h**](syn__autoip_8h.md)

[Go to the source code of this file](syn__autoip_8h_source.md)

_Zero-Heap Native RFC 3927 AutoIP (Link-Local 169.254.x.x) Engine._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/net/syn_eth.h"`
* `#include "syntropic/pt/syn_pt.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_AUTOIP**](structSYN__AUTOIP.md) <br> |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_AUTOIP\_State**](#enum-syn_autoip_state)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_autoip\_build\_announce**](#function-syn_autoip_build_announce) ([**SYN\_AUTOIP**](structSYN__AUTOIP.md) \* autoip, const uint8\_t mac\_addr, uint8\_t \* buf\_out, size\_t \* len\_out) <br>_Build an ARP Announcement frame to announce claimed IP._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_autoip\_build\_probe**](#function-syn_autoip_build_probe) ([**SYN\_AUTOIP**](structSYN__AUTOIP.md) \* autoip, const uint8\_t mac\_addr, uint8\_t \* buf\_out, size\_t \* len\_out) <br>_Build an ARP Probe frame to verify IP availability._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_autoip\_init**](#function-syn_autoip_init) ([**SYN\_AUTOIP**](structSYN__AUTOIP.md) \* autoip, const uint8\_t mac\_addr) <br>_Initialize RFC 3927 AutoIP Engine Context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_autoip\_process\_arp**](#function-syn_autoip_process_arp) ([**SYN\_AUTOIP**](structSYN__AUTOIP.md) \* autoip, [**SYN\_ETH**](structSYN__ETH.md) \* eth, const uint8\_t \* arp\_frame, size\_t len) <br>_Process ARP response frame during probing._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**PT\_AUTOIP\_BLOCK\_BOUND**](syn__autoip_8h.md#define-pt_autoip_block_bound) (pt, task, autoip) `[**PT\_BLOCK\_CONDITION**](syn__pt_8h.md#define-pt_block_condition)(pt, task, (autoip)-&gt;state == SYN\_AUTOIP\_STATE\_BOUND)`<br>_Block task execution (SYN\_TASK\_BLOCKED) until AutoIP link-local binding completes._  |
| define  | [**PT\_AUTOIP\_WAIT\_BOUND**](syn__autoip_8h.md#define-pt_autoip_wait_bound) (pt, autoip) `[**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until)(pt, (autoip)-&gt;state == SYN\_AUTOIP\_STATE\_BOUND)`<br>_Block a protothread coroutine until AutoIP link-local binding completes._  |
| define  | [**SYN\_AUTOIP\_BASE\_IP**](syn__autoip_8h.md#define-syn_autoip_base_ip)  `0xA9FE0101UL`<br> |
| define  | [**SYN\_AUTOIP\_NETMASK**](syn__autoip_8h.md#define-syn_autoip_netmask)  `0xFFFF0000UL`<br> |

## Detailed Description


Specifications:
* RFC 3927 Dynamic Configuration of IPv4 Link-Local Addresses
* Address Range: 169.254.1.0 to 169.254.254.255
* Zero dynamic memory allocation (0 bytes heap) 




    
## Public Types Documentation




### enum SYN\_AUTOIP\_State 

```C++
enum SYN_AUTOIP_State {
    SYN_AUTOIP_STATE_INIT = 0,
    SYN_AUTOIP_STATE_PROBE,
    SYN_AUTOIP_STATE_ANNOUNCE,
    SYN_AUTOIP_STATE_BOUND
};
```



AutoIP State Machine Enum. 


        

<hr>
## Public Functions Documentation




### function syn\_autoip\_build\_announce 

_Build an ARP Announcement frame to announce claimed IP._ 
```C++
SYN_Status syn_autoip_build_announce (
    SYN_AUTOIP * autoip,
    const uint8_t mac_addr,
    uint8_t * buf_out,
    size_t * len_out
) 
```





**Parameters:**


* `autoip` Pointer to AutoIP context. 
* `mac_addr` 6-byte client MAC address. 
* `buf_out` Output frame buffer (must hold at least 60 bytes). 
* `len_out` Pointer to receive length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_autoip\_build\_probe 

_Build an ARP Probe frame to verify IP availability._ 
```C++
SYN_Status syn_autoip_build_probe (
    SYN_AUTOIP * autoip,
    const uint8_t mac_addr,
    uint8_t * buf_out,
    size_t * len_out
) 
```





**Parameters:**


* `autoip` Pointer to AutoIP context. 
* `mac_addr` 6-byte client MAC address. 
* `buf_out` Output frame buffer (must hold at least 60 bytes). 
* `len_out` Pointer to receive length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_autoip\_init 

_Initialize RFC 3927 AutoIP Engine Context._ 
```C++
SYN_Status syn_autoip_init (
    SYN_AUTOIP * autoip,
    const uint8_t mac_addr
) 
```





**Parameters:**


* `autoip` Pointer to AutoIP context. 
* `mac_addr` 6-byte client MAC address (used to seed candidate IP selection). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_autoip\_process\_arp 

_Process ARP response frame during probing._ 
```C++
SYN_Status syn_autoip_process_arp (
    SYN_AUTOIP * autoip,
    SYN_ETH * eth,
    const uint8_t * arp_frame,
    size_t len
) 
```





**Parameters:**


* `autoip` Pointer to AutoIP context. 
* `eth` Pointer to Ethernet context (auto-updated when BOUND). 
* `arp_frame` Pointer to received frame. 
* `len` Length in bytes. 



**Returns:**

SYN\_OK when BOUND, SYN\_BUSY if conflict detected and new candidate picked. 





        

<hr>
## Macro Definition Documentation





### define PT\_AUTOIP\_BLOCK\_BOUND 

_Block task execution (SYN\_TASK\_BLOCKED) until AutoIP link-local binding completes._ 
```C++
#define PT_AUTOIP_BLOCK_BOUND (
    pt,
    task,
    autoip
) `PT_BLOCK_CONDITION (pt, task, (autoip)->state == SYN_AUTOIP_STATE_BOUND )`
```





**Parameters:**


* `pt` Protothread context. 
* `task` Pointer to [**SYN\_Task**](structSYN__Task.md). 
* `autoip` Pointer to AutoIP context. 




        

<hr>



### define PT\_AUTOIP\_WAIT\_BOUND 

_Block a protothread coroutine until AutoIP link-local binding completes._ 
```C++
#define PT_AUTOIP_WAIT_BOUND (
    pt,
    autoip
) `PT_WAIT_UNTIL (pt, (autoip)->state == SYN_AUTOIP_STATE_BOUND )`
```





**Parameters:**


* `pt` Protothread context. 
* `autoip` Pointer to AutoIP context. 




        

<hr>



### define SYN\_AUTOIP\_BASE\_IP 

```C++
#define SYN_AUTOIP_BASE_IP `0xA9FE0101UL`
```



Link-local base IP address 169.254.1.1 


        

<hr>



### define SYN\_AUTOIP\_NETMASK 

```C++
#define SYN_AUTOIP_NETMASK `0xFFFF0000UL`
```



Link-local network mask 255.255.0.0 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_autoip.h`

