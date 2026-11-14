

# File syn\_ntp\_server.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_ntp\_server.c**](syn__ntp__server_8c.md)

[Go to the source code of this file](syn__ntp__server_8c_source.md)

_Zero-Heap NTP v4 Server Implementation._ 

* `#include "syn_ntp_server.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ntp\_server\_init**](#function-syn_ntp_server_init) ([**SYN\_NTPServer**](structSYN__NTPServer.md) \* server, uint8\_t stratum, uint32\_t(\*)(void) get\_epoch\_sec\_cb) <br>_Initialize NTP Server Instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ntp\_server\_process\_packet**](#function-syn_ntp_server_process_packet) ([**SYN\_NTPServer**](structSYN__NTPServer.md) \* server, const uint8\_t request\_pkt, uint8\_t response\_pkt) <br>_Process an incoming 48-byte NTP client request packet and format response._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**write\_u32\_be**](#function-write_u32_be) (uint8\_t \* buf, uint32\_t val) <br>_Write 32-bit integer into big-endian byte buffer._  |


























## Public Functions Documentation




### function syn\_ntp\_server\_init 

_Initialize NTP Server Instance._ 
```C++
SYN_Status syn_ntp_server_init (
    SYN_NTPServer * server,
    uint8_t stratum,
    uint32_t(*)(void) get_epoch_sec_cb
) 
```





**Parameters:**


* `server` Pointer to NTP server instance. 
* `stratum` Server stratum level (e.g. 1 for GPS, 2 for SNTP relay). 
* `get_epoch_sec_cb` Callback returning current UTC epoch seconds. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL. 





        

<hr>



### function syn\_ntp\_server\_process\_packet 

_Process an incoming 48-byte NTP client request packet and format response._ 
```C++
SYN_Status syn_ntp_server_process_packet (
    SYN_NTPServer * server,
    const uint8_t request_pkt,
    uint8_t response_pkt
) 
```





**Parameters:**


* `server` Pointer to NTP server context. 
* `request_pkt` Raw 48-byte input request packet. 
* `response_pkt` Output 48-byte response packet buffer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL or invalid. 





        

<hr>
## Public Static Functions Documentation




### function write\_u32\_be 

_Write 32-bit integer into big-endian byte buffer._ 
```C++
static void write_u32_be (
    uint8_t * buf,
    uint32_t val
) 
```





**Parameters:**


* `buf` Pointer to 4-byte destination buffer. 
* `val` 32-bit unsigned integer value. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_ntp_server.c`

