

# File syn\_ntp\_server.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_ntp\_server.h**](syn__ntp__server_8h.md)

[Go to the source code of this file](syn__ntp__server_8h_source.md)

_Zero-Heap NTP v4 Server Protocol Engine over UDP port 123._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/port/syn_port_socket.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_NTPServer**](structSYN__NTPServer.md) <br> |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ntp\_server\_init**](#function-syn_ntp_server_init) ([**SYN\_NTPServer**](structSYN__NTPServer.md) \* server, uint8\_t stratum, uint32\_t(\*)(void) get\_epoch\_sec\_cb) <br>_Initialize NTP Server Instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ntp\_server\_process\_packet**](#function-syn_ntp_server_process_packet) ([**SYN\_NTPServer**](structSYN__NTPServer.md) \* server, const uint8\_t request\_pkt, uint8\_t response\_pkt) <br>_Process an incoming 48-byte NTP client request packet and format response._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_NTP\_EPOCH\_OFFSET**](syn__ntp__server_8h.md#define-syn_ntp_epoch_offset)  `2208988800UL`<br> |
| define  | [**SYN\_NTP\_PACKET\_LEN**](syn__ntp__server_8h.md#define-syn_ntp_packet_len)  `48U`<br> |
| define  | [**SYN\_NTP\_SERVER\_PORT**](syn__ntp__server_8h.md#define-syn_ntp_server_port)  `123U`<br> |

## Detailed Description


Implements a lightweight NTP v4 (RFC 5905) time server allowing SyntropicOS microcontrollers acting as network gateways to serve time synchronization to local network nodes using system RTC or GPS time. 


    
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
## Macro Definition Documentation





### define SYN\_NTP\_EPOCH\_OFFSET 

```C++
#define SYN_NTP_EPOCH_OFFSET `2208988800UL`
```



Seconds offset between 1900 and 1970 UTC epochs 


        

<hr>



### define SYN\_NTP\_PACKET\_LEN 

```C++
#define SYN_NTP_PACKET_LEN `48U`
```



Standard 48-byte NTP Packet Length 


        

<hr>



### define SYN\_NTP\_SERVER\_PORT 

```C++
#define SYN_NTP_SERVER_PORT `123U`
```



Standard NTP UDP Server Port (123) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_ntp_server.h`

