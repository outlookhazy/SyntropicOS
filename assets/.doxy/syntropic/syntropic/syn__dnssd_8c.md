

# File syn\_dnssd.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_dnssd.c**](syn__dnssd_8c.md)

[Go to the source code of this file](syn__dnssd_8c_source.md)

_DNS-Based Service Discovery (DNS-SD - RFC 6763 / RFC 6762) implementation._ 

* `#include "syn_dnssd.h"`
* `#include "../port/syn_port_system.h"`
* `#include "../sched/syn_sched.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_pack.h"`
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
|  void | [**dnssd\_append\_label**](#function-dnssd_append_label) (char \* out, size\_t max\_out, size\_t \* out\_len, const uint8\_t \* label, size\_t len) <br>_Append one DNS label (with leading dot if non-empty) into a bounded C string._  |
|  uint8\_t | [**dnssd\_ascii\_lower**](#function-dnssd_ascii_lower) (uint8\_t c) <br>_Fold an ASCII uppercase letter to lowercase._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnssd\_browse\_wait\_done**](#function-dnssd_browse_wait_done) (const [**SYN\_DnsSd\_Browser**](structSYN__DnsSd__Browser.md) \* browser) <br>_Check whether a DNS-SD browser task should wake from PT\_WAIT\_UNTIL._  |
|  void | [**dnssd\_handle\_browse\_pkt**](#function-dnssd_handle_browse_pkt) ([**SYN\_DnsSd\_Browser**](structSYN__DnsSd__Browser.md) \* browser, const uint8\_t \* buf, size\_t len) <br>_Parse one received mDNS browser packet and invoke callback on service match._  |
|  void | [**dnssd\_handle\_query\_pkt**](#function-dnssd_handle_query_pkt) (const [**SYN\_DnsSd**](structSYN__DnsSd.md) \* sd, uint8\_t \* pkt, size\_t cap, size\_t qlen) <br>_Process one received mDNS query datagram in-place and multicast any matching response._  |
|  const [**SYN\_DnsSd\_Service**](structSYN__DnsSd__Service.md) \* | [**dnssd\_match\_registered**](#function-dnssd_match_registered) (const [**SYN\_DnsSd**](structSYN__DnsSd.md) \* sd, const uint8\_t \* query\_buf, size\_t query\_len, size\_t name\_start, size\_t \* next\_pos) <br>_Match a DNS question at name\_start against any registered service._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnssd\_pack\_a\_rr**](#function-dnssd_pack_a_rr) (const [**SYN\_DnsSd\_Service**](structSYN__DnsSd__Service.md) \* svc, uint8\_t \* buf, size\_t cap, size\_t \* pos) <br>_Pack IPv4 A additional record into bounded DNS-SD response buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnssd\_pack\_ptr\_rr**](#function-dnssd_pack_ptr_rr) (const [**SYN\_DnsSd\_Service**](structSYN__DnsSd__Service.md) \* svc, uint8\_t \* buf, size\_t cap, size\_t \* pos) <br>_Pack PTR answer record into bounded DNS-SD response buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnssd\_pack\_srv\_rr**](#function-dnssd_pack_srv_rr) (const [**SYN\_DnsSd\_Service**](structSYN__DnsSd__Service.md) \* svc, uint8\_t \* buf, size\_t cap, size\_t \* pos) <br>_Pack SRV additional record into bounded DNS-SD response buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnssd\_pack\_txt\_rr**](#function-dnssd_pack_txt_rr) (const [**SYN\_DnsSd\_Service**](structSYN__DnsSd__Service.md) \* svc, uint8\_t \* buf, size\_t cap, size\_t \* pos) <br>_Pack TXT additional record into bounded DNS-SD response buffer._  |
|  void | [**dnssd\_parse\_ptr\_rdata**](#function-dnssd_parse_ptr_rdata) (const uint8\_t \* resp\_buf, size\_t rdata\_start, uint16\_t rdlength, [**SYN\_DnsSd\_Discovered**](structSYN__DnsSd__Discovered.md) \* out\_disc) <br>_Parse PTR RDATA into instance\_name and service\_type fields._  |
|  void | [**dnssd\_parse\_rr**](#function-dnssd_parse_rr) (const uint8\_t \* resp\_buf, size\_t rr\_hdr\_pos, uint16\_t rdlength, [**SYN\_DnsSd\_Discovered**](structSYN__DnsSd__Discovered.md) \* out\_disc) <br>_Dispatch a single DNS resource record into a discovered service descriptor._  |
|  void | [**dnssd\_parse\_srv\_rdata**](#function-dnssd_parse_srv_rdata) (const uint8\_t \* resp\_buf, size\_t rdata\_start, uint16\_t rdlength, [**SYN\_DnsSd\_Discovered**](structSYN__DnsSd__Discovered.md) \* out\_disc) <br>_Parse SRV RDATA into port and hostname fields._  |
|  void | [**dnssd\_patch\_rdlen**](#function-dnssd_patch_rdlen) (uint8\_t \* buf, size\_t rdlen\_pos, size\_t end\_pos) <br>_Backpatch a 16-bit big-endian RDLENGTH field after writing RDATA._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnssd\_put\_bytes**](#function-dnssd_put_bytes) (uint8\_t \* buf, size\_t cap, size\_t \* pos, const void \* src, size\_t len) <br>_Append raw bytes into a bounded output buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnssd\_put\_hostname\_fqdn**](#function-dnssd_put_hostname_fqdn) (uint8\_t \* buf, size\_t cap, size\_t \* pos, const char \* hostname) <br>_Write DNS hostname FQDN (_ `host.local.` _) into bounded buffer._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnssd\_put\_instance\_fqdn**](#function-dnssd_put_instance_fqdn) (uint8\_t \* buf, size\_t cap, size\_t \* pos, const char \* instance, const char \* service\_type) <br>_Write DNS instance FQDN into bounded buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnssd\_put\_label**](#function-dnssd_put_label) (uint8\_t \* buf, size\_t cap, size\_t \* pos, const char \* str) <br>_Write DNS label (clamped to 63 octets) with length prefix into bounded buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnssd\_put\_rr\_meta**](#function-dnssd_put_rr_meta) (uint8\_t \* buf, size\_t cap, size\_t \* pos, uint16\_t type, uint16\_t cls, uint32\_t ttl, size\_t \* rdlen\_pos) <br>_Write fixed 10-byte DNS RR metadata and record RDLENGTH offset._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnssd\_put\_service\_type\_name**](#function-dnssd_put_service_type_name) (uint8\_t \* buf, size\_t cap, size\_t \* pos, const char \* service\_type) <br>_Write DNS service type domain name (_ `_svc._proto.local.` _) into bounded buffer._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnssd\_put\_u16**](#function-dnssd_put_u16) (uint8\_t \* buf, size\_t cap, size\_t \* pos, uint16\_t val) <br>_Append a 16-bit big-endian integer into a bounded output buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnssd\_put\_u32**](#function-dnssd_put_u32) (uint8\_t \* buf, size\_t cap, size\_t \* pos, uint32\_t val) <br>_Append a 32-bit big-endian integer into a bounded output buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnssd\_put\_u8**](#function-dnssd_put_u8) (uint8\_t \* buf, size\_t cap, size\_t \* pos, uint8\_t val) <br>_Append a single byte into a bounded output buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnssd\_qtype\_qclass\_valid**](#function-dnssd_qtype_qclass_valid) (uint16\_t qtype, uint16\_t qclass) <br>_Validate QTYPE and QCLASS for a DNS-SD service query._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnssd\_read\_first\_label**](#function-dnssd_read_first_label) (const uint8\_t \* buf, size\_t stream\_end, size\_t start\_pos, char out\_label, size\_t \* rest\_pos) <br>_Read the first DNS label (following up to 8 backward compression pointers) as raw bytes._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnssd\_skip\_question**](#function-dnssd_skip_question) (const uint8\_t \* buf, size\_t buf\_len, size\_t \* pos) <br>_Skip a DNS question entry (QNAME with optional 0xC0 pointer + QTYPE + QCLASS)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnssd\_streq\_ci**](#function-dnssd_streq_ci) (const char \* a, const char \* b) <br>_Compare two NUL-terminated strings for ASCII case-insensitive equality._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnssd\_valid\_service**](#function-dnssd_valid_service) (const [**SYN\_DnsSd\_Service**](structSYN__DnsSd__Service.md) \* svc) <br>_Validate a DNS-SD service descriptor including TXT array bounds and non-NULL entries._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnssd\_valid\_service\_type**](#function-dnssd_valid_service_type) (const char \* service\_type) <br>_Validate a DNS-SD service type string (_ `<svc>` _or_`<svc>.<proto>` _)._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**match\_qname\_service**](#function-match_qname_service) (const uint8\_t \* buf, size\_t buf\_len, size\_t \* offset, const char \* service\_type) <br>_Match DNS question name against registered service type (case-insensitive)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**pack\_service\_response**](#function-pack_service_response) (const [**SYN\_DnsSd\_Service**](structSYN__DnsSd__Service.md) \* svc, uint8\_t \* resp\_buf, size\_t max\_resp\_len, size\_t \* resp\_len) <br>_Pack DNS-SD response packet for a service (PTR, SRV, TXT, A records)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**skip\_or\_read\_name**](#function-skip_or_read_name) (const uint8\_t \* buf, size\_t stream\_end, size\_t \* pos, char \* out, size\_t max\_out) <br>_Extract a DNS domain name (following up to 8 backward 0xC0 pointers) into a C string._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**DNSSD\_MCAST\_ADDR**](syn__dnssd_8c.md#define-dnssd_mcast_addr)  `"224.0.0.251"`<br>_Multicast DNS IPv4 address._  |
| define  | [**DNSSD\_TTL\_PTR**](syn__dnssd_8c.md#define-dnssd_ttl_ptr)  `4500U`<br>_Default TTL for PTR records (75 minutes)._  |
| define  | [**DNSSD\_TTL\_SRV**](syn__dnssd_8c.md#define-dnssd_ttl_srv)  `120U`<br>_Default TTL for SRV/A records (2 minutes)._  |

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

PT\_IDLE while running, PT\_ENDED when discovery timeout elapses. 





        

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

PT\_IDLE while running, PT\_ENDED when done. 





        

<hr>
## Public Static Functions Documentation




### function dnssd\_append\_label 

_Append one DNS label (with leading dot if non-empty) into a bounded C string._ 
```C++
static void dnssd_append_label (
    char * out,
    size_t max_out,
    size_t * out_len,
    const uint8_t * label,
    size_t len
) 
```





**Parameters:**


* `out` Output string buffer. 
* `max_out` Output buffer capacity. 
* `out_len` [in,out] Current string length. 
* `label` Label bytes. 
* `len` Label byte length. 




        

<hr>



### function dnssd\_ascii\_lower 

_Fold an ASCII uppercase letter to lowercase._ 
```C++
static uint8_t dnssd_ascii_lower (
    uint8_t c
) 
```





**Parameters:**


* `c` Input byte. 



**Returns:**

Lowercase ASCII byte. 





        

<hr>



### function dnssd\_browse\_wait\_done 

_Check whether a DNS-SD browser task should wake from PT\_WAIT\_UNTIL._ 
```C++
static bool dnssd_browse_wait_done (
    const SYN_DnsSd_Browser * browser
) 
```





**Parameters:**


* `browser` Browser context. 



**Returns:**

true if inactive, socket closed, timed out, or UDP socket readable. 





        

<hr>



### function dnssd\_handle\_browse\_pkt 

_Parse one received mDNS browser packet and invoke callback on service match._ 
```C++
static void dnssd_handle_browse_pkt (
    SYN_DnsSd_Browser * browser,
    const uint8_t * buf,
    size_t len
) 
```





**Parameters:**


* `browser` Browser context. 
* `buf` Received datagram bytes. 
* `len` Received datagram length. 




        

<hr>



### function dnssd\_handle\_query\_pkt 

_Process one received mDNS query datagram in-place and multicast any matching response._ 
```C++
static void dnssd_handle_query_pkt (
    const SYN_DnsSd * sd,
    uint8_t * pkt,
    size_t cap,
    size_t qlen
) 
```





**Parameters:**


* `sd` DNS-SD daemon context. 
* `pkt` Shared packet buffer containing query on entry and response on match. 
* `cap` Total capacity of pkt buffer. 
* `qlen` Received query byte length. 




        

<hr>



### function dnssd\_match\_registered 

_Match a DNS question at name\_start against any registered service._ 
```C++
static const SYN_DnsSd_Service * dnssd_match_registered (
    const SYN_DnsSd * sd,
    const uint8_t * query_buf,
    size_t query_len,
    size_t name_start,
    size_t * next_pos
) 
```





**Parameters:**


* `sd` DNS-SD instance. 
* `query_buf` Query packet buffer. 
* `query_len` Query packet length. 
* `name_start` Offset of QNAME. 
* `next_pos` [out] Offset after QNAME + QTYPE + QCLASS when matched. 



**Returns:**

Pointer to matching service, or NULL. 





        

<hr>



### function dnssd\_pack\_a\_rr 

_Pack IPv4 A additional record into bounded DNS-SD response buffer._ 
```C++
static bool dnssd_pack_a_rr (
    const SYN_DnsSd_Service * svc,
    uint8_t * buf,
    size_t cap,
    size_t * pos
) 
```





**Parameters:**


* `svc` Service definition. 
* `buf` Destination buffer. 
* `cap` Buffer capacity. 
* `pos` [in,out] Write cursor. 



**Returns:**

true on success, false on truncation. 





        

<hr>



### function dnssd\_pack\_ptr\_rr 

_Pack PTR answer record into bounded DNS-SD response buffer._ 
```C++
static bool dnssd_pack_ptr_rr (
    const SYN_DnsSd_Service * svc,
    uint8_t * buf,
    size_t cap,
    size_t * pos
) 
```





**Parameters:**


* `svc` Service definition. 
* `buf` Destination buffer. 
* `cap` Buffer capacity. 
* `pos` [in,out] Write cursor. 



**Returns:**

true on success, false on truncation. 





        

<hr>



### function dnssd\_pack\_srv\_rr 

_Pack SRV additional record into bounded DNS-SD response buffer._ 
```C++
static bool dnssd_pack_srv_rr (
    const SYN_DnsSd_Service * svc,
    uint8_t * buf,
    size_t cap,
    size_t * pos
) 
```





**Parameters:**


* `svc` Service definition. 
* `buf` Destination buffer. 
* `cap` Buffer capacity. 
* `pos` [in,out] Write cursor. 



**Returns:**

true on success, false on truncation. 





        

<hr>



### function dnssd\_pack\_txt\_rr 

_Pack TXT additional record into bounded DNS-SD response buffer._ 
```C++
static bool dnssd_pack_txt_rr (
    const SYN_DnsSd_Service * svc,
    uint8_t * buf,
    size_t cap,
    size_t * pos
) 
```





**Parameters:**


* `svc` Service definition. 
* `buf` Destination buffer. 
* `cap` Buffer capacity. 
* `pos` [in,out] Write cursor. 



**Returns:**

true on success, false on truncation. 





        

<hr>



### function dnssd\_parse\_ptr\_rdata 

_Parse PTR RDATA into instance\_name and service\_type fields._ 
```C++
static void dnssd_parse_ptr_rdata (
    const uint8_t * resp_buf,
    size_t rdata_start,
    uint16_t rdlength,
    SYN_DnsSd_Discovered * out_disc
) 
```





**Parameters:**


* `resp_buf` DNS response packet buffer. 
* `rdata_start` Offset of PTR RDATA. 
* `rdlength` RDATA byte length. 
* `out_disc` [out] Discovered service output struct. 




        

<hr>



### function dnssd\_parse\_rr 

_Dispatch a single DNS resource record into a discovered service descriptor._ 
```C++
static void dnssd_parse_rr (
    const uint8_t * resp_buf,
    size_t rr_hdr_pos,
    uint16_t rdlength,
    SYN_DnsSd_Discovered * out_disc
) 
```





**Parameters:**


* `resp_buf` DNS response packet buffer. 
* `rr_hdr_pos` Offset of 10-byte RR header (`TYPE`, `CLASS`, `TTL`, `RDLENGTH`). 
* `rdlength` RDATA byte length. 
* `out_disc` [out] Discovered service output struct. 




        

<hr>



### function dnssd\_parse\_srv\_rdata 

_Parse SRV RDATA into port and hostname fields._ 
```C++
static void dnssd_parse_srv_rdata (
    const uint8_t * resp_buf,
    size_t rdata_start,
    uint16_t rdlength,
    SYN_DnsSd_Discovered * out_disc
) 
```





**Parameters:**


* `resp_buf` DNS response packet buffer. 
* `rdata_start` Offset of SRV RDATA. 
* `rdlength` RDATA byte length. 
* `out_disc` [out] Discovered service output struct. 




        

<hr>



### function dnssd\_patch\_rdlen 

_Backpatch a 16-bit big-endian RDLENGTH field after writing RDATA._ 
```C++
static void dnssd_patch_rdlen (
    uint8_t * buf,
    size_t rdlen_pos,
    size_t end_pos
) 
```





**Parameters:**


* `buf` Packet buffer. 
* `rdlen_pos` Offset of RDLENGTH field. 
* `end_pos` Offset immediately following RDATA. 




        

<hr>



### function dnssd\_put\_bytes 

_Append raw bytes into a bounded output buffer._ 
```C++
static bool dnssd_put_bytes (
    uint8_t * buf,
    size_t cap,
    size_t * pos,
    const void * src,
    size_t len
) 
```





**Parameters:**


* `buf` Destination buffer. 
* `cap` Buffer capacity. 
* `pos` [in,out] Write cursor. 
* `src` Source bytes. 
* `len` Byte count to append. 



**Returns:**

true if bytes fit and were written, false on truncation. 





        

<hr>



### function dnssd\_put\_hostname\_fqdn 

_Write DNS hostname FQDN (_ `host.local.` _) into bounded buffer._
```C++
static bool dnssd_put_hostname_fqdn (
    uint8_t * buf,
    size_t cap,
    size_t * pos,
    const char * hostname
) 
```





**Parameters:**


* `buf` Destination buffer. 
* `cap` Buffer capacity. 
* `pos` [in,out] Write cursor. 
* `hostname` Hostname string. 



**Returns:**

true on success, false on truncation. 





        

<hr>



### function dnssd\_put\_instance\_fqdn 

_Write DNS instance FQDN into bounded buffer._ 
```C++
static bool dnssd_put_instance_fqdn (
    uint8_t * buf,
    size_t cap,
    size_t * pos,
    const char * instance,
    const char * service_type
) 
```





**Parameters:**


* `buf` Destination buffer. 
* `cap` Buffer capacity. 
* `pos` [in,out] Write cursor. 
* `instance` Instance name. 
* `service_type` Service type. 



**Returns:**

true on success, false on truncation. 





        

<hr>



### function dnssd\_put\_label 

_Write DNS label (clamped to 63 octets) with length prefix into bounded buffer._ 
```C++
static bool dnssd_put_label (
    uint8_t * buf,
    size_t cap,
    size_t * pos,
    const char * str
) 
```





**Parameters:**


* `buf` Destination buffer. 
* `cap` Buffer capacity. 
* `pos` [in,out] Write cursor. 
* `str` Label string. 



**Returns:**

true on success, false on truncation. 





        

<hr>



### function dnssd\_put\_rr\_meta 

_Write fixed 10-byte DNS RR metadata and record RDLENGTH offset._ 
```C++
static bool dnssd_put_rr_meta (
    uint8_t * buf,
    size_t cap,
    size_t * pos,
    uint16_t type,
    uint16_t cls,
    uint32_t ttl,
    size_t * rdlen_pos
) 
```





**Parameters:**


* `buf` Destination buffer. 
* `cap` Buffer capacity. 
* `pos` [in,out] Write cursor. 
* `type` RR type code. 
* `cls` RR class + cache-flush bits. 
* `ttl` TTL in seconds. 
* `rdlen_pos` [out] Offset of 16-bit RDLENGTH field. 



**Returns:**

true on success, false on truncation. 





        

<hr>



### function dnssd\_put\_service\_type\_name 

_Write DNS service type domain name (_ `_svc._proto.local.` _) into bounded buffer._
```C++
static bool dnssd_put_service_type_name (
    uint8_t * buf,
    size_t cap,
    size_t * pos,
    const char * service_type
) 
```





**Parameters:**


* `buf` Destination buffer. 
* `cap` Buffer capacity. 
* `pos` [in,out] Write cursor. 
* `service_type` Service type string. 



**Returns:**

true on success, false on truncation. 





        

<hr>



### function dnssd\_put\_u16 

_Append a 16-bit big-endian integer into a bounded output buffer._ 
```C++
static bool dnssd_put_u16 (
    uint8_t * buf,
    size_t cap,
    size_t * pos,
    uint16_t val
) 
```





**Parameters:**


* `buf` Destination buffer. 
* `cap` Buffer capacity. 
* `pos` [in,out] Write cursor. 
* `val` 16-bit value. 



**Returns:**

true on success, false on truncation. 





        

<hr>



### function dnssd\_put\_u32 

_Append a 32-bit big-endian integer into a bounded output buffer._ 
```C++
static bool dnssd_put_u32 (
    uint8_t * buf,
    size_t cap,
    size_t * pos,
    uint32_t val
) 
```





**Parameters:**


* `buf` Destination buffer. 
* `cap` Buffer capacity. 
* `pos` [in,out] Write cursor. 
* `val` 32-bit value. 



**Returns:**

true on success, false on truncation. 





        

<hr>



### function dnssd\_put\_u8 

_Append a single byte into a bounded output buffer._ 
```C++
static bool dnssd_put_u8 (
    uint8_t * buf,
    size_t cap,
    size_t * pos,
    uint8_t val
) 
```





**Parameters:**


* `buf` Destination buffer. 
* `cap` Buffer capacity. 
* `pos` [in,out] Write cursor. 
* `val` Byte value. 



**Returns:**

true on success, false on truncation. 





        

<hr>



### function dnssd\_qtype\_qclass\_valid 

_Validate QTYPE and QCLASS for a DNS-SD service query._ 
```C++
static bool dnssd_qtype_qclass_valid (
    uint16_t qtype,
    uint16_t qclass
) 
```





**Parameters:**


* `qtype` 16-bit DNS QTYPE. 
* `qclass` 16-bit DNS QCLASS (may include top unicast-response QU bit). 



**Returns:**

true if QTYPE is PTR/SRV/TXT/ANY and class is IN/ANY. 





        

<hr>



### function dnssd\_read\_first\_label 

_Read the first DNS label (following up to 8 backward compression pointers) as raw bytes._ 
```C++
static bool dnssd_read_first_label (
    const uint8_t * buf,
    size_t stream_end,
    size_t start_pos,
    char out_label,
    size_t * rest_pos
) 
```





**Parameters:**


* `buf` Packet buffer. 
* `stream_end` End offset of inline RDATA (`<= msg_len`). 
* `start_pos` Starting offset of the domain name. 
* `out_label` [out] Copy of first label (NUL-terminated, max 63 chars). 
* `rest_pos` [out] Offset immediately after first label. 



**Returns:**

true if a non-empty first label was extracted within bounds, false otherwise. 





        

<hr>



### function dnssd\_skip\_question 

_Skip a DNS question entry (QNAME with optional 0xC0 pointer + QTYPE + QCLASS)._ 
```C++
static bool dnssd_skip_question (
    const uint8_t * buf,
    size_t buf_len,
    size_t * pos
) 
```





**Parameters:**


* `buf` DNS query buffer. 
* `buf_len` Total query byte length. 
* `pos` [in,out] Read cursor. 



**Returns:**

true if question entry was well-formed within bounds, false otherwise. 





        

<hr>



### function dnssd\_streq\_ci 

_Compare two NUL-terminated strings for ASCII case-insensitive equality._ 
```C++
static bool dnssd_streq_ci (
    const char * a,
    const char * b
) 
```





**Parameters:**


* `a` First string. 
* `b` Second string. 



**Returns:**

true if strings match ignoring ASCII case. 





        

<hr>



### function dnssd\_valid\_service 

_Validate a DNS-SD service descriptor including TXT array bounds and non-NULL entries._ 
```C++
static bool dnssd_valid_service (
    const SYN_DnsSd_Service * svc
) 
```





**Parameters:**


* `svc` Service configuration. 



**Returns:**

true if valid. 





        

<hr>



### function dnssd\_valid\_service\_type 

_Validate a DNS-SD service type string (_ `<svc>` _or_`<svc>.<proto>` _)._
```C++
static bool dnssd_valid_service_type (
    const char * service_type
) 
```





**Parameters:**


* `service_type` Service type string. 



**Returns:**

true if labels are non-empty and contain at most one dot. 





        

<hr>



### function match\_qname\_service 

_Match DNS question name against registered service type (case-insensitive)._ 
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

_Extract a DNS domain name (following up to 8 backward 0xC0 pointers) into a C string._ 
```C++
static bool skip_or_read_name (
    const uint8_t * buf,
    size_t stream_end,
    size_t * pos,
    char * out,
    size_t max_out
) 
```





**Parameters:**


* `buf` Packet start buffer. 
* `stream_end` End offset of readable section (`<= msg_len`). 
* `pos` [in,out] Inline cursor; updated past inline name on success. 
* `out` [out] Output string buffer (capacity &gt;= 2). 
* `max_out` Capacity of out buffer. 



**Returns:**

true if name was well-formed within bounds, false otherwise. 





        

<hr>
## Macro Definition Documentation





### define DNSSD\_MCAST\_ADDR 

_Multicast DNS IPv4 address._ 
```C++
#define DNSSD_MCAST_ADDR `"224.0.0.251"`
```




<hr>



### define DNSSD\_TTL\_PTR 

_Default TTL for PTR records (75 minutes)._ 
```C++
#define DNSSD_TTL_PTR `4500U`
```




<hr>



### define DNSSD\_TTL\_SRV 

_Default TTL for SRV/A records (2 minutes)._ 
```C++
#define DNSSD_TTL_SRV `120U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dnssd.c`

