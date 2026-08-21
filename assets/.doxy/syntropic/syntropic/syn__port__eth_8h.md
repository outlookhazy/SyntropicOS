

# File syn\_port\_eth.h



[**FileList**](files.md) **>** [**port**](dir_fd8b95245ffcce776715f180c056b450.md) **>** [**syn\_port\_eth.h**](syn__port__eth_8h.md)

[Go to the source code of this file](syn__port__eth_8h_source.md)

_Ethernet HAL hardware port interface contract._ 

* `#include "syntropic/common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_eth\_init**](#function-syn_port_eth_init) (const uint8\_t mac\_addr) <br>_Initialize low-level Ethernet MAC hardware (RMII/MII/SPI PHY)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_eth\_rx**](#function-syn_port_eth_rx) (void \* buf, size\_t max\_len, size\_t \* out\_len) <br>_Receive raw Ethernet frame from PHY buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_eth\_tx**](#function-syn_port_eth_tx) (const void \* frame, size\_t len) <br>_Transmit raw Ethernet frame over PHY._  |




























## Public Functions Documentation




### function syn\_port\_eth\_init 

_Initialize low-level Ethernet MAC hardware (RMII/MII/SPI PHY)._ 
```C++
SYN_Status syn_port_eth_init (
    const uint8_t mac_addr
) 
```





**Parameters:**


* `mac_addr` 6-byte hardware MAC address. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_eth\_rx 

_Receive raw Ethernet frame from PHY buffer._ 
```C++
SYN_Status syn_port_eth_rx (
    void * buf,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `buf` Pointer to destination buffer. 
* `max_len` Max capacity (must be &gt;= 1514 bytes). 
* `out_len` Pointer to receive frame byte length. 



**Returns:**

SYN\_OK on success, SYN\_BUSY if no frame available. 





        

<hr>



### function syn\_port\_eth\_tx 

_Transmit raw Ethernet frame over PHY._ 
```C++
SYN_Status syn_port_eth_tx (
    const void * frame,
    size_t len
) 
```





**Parameters:**


* `frame` Pointer to frame bytes. 
* `len` Frame length (60..1514 bytes). 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/port/syn_port_eth.h`

