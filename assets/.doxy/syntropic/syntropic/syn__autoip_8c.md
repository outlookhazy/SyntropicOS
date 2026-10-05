

# File syn\_autoip.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_autoip.c**](syn__autoip_8c.md)

[Go to the source code of this file](syn__autoip_8c_source.md)

_Zero-Heap Native RFC 3927 AutoIP Engine Implementation._ 

* `#include "syntropic/net/syn_autoip.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_autoip\_build\_announce**](#function-syn_autoip_build_announce) ([**SYN\_AUTOIP**](structSYN__AUTOIP.md) \* autoip, const uint8\_t mac\_addr, uint8\_t \* buf\_out, size\_t \* len\_out) <br>_Build an ARP Announcement frame to announce claimed IP._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_autoip\_build\_probe**](#function-syn_autoip_build_probe) ([**SYN\_AUTOIP**](structSYN__AUTOIP.md) \* autoip, const uint8\_t mac\_addr, uint8\_t \* buf\_out, size\_t \* len\_out) <br>_Build an ARP Probe frame to verify IP availability._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_autoip\_init**](#function-syn_autoip_init) ([**SYN\_AUTOIP**](structSYN__AUTOIP.md) \* autoip, const uint8\_t mac\_addr) <br>_Initialize RFC 3927 AutoIP Engine Context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_autoip\_process\_arp**](#function-syn_autoip_process_arp) ([**SYN\_AUTOIP**](structSYN__AUTOIP.md) \* autoip, [**SYN\_ETH**](structSYN__ETH.md) \* eth, const uint8\_t \* arp\_frame, size\_t len) <br>_Process ARP response frame during probing._  |




























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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_autoip.c`

