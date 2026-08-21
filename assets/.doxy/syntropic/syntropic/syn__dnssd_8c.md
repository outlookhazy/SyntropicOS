

# File syn\_dnssd.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_dnssd.c**](syn__dnssd_8c.md)

[Go to the source code of this file](syn__dnssd_8c_source.md)

_DNS-Based Service Discovery (DNS-SD - RFC 6763 / RFC 6762) implementation._ 

* `#include "syn_dnssd.h"`
* `#include "../port/syn_port_system.h"`
* `#include "../util/syn_assert.h"`
* `#include <stdio.h>`
* `#include <string.h>`





































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


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**match\_qname\_service**](#function-match_qname_service) (const uint8\_t \* buf, size\_t buf\_len, size\_t \* offset, const char \* service\_type) <br>_Match DNS question name against registered service type._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**pack\_service\_response**](#function-pack_service_response) (const [**SYN\_DnsSd\_Service**](structSYN__DnsSd__Service.md) \* svc, uint8\_t \* resp\_buf, size\_t max\_resp\_len, size\_t \* resp\_len) <br>_Pack DNS-SD response packet for a service (PTR, SRV, TXT, A records)._  |
|  void | [**skip\_or\_read\_name**](#function-skip_or_read_name) (const uint8\_t \* buf, size\_t buf\_len, size\_t \* pos, char \* out, size\_t max\_out) <br>_Helper to skip or extract a DNS domain name (handling pointers)._  |
|  void | [**write\_hostname\_fqdn**](#function-write_hostname_fqdn) (uint8\_t \* buf, size\_t \* pos, const char \* hostname) <br>_Write DNS hostname FQDN to buffer._  |
|  void | [**write\_instance\_fqdn**](#function-write_instance_fqdn) (uint8\_t \* buf, size\_t \* pos, const char \* instance, const char \* service\_type) <br>_Write DNS instance FQDN to buffer._  |
|  void | [**write\_label**](#function-write_label) (uint8\_t \* buf, size\_t \* pos, const char \* str) <br>_Write DNS label with length prefix to buffer._  |
|  void | [**write\_service\_type\_name**](#function-write_service_type_name) (uint8\_t \* buf, size\_t \* pos, const char \* service\_type) <br>_Write DNS service type domain name to buffer._  |
|  void | [**write\_u16**](#function-write_u16) (uint8\_t \* buf, size\_t \* pos, uint16\_t val) <br>_Write 16-bit big-endian integer to buffer._  |
|  void | [**write\_u32**](#function-write_u32) (uint8\_t \* buf, size\_t \* pos, uint32\_t val) <br>_Write 32-bit big-endian integer to buffer._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**DNSSD\_MCAST\_ADDR**](syn__dnssd_8c.md#define-dnssd_mcast_addr)  `"224.0.0.251"`<br>_Multicast DNS IPv4 address._  |
| define  | [**DNSSD\_TTL\_PTR**](syn__dnssd_8c.md#define-dnssd_ttl_ptr)  `4500U`<br>_Default TTL for PTR records (75 minutes)_  |
| define  | [**DNSSD\_TTL\_SRV**](syn__dnssd_8c.md#define-dnssd_ttl_srv)  `120U`<br>_Default TTL for SRV/A records (2 minutes)_  |

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
## Public Static Functions Documentation




### function match\_qname\_service 

_Match DNS question name against registered service type._ 
```C++
static bool match_qname_service (
    const uint8_t * buf,
    size_t buf_len,
    size_t * offset,
    const char * service_type
) 
```





**Parameters:**


* `buf` Input buffer containing DNS question. 
* `buf_len` Total buffer length. 
* `offset` Offset within buffer. 
* `service_type` Expected service type. 



**Returns:**

True if question matches service type, false otherwise. 





        

<hr>



### function pack\_service\_response 

_Pack DNS-SD response packet for a service (PTR, SRV, TXT, A records)._ 
```C++
static SYN_Status pack_service_response (
    const SYN_DnsSd_Service * svc,
    uint8_t * resp_buf,
    size_t max_resp_len,
    size_t * resp_len
) 
```





**Parameters:**


* `svc` Service definition. 
* `resp_buf` Buffer to receive response packet. 
* `max_resp_len` Capacity of resp\_buf. 
* `resp_len` Bytes written. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on buffer overflow. 





        

<hr>



### function skip\_or\_read\_name 

_Helper to skip or extract a DNS domain name (handling pointers)._ 
```C++
static void skip_or_read_name (
    const uint8_t * buf,
    size_t buf_len,
    size_t * pos,
    char * out,
    size_t max_out
) 
```





**Parameters:**


* `buf` Input buffer containing DNS packet. 
* `buf_len` Total buffer length. 
* `pos` Offset within buffer. 
* `out` Output string buffer (or NULL to skip). 
* `max_out` Maximum capacity of out buffer. 




        

<hr>



### function write\_hostname\_fqdn 

_Write DNS hostname FQDN to buffer._ 
```C++
static void write_hostname_fqdn (
    uint8_t * buf,
    size_t * pos,
    const char * hostname
) 
```





**Parameters:**


* `buf` Output buffer. 
* `pos` Current buffer offset. 
* `hostname` Hostname string. 




        

<hr>



### function write\_instance\_fqdn 

_Write DNS instance FQDN to buffer._ 
```C++
static void write_instance_fqdn (
    uint8_t * buf,
    size_t * pos,
    const char * instance,
    const char * service_type
) 
```





**Parameters:**


* `buf` Output buffer. 
* `pos` Current buffer offset. 
* `instance` Instance name. 
* `service_type` Service type. 




        

<hr>



### function write\_label 

_Write DNS label with length prefix to buffer._ 
```C++
static void write_label (
    uint8_t * buf,
    size_t * pos,
    const char * str
) 
```





**Parameters:**


* `buf` Output buffer. 
* `pos` Current buffer offset. 
* `str` Label string. 




        

<hr>



### function write\_service\_type\_name 

_Write DNS service type domain name to buffer._ 
```C++
static void write_service_type_name (
    uint8_t * buf,
    size_t * pos,
    const char * service_type
) 
```





**Parameters:**


* `buf` Output buffer. 
* `pos` Current buffer offset. 
* `service_type` Service type string. 




        

<hr>



### function write\_u16 

_Write 16-bit big-endian integer to buffer._ 
```C++
static void write_u16 (
    uint8_t * buf,
    size_t * pos,
    uint16_t val
) 
```





**Parameters:**


* `buf` Output buffer. 
* `pos` Current buffer offset. 
* `val` Value to write. 




        

<hr>



### function write\_u32 

_Write 32-bit big-endian integer to buffer._ 
```C++
static void write_u32 (
    uint8_t * buf,
    size_t * pos,
    uint32_t val
) 
```





**Parameters:**


* `buf` Output buffer. 
* `pos` Current buffer offset. 
* `val` Value to write. 




        

<hr>
## Macro Definition Documentation





### define DNSSD\_MCAST\_ADDR 

_Multicast DNS IPv4 address._ 
```C++
#define DNSSD_MCAST_ADDR `"224.0.0.251"`
```




<hr>



### define DNSSD\_TTL\_PTR 

_Default TTL for PTR records (75 minutes)_ 
```C++
#define DNSSD_TTL_PTR `4500U`
```




<hr>



### define DNSSD\_TTL\_SRV 

_Default TTL for SRV/A records (2 minutes)_ 
```C++
#define DNSSD_TTL_SRV `120U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dnssd.c`

