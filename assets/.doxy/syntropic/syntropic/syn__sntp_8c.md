

# File syn\_sntp.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_sntp.c**](syn__sntp_8c.md)

[Go to the source code of this file](syn__sntp_8c_source.md)

_SNTP client implementation — RFC 4330 subset._ 

* `#include "syn_sntp.h"`
* `#include "../util/syn_assert.h"`
* `#include "../port/syn_port_system.h"`
* `#include <string.h>`
* `#include <stdio.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  int32\_t | [**syn\_sntp\_get\_drift\_ppm**](#function-syn_sntp_get_drift_ppm) (const [**SYN\_SNTP**](structSYN__SNTP.md) \* sntp) <br>_Get the calculated hardware clock drift in Parts-Per-Million (PPM)._  |
|  uint32\_t | [**syn\_sntp\_get\_epoch\_ns**](#function-syn_sntp_get_epoch_ns) (const [**SYN\_SNTP**](structSYN__SNTP.md) \* sntp) <br>_Get current nanosecond component of the timestamp._  |
|  uint32\_t | [**syn\_sntp\_get\_epoch\_s**](#function-syn_sntp_get_epoch_s) (const [**SYN\_SNTP**](structSYN__SNTP.md) \* sntp) <br>_Get current UTC epoch in seconds._  |
|  void | [**syn\_sntp\_init**](#function-syn_sntp_init) ([**SYN\_SNTP**](structSYN__SNTP.md) \* sntp, const [**SYN\_SockAddr**](structSYN__SockAddr.md) \* server, uint32\_t sync\_interval\_s) <br>_Initialize the SNTP client._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sntp\_query**](#function-syn_sntp_query) ([**SYN\_SNTP**](structSYN__SNTP.md) \* sntp) <br>_Perform a single blocking NTP query._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_sntp\_task**](#function-syn_sntp_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative protothread task for periodic NTP sync._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**load32\_be**](#function-load32_be) (const uint8\_t \* p) <br>_Read a big-endian uint32 from a byte buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**sntp\_parse\_packet**](#function-sntp_parse_packet) ([**SYN\_SNTP**](structSYN__SNTP.md) \* sntp, const uint8\_t \* pkt, size\_t len) <br>_Parse an SNTP response packet and extract the transmit timestamp._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**sntp\_send\_request**](#function-sntp_send_request) (const [**SYN\_SNTP**](structSYN__SNTP.md) \* sntp, [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock) <br>_Build and send an SNTP request on an already-open socket._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**sntp\_try\_recv**](#function-sntp_try_recv) ([**SYN\_SNTP**](structSYN__SNTP.md) \* sntp, [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock) <br>_Non-blocking receive: try to read an NTP response._  |


























## Public Functions Documentation




### function syn\_sntp\_get\_drift\_ppm 

_Get the calculated hardware clock drift in Parts-Per-Million (PPM)._ 
```C++
int32_t syn_sntp_get_drift_ppm (
    const SYN_SNTP * sntp
) 
```



Calculated automatically on subsequent NTP syncs by comparing network time elapsed against local hardware timer elapsed.




**Parameters:**


* `sntp` Client context. 



**Returns:**

Calculated drift in PPM (+ = local clock runs fast, - = slow). 





        

<hr>



### function syn\_sntp\_get\_epoch\_ns 

_Get current nanosecond component of the timestamp._ 
```C++
uint32_t syn_sntp_get_epoch_ns (
    const SYN_SNTP * sntp
) 
```



Derived from the tick counter sub-second remainder + NTP fractional seconds from the last sync. Approximate — not sub-ms accurate.




**Parameters:**


* `sntp` Client context. 



**Returns:**

Nanoseconds (0–999999999), or 0 if unsynced. 





        

<hr>



### function syn\_sntp\_get\_epoch\_s 

_Get current UTC epoch in seconds._ 
```C++
uint32_t syn_sntp_get_epoch_s (
    const SYN_SNTP * sntp
) 
```



Computed as: last\_sync\_epoch + (current\_tick - sync\_tick) / 1000. Returns 0 if not yet synced.




**Parameters:**


* `sntp` Client context. 



**Returns:**

Current epoch seconds, or 0 if unsynced. 





        

<hr>



### function syn\_sntp\_init 

_Initialize the SNTP client._ 
```C++
void syn_sntp_init (
    SYN_SNTP * sntp,
    const SYN_SockAddr * server,
    uint32_t sync_interval_s
) 
```





**Parameters:**


* `sntp` Client context. 
* `server` NTP server address (typically port 123). 
* `sync_interval_s` Seconds between re-syncs (e.g. 3600 for hourly). 




        

<hr>



### function syn\_sntp\_query 

_Perform a single blocking NTP query._ 
```C++
SYN_Status syn_sntp_query (
    SYN_SNTP * sntp
) 
```



Opens a UDP socket, sends a request, waits for the response, and updates the internal epoch. Closes the socket when done.




**Parameters:**


* `sntp` Client context. 



**Returns:**

SYN\_OK on success, SYN\_TIMEOUT or SYN\_ERROR on failure. 





        

<hr>



### function syn\_sntp\_task 

_Cooperative protothread task for periodic NTP sync._ 
```C++
SYN_PT_Status syn_sntp_task (
    SYN_PT * pt,
    SYN_Task * task
) 
```



Syncs on startup, then re-syncs every sync\_interval\_s seconds. Pass the [**SYN\_SNTP**](structSYN__SNTP.md) context via task-&gt;user\_data.




**Parameters:**


* `pt` Protothread. 
* `task` Task descriptor. 



**Returns:**

PT status (PT\_WAITING, PT\_YIELDED, or PT\_ENDED). 





        

<hr>
## Public Static Functions Documentation




### function load32\_be 

_Read a big-endian uint32 from a byte buffer._ 
```C++
static inline uint32_t load32_be (
    const uint8_t * p
) 
```





**Parameters:**


* `p` Source bytes. 



**Returns:**

32-bit value. 





        

<hr>



### function sntp\_parse\_packet 

_Parse an SNTP response packet and extract the transmit timestamp._ 
```C++
static SYN_Status sntp_parse_packet (
    SYN_SNTP * sntp,
    const uint8_t * pkt,
    size_t len
) 
```





**Parameters:**


* `sntp` SNTP client instance. 
* `pkt` Raw UDP payload. 
* `len` Payload length in bytes. 



**Returns:**

SYN\_OK on success, SYN\_BUSY if packet too short, SYN\_ERROR on invalid response. 





        

<hr>



### function sntp\_send\_request 

_Build and send an SNTP request on an already-open socket._ 
```C++
static SYN_Status sntp_send_request (
    const SYN_SNTP * sntp,
    SYN_Socket sock
) 
```





**Parameters:**


* `sntp` SNTP context (for server address). 
* `sock` Open UDP socket. 



**Returns:**

SYN\_OK if sent, SYN\_ERROR on failure. 





        

<hr>



### function sntp\_try\_recv 

_Non-blocking receive: try to read an NTP response._ 
```C++
static SYN_Status sntp_try_recv (
    SYN_SNTP * sntp,
    SYN_Socket sock
) 
```



Calls recvfrom with timeout\_ms = 0 (non-blocking). If a valid response is available, parses it and sets sntp-&gt;synced = true.




**Parameters:**


* `sntp` SNTP context (updated on success). 
* `sock` Open UDP socket. 



**Returns:**

SYN\_OK if synced, SYN\_BUSY if no data yet, SYN\_ERROR on bad response. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_sntp.c`

