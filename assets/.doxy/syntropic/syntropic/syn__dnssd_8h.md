

# File syn\_dnssd.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_dnssd.h**](syn__dnssd_8h.md)

[Go to the source code of this file](syn__dnssd_8h_source.md)

_DNS-Based Service Discovery (DNS-SD - RFC 6763 / RFC 6762)._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_socket.h"`
* `#include "../pt/syn_pt.h"`
* `#include "../sched/syn_task.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_DnsSd**](structSYN__DnsSd.md) <br>_DNS-SD Daemon Context._  |
| struct | [**SYN\_DnsSd\_Browser**](structSYN__DnsSd__Browser.md) <br>_DNS-SD Service Discovery Browser Context._  |
| struct | [**SYN\_DnsSd\_Discovered**](structSYN__DnsSd__Discovered.md) <br>_Discovered Service Information received from mDNS response._  |
| struct | [**SYN\_DnsSd\_Service**](structSYN__DnsSd__Service.md) <br>_DNS-SD Service Definition._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_DnsSd\_DiscoverCallback**](#typedef-syn_dnssd_discovercallback)  <br>_Callback invoked when a remote service is discovered via DNS-SD._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dnssd\_announce**](#function-syn_dnssd_announce) (const [**SYN\_DnsSd**](structSYN__DnsSd.md) \* sd, size\_t service\_index, uint8\_t \* resp\_buf, size\_t max\_resp\_len, size\_t \* resp\_len) <br>_Format and send gratuitous multicast DNS-SD announcement (RFC 6762 §8.3)._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_dnssd\_browse\_task**](#function-syn_dnssd_browse_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative protothread coroutine for background DNS-SD browsing._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dnssd\_browser\_init**](#function-syn_dnssd_browser_init) ([**SYN\_DnsSd\_Browser**](structSYN__DnsSd__Browser.md) \* browser, const [**SYN\_DnsSd**](structSYN__DnsSd.md) \* sd, const char \* service\_type, [**SYN\_DnsSd\_DiscoverCallback**](syn__dnssd_8h.md#typedef-syn_dnssd_discovercallback) cb, void \* user\_data, uint32\_t timeout\_ms) <br>_Initialize and start a DNS-SD service browsing session._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dnssd\_build\_query**](#function-syn_dnssd_build_query) (const char \* service\_type, uint8\_t \* query\_buf, size\_t max\_len, size\_t \* query\_len) <br>_Build an mDNS service discovery query (RFC 6762 / RFC 6763)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dnssd\_discover**](#function-syn_dnssd_discover) (const [**SYN\_DnsSd**](structSYN__DnsSd.md) \* sd, const char \* service\_type) <br>_Send an mDNS discovery query for a specific service type on multicast UDP._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dnssd\_init**](#function-syn_dnssd_init) ([**SYN\_DnsSd**](structSYN__DnsSd.md) \* sd) <br>_Initialize DNS-SD daemon context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dnssd\_parse\_response**](#function-syn_dnssd_parse_response) (const uint8\_t \* resp\_buf, size\_t resp\_len, [**SYN\_DnsSd\_Discovered**](structSYN__DnsSd__Discovered.md) \* out\_disc) <br>_Parse an incoming mDNS response packet into a discovered service structure._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dnssd\_process\_query**](#function-syn_dnssd_process_query) (const [**SYN\_DnsSd**](structSYN__DnsSd.md) \* sd, const uint8\_t \* query\_buf, size\_t query\_len, uint8\_t \* resp\_buf, size\_t max\_resp\_len, size\_t \* resp\_len) <br>_Process an incoming mDNS / DNS-SD query packet and format response._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dnssd\_register**](#function-syn_dnssd_register) ([**SYN\_DnsSd**](structSYN__DnsSd.md) \* sd, const [**SYN\_DnsSd\_Service**](structSYN__DnsSd__Service.md) \* svc) <br>_Register a service for DNS-SD broadcast/discovery._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_dnssd\_task**](#function-syn_dnssd_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative protothread task for responding to DNS-SD discovery queries._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_DNSSD\_MAX\_SERVICES**](syn__dnssd_8h.md#define-syn_dnssd_max_services)  `4U`<br>_Maximum registered services per DNS-SD instance._  |
| define  | [**SYN\_DNSSD\_MAX\_TXT\_RECORDS**](syn__dnssd_8h.md#define-syn_dnssd_max_txt_records)  `8U`<br>_Maximum TXT key-value attributes per service._  |
| define  | [**SYN\_DNSSD\_PORT**](syn__dnssd_8h.md#define-syn_dnssd_port)  `5353U`<br>_Standard mDNS / DNS-SD Multicast Port._  |

## Detailed Description


Provides zero-allocation, cleanroom multicast DNS-SD service announcement and query responding for embedded devices (PTR, SRV, TXT, and A records). 


    
## Public Types Documentation




### typedef SYN\_DnsSd\_DiscoverCallback 

_Callback invoked when a remote service is discovered via DNS-SD._ 
```C++
typedef void(* SYN_DnsSd_DiscoverCallback) (const SYN_DnsSd_Discovered *service, void *user_data);
```





**Parameters:**


* `service` Pointer to the discovered service attributes. 
* `user_data` User context pointer passed to [**syn\_dnssd\_browser\_init()**](syn__dnssd_8h.md#function-syn_dnssd_browser_init). 




        

<hr>
## Public Functions Documentation




### function syn\_dnssd\_announce 

_Format and send gratuitous multicast DNS-SD announcement (RFC 6762 §8.3)._ 
```C++
SYN_Status syn_dnssd_announce (
    const SYN_DnsSd * sd,
    size_t service_index,
    uint8_t * resp_buf,
    size_t max_resp_len,
    size_t * resp_len
) 
```





**Parameters:**


* `sd` DNS-SD context. 
* `service_index` Index of service to announce. 
* `resp_buf` [out] Buffer to receive formatted response packet. 
* `max_resp_len` Capacity of resp\_buf. 
* `resp_len` [out] Number of bytes written to resp\_buf. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on invalid parameters or send failure. 





        

<hr>



### function syn\_dnssd\_browse\_task 

_Cooperative protothread coroutine for background DNS-SD browsing._ 
```C++
SYN_PT_Status syn_dnssd_browse_task (
    SYN_PT * pt,
    SYN_Task * task
) 
```



Yields until responses arrive or timeout expires. Parses incoming packets and invokes the discovery callback.




**Parameters:**


* `pt` Protothread pointer. 
* `task` Task descriptor with user\_data pointing to [**SYN\_DnsSd\_Browser**](structSYN__DnsSd__Browser.md). 



**Returns:**

PT\_WAITING while running, PT\_EXITED when discovery timeout elapses. 





        

<hr>



### function syn\_dnssd\_browser\_init 

_Initialize and start a DNS-SD service browsing session._ 
```C++
SYN_Status syn_dnssd_browser_init (
    SYN_DnsSd_Browser * browser,
    const SYN_DnsSd * sd,
    const char * service_type,
    SYN_DnsSd_DiscoverCallback cb,
    void * user_data,
    uint32_t timeout_ms
) 
```



Transmits the initial multicast PTR query and arms the browser context.




**Parameters:**


* `browser` Browser context instance. 
* `sd` Initialized DNS-SD daemon instance (provides socket). 
* `service_type` Service type to search for (e.g. "\_http.\_tcp", "\_coap.\_udp"). 
* `cb` Callback invoked whenever a valid response is received. 
* `user_data` Optional user context pointer. 
* `timeout_ms` Maximum time in ms to listen for discovery responses. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on invalid parameters. 





        

<hr>



### function syn\_dnssd\_build\_query 

_Build an mDNS service discovery query (RFC 6762 / RFC 6763)._ 
```C++
SYN_Status syn_dnssd_build_query (
    const char * service_type,
    uint8_t * query_buf,
    size_t max_len,
    size_t * query_len
) 
```



Formats a standard PTR query for `_service._proto.local`.




**Parameters:**


* `service_type` Service type to search for (e.g. "\_http.\_tcp", "\_coap.\_udp"). 
* `query_buf` [out] Output buffer for DNS query packet. 
* `max_len` Capacity of query\_buf. 
* `query_len` [out] Written length of query packet. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on buffer overflow or invalid arguments. 





        

<hr>



### function syn\_dnssd\_discover 

_Send an mDNS discovery query for a specific service type on multicast UDP._ 
```C++
SYN_Status syn_dnssd_discover (
    const SYN_DnsSd * sd,
    const char * service_type
) 
```





**Parameters:**


* `sd` DNS-SD context containing open multicast socket. 
* `service_type` Service type to query for (e.g. "\_http.\_tcp"). 



**Returns:**

SYN\_OK on successful multicast transmit, SYN\_ERROR otherwise. 





        

<hr>



### function syn\_dnssd\_init 

_Initialize DNS-SD daemon context._ 
```C++
SYN_Status syn_dnssd_init (
    SYN_DnsSd * sd
) 
```





**Parameters:**


* `sd` DNS-SD instance. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on socket/multicast error. 





        

<hr>



### function syn\_dnssd\_parse\_response 

_Parse an incoming mDNS response packet into a discovered service structure._ 
```C++
SYN_Status syn_dnssd_parse_response (
    const uint8_t * resp_buf,
    size_t resp_len,
    SYN_DnsSd_Discovered * out_disc
) 
```





**Parameters:**


* `resp_buf` Buffer containing raw DNS response. 
* `resp_len` Length of raw DNS response. 
* `out_disc` [out] Structure to receive parsed service attributes. 



**Returns:**

SYN\_OK if successfully parsed, SYN\_ERROR otherwise. 





        

<hr>



### function syn\_dnssd\_process\_query 

_Process an incoming mDNS / DNS-SD query packet and format response._ 
```C++
SYN_Status syn_dnssd_process_query (
    const SYN_DnsSd * sd,
    const uint8_t * query_buf,
    size_t query_len,
    uint8_t * resp_buf,
    size_t max_resp_len,
    size_t * resp_len
) 
```





**Parameters:**


* `sd` DNS-SD context containing registered services. 
* `query_buf` Incoming raw DNS packet. 
* `query_len` Query packet byte length. 
* `resp_buf` [out] Buffer to receive formatted response packet. 
* `max_resp_len` Capacity of resp\_buf. 
* `resp_len` [out] Number of bytes written to resp\_buf. 



**Returns:**

SYN\_OK if query matched and response generated, SYN\_ERROR/SYN\_NOT\_FOUND otherwise. 





        

<hr>



### function syn\_dnssd\_register 

_Register a service for DNS-SD broadcast/discovery._ 
```C++
SYN_Status syn_dnssd_register (
    SYN_DnsSd * sd,
    const SYN_DnsSd_Service * svc
) 
```





**Parameters:**


* `sd` DNS-SD instance. 
* `svc` Service configuration to register. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if table is full or invalid parameters. 





        

<hr>



### function syn\_dnssd\_task 

_Cooperative protothread task for responding to DNS-SD discovery queries._ 
```C++
SYN_PT_Status syn_dnssd_task (
    SYN_PT * pt,
    SYN_Task * task
) 
```





**Parameters:**


* `pt` Protothread pointer. 
* `task` Task structure with user\_data pointing to [**SYN\_DnsSd**](structSYN__DnsSd.md) instance. 



**Returns:**

PT\_WAITING while running, PT\_EXITED when done. 





        

<hr>
## Macro Definition Documentation





### define SYN\_DNSSD\_MAX\_SERVICES 

_Maximum registered services per DNS-SD instance._ 
```C++
#define SYN_DNSSD_MAX_SERVICES `4U`
```




<hr>



### define SYN\_DNSSD\_MAX\_TXT\_RECORDS 

_Maximum TXT key-value attributes per service._ 
```C++
#define SYN_DNSSD_MAX_TXT_RECORDS `8U`
```




<hr>



### define SYN\_DNSSD\_PORT 

_Standard mDNS / DNS-SD Multicast Port._ 
```C++
#define SYN_DNSSD_PORT `5353U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dnssd.h`

