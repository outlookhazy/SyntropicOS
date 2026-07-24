

# File syn\_sntp.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_sntp.h**](syn__sntp_8h.md)

[Go to the source code of this file](syn__sntp_8h_source.md)

_SNTP client — lightweight time synchronization over UDP._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_socket.h"`
* `#include "../pt/syn_pt.h"`
* `#include "../sched/syn_task.h"`
* `#include "../util/syn_backoff.h"`
* `#include "../dsp/syn_filter.h"`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_SNTP**](structSYN__SNTP.md) <br>_SNTP client context — caller-owned._  |






















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
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_sntp\_is\_synced**](#function-syn_sntp_is_synced) (const [**SYN\_SNTP**](structSYN__SNTP.md) \* sntp) <br>_Check if the client has successfully synced at least once._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_SNTP\_EPOCH\_OFFSET**](syn__sntp_8h.md#define-syn_sntp_epoch_offset)  `2208988800UL`<br> |
| define  | [**SYN\_SNTP\_MAX\_RETRIES**](syn__sntp_8h.md#define-syn_sntp_max_retries)  `3`<br> |
| define  | [**SYN\_SNTP\_PACKET\_SIZE**](syn__sntp_8h.md#define-syn_sntp_packet_size)  `48`<br> |
| define  | [**SYN\_SNTP\_TIMEOUT\_MS**](syn__sntp_8h.md#define-syn_sntp_timeout_ms)  `3000`<br> |

## Detailed Description


Implements a Simple NTP (RFC 4330) client that queries a single NTP server and maintains a local epoch clock offset. The client runs as a cooperative protothread task, periodically re-syncing.


After the first successful sync, [**syn\_sntp\_get\_epoch\_s()**](syn__sntp_8h.md#function-syn_sntp_get_epoch_s) returns a real UTC epoch timestamp derived from the last NTP response plus local tick elapsed since then.


Used by the WireGuard module for TAI64N handshake timestamps.




**
**


```C++
static SYN_SNTP sntp;
static SYN_SockAddr ntp_server = { .ip = {216,239,35,0}, .port = 123 };

syn_sntp_init(&sntp, &ntp_server, 3600);  // re-sync every hour

// Register as a scheduler task:
syn_task_create(&tasks[0], "sntp", syn_sntp_task, 2, &sntp);

// Later, read the time:
if (syn_sntp_is_synced(&sntp)) {
    uint32_t now = syn_sntp_get_epoch_s(&sntp);
}
```
 





    
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




### function syn\_sntp\_is\_synced 

_Check if the client has successfully synced at least once._ 
```C++
static inline bool syn_sntp_is_synced (
    const SYN_SNTP * sntp
) 
```





**Parameters:**


* `sntp` Client context. 



**Returns:**

true if synced. 





        

<hr>
## Macro Definition Documentation





### define SYN\_SNTP\_EPOCH\_OFFSET 

```C++
#define SYN_SNTP_EPOCH_OFFSET `2208988800UL`
```



NTP epoch offset: seconds between 1900-01-01 and 1970-01-01. 


        

<hr>



### define SYN\_SNTP\_MAX\_RETRIES 

```C++
#define SYN_SNTP_MAX_RETRIES `3`
```



Maximum retry attempts per sync cycle. 


        

<hr>



### define SYN\_SNTP\_PACKET\_SIZE 

```C++
#define SYN_SNTP_PACKET_SIZE `48`
```



NTP packet size (both request and response). 


        

<hr>



### define SYN\_SNTP\_TIMEOUT\_MS 

```C++
#define SYN_SNTP_TIMEOUT_MS `3000`
```



Default receive timeout for NTP response (ms). 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_sntp.h`

