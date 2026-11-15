

# File syn\_dns.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_dns.c**](syn__dns_8c.md)

[Go to the source code of this file](syn__dns_8c_source.md)

_UDP DNS resolver and mDNS responder implementation._ 

* `#include "../port/syn_port_system.h"`
* `#include "../sched/syn_sched.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_pack.h"`
* `#include "syn_dns.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_dns\_init**](#function-syn_dns_init) ([**SYN\_DnsResolver**](structSYN__DnsResolver.md) \* resolver) <br>_Initialize a DNS resolver context to safe defaults._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_dns\_resolve\_task**](#function-syn_dns_resolve_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative task for resolving a hostname via DNS._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mdns\_init**](#function-syn_mdns_init) ([**SYN\_Mdns**](structSYN__Mdns.md) \* mdns, const char \* hostname, const uint8\_t ip) <br>_Initialize the mDNS responder._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_mdns\_task**](#function-syn_mdns_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative task for responding to local mDNS queries._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**dns\_ascii\_lower**](#function-dns_ascii_lower) (uint8\_t c) <br>_Fold an ASCII uppercase letter to lowercase._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dns\_build\_and\_send\_query**](#function-dns_build_and_send_query) ([**SYN\_DnsResolver**](structSYN__DnsResolver.md) \* r) <br>_Encode and transmit the UDP DNS A-record query packet._  |
|  void | [**dns\_close\_socket**](#function-dns_close_socket) ([**SYN\_DnsResolver**](structSYN__DnsResolver.md) \* r) <br>_Close resolver UDP socket if currently owned by this resolver._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dns\_memeq\_ci**](#function-dns_memeq_ci) (const void \* a, const void \* b, size\_t len) <br>_Compare two byte slices for ASCII case-insensitive equality._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dns\_packet\_matches\_txid**](#function-dns_packet_matches_txid) (const [**SYN\_DnsResolver**](structSYN__DnsResolver.md) \* r, int n) <br>_Check whether received UDP datagram matches expected DNS header and TXID._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dns\_skip\_questions**](#function-dns_skip_questions) (const uint8\_t \* buf, size\_t rx\_len, uint16\_t questions, size\_t \* pos) <br>_Skip DNS question entries in a packet._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dns\_wait\_ready**](#function-dns_wait_ready) (const [**SYN\_DnsResolver**](structSYN__DnsResolver.md) \* r) <br>_Check whether DNS resolver socket is readable or timeout has elapsed._  |
|  size\_t | [**encode\_qname**](#function-encode_qname) (uint8\_t \* dest, size\_t dest\_capacity, const char \* src) <br>_Encode a hostname as a DNS QNAME (label-length format)._  |
|  size\_t | [**get\_base\_hostname\_len**](#function-get_base_hostname_len) (const char \* hostname) <br>_Get length of base hostname (excluding trailing ".local" if present)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**match\_dns\_label**](#function-match_dns_label) (const uint8\_t \* buf, size\_t buf\_len, size\_t \* pos, size\_t \* end\_pos, uint8\_t \* hops, const char \* label, size\_t label\_len) <br>_Match a single DNS label against expected ASCII bytes (case-insensitive)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**match\_qname\_local**](#function-match_qname_local) (const uint8\_t \* buf, size\_t buf\_len, size\_t \* pos, const char \* hostname) <br>_Check if QNAME matches hostname.local (case-insensitive, compressed)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**mdns\_deref\_pointers**](#function-mdns_deref_pointers) (const uint8\_t \* buf, size\_t buf\_len, size\_t \* p, size\_t \* end\_pos, uint8\_t \* hops) <br>_Dereference up to 8 strictly backward 0xC0 DNS compression pointers._  |
|  void | [**mdns\_handle\_packet**](#function-mdns_handle_packet) (const [**SYN\_Mdns**](structSYN__Mdns.md) \* mdns, const uint8\_t \* buf, size\_t rx\_len) <br>_Inspect an incoming mDNS UDP packet and reply if a question matches._  |
|  void | [**mdns\_send\_response**](#function-mdns_send_response) (const [**SYN\_Mdns**](structSYN__Mdns.md) \* mdns) <br>_Build and multicast an mDNS IPv4 A-record response for mdns-&gt;hostname._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**parse\_qname**](#function-parse_qname) (const uint8\_t \* buf, size\_t buf\_len, size\_t \* pos) <br>_Skip a QNAME in a DNS packet (handles compression pointers)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**parse\_response**](#function-parse_response) (const uint8\_t \* buf, size\_t rx\_len, [**SYN\_SockAddr**](structSYN__SockAddr.md) \* addr\_out) <br>_Parse a DNS response packet (validated &gt;= 12 bytes with matching TXID) to extract the first IPv4 A record._  |


























## Public Functions Documentation




### function syn\_dns\_init 

_Initialize a DNS resolver context to safe defaults._ 
```C++
void syn_dns_init (
    SYN_DnsResolver * resolver
) 
```



Clears the structure and marks the internal UDP socket descriptor invalid.




**Parameters:**


* `resolver` DNS resolver context to initialize. 




        

<hr>



### function syn\_dns\_resolve\_task 

_Cooperative task for resolving a hostname via DNS._ 
```C++
SYN_PT_Status syn_dns_resolve_task (
    SYN_PT * pt,
    SYN_Task * task
) 
```



Register this as a [**SYN\_TaskFunc**](syn__task_8h.md#typedef-syn_taskfunc) with user\_data pointing to a [**SYN\_DnsResolver**](structSYN__DnsResolver.md) instance. The task runs non-blocking UDP queries, yielding until resolution completes or times out.




**Parameters:**


* `pt` Protothread pointer. 
* `task` Task structure whose user\_data points to a [**SYN\_DnsResolver**](structSYN__DnsResolver.md). 



**Returns:**

PT\_IDLE while waiting, PT\_ENDED when done, or PT\_EXITED on early abort. 





        

<hr>



### function syn\_mdns\_init 

_Initialize the mDNS responder._ 
```C++
SYN_Status syn_mdns_init (
    SYN_Mdns * mdns,
    const char * hostname,
    const uint8_t ip
) 
```





**Parameters:**


* `mdns` mDNS instance. 
* `hostname` Hostname to respond to (without .local suffix). 
* `ip` IPv4 address to advertise. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_mdns\_task 

_Cooperative task for responding to local mDNS queries._ 
```C++
SYN_PT_Status syn_mdns_task (
    SYN_PT * pt,
    SYN_Task * task
) 
```





**Parameters:**


* `pt` Protothread pointer. 
* `task` Task structure whose user\_data points to a [**SYN\_Mdns**](structSYN__Mdns.md). 



**Returns:**

PT\_IDLE while running, PT\_ENDED/PT\_EXITED when done. 





        

<hr>
## Public Static Functions Documentation




### function dns\_ascii\_lower 

_Fold an ASCII uppercase letter to lowercase._ 
```C++
static uint8_t dns_ascii_lower (
    uint8_t c
) 
```





**Parameters:**


* `c` Input byte. 



**Returns:**

Lowercase ASCII byte. 





        

<hr>



### function dns\_build\_and\_send\_query 

_Encode and transmit the UDP DNS A-record query packet._ 
```C++
static bool dns_build_and_send_query (
    SYN_DnsResolver * r
) 
```





**Parameters:**


* `r` DNS resolver instance with open socket. 



**Returns:**

true if query was encoded and sent, false on error. 





        

<hr>



### function dns\_close\_socket 

_Close resolver UDP socket if currently owned by this resolver._ 
```C++
static void dns_close_socket (
    SYN_DnsResolver * r
) 
```





**Parameters:**


* `r` DNS resolver instance. 




        

<hr>



### function dns\_memeq\_ci 

_Compare two byte slices for ASCII case-insensitive equality._ 
```C++
static bool dns_memeq_ci (
    const void * a,
    const void * b,
    size_t len
) 
```





**Parameters:**


* `a` First byte buffer. 
* `b` Second byte buffer. 
* `len` Byte length to compare. 



**Returns:**

true if slices match ignoring ASCII case. 





        

<hr>



### function dns\_packet\_matches\_txid 

_Check whether received UDP datagram matches expected DNS header and TXID._ 
```C++
static bool dns_packet_matches_txid (
    const SYN_DnsResolver * r,
    int n
) 
```





**Parameters:**


* `r` DNS resolver instance. 
* `n` Received byte count (&gt; 0). 



**Returns:**

true if length &gt;= 12 and TXID matches. 





        

<hr>



### function dns\_skip\_questions 

_Skip DNS question entries in a packet._ 
```C++
static bool dns_skip_questions (
    const uint8_t * buf,
    size_t rx_len,
    uint16_t questions,
    size_t * pos
) 
```





**Parameters:**


* `buf` Packet buffer. 
* `rx_len` Packet length. 
* `questions` Number of question entries to skip. 
* `pos` [in/out] Parse cursor. 



**Returns:**

true if all question entries fit within bounds. 





        

<hr>



### function dns\_wait\_ready 

_Check whether DNS resolver socket is readable or timeout has elapsed._ 
```C++
static bool dns_wait_ready (
    const SYN_DnsResolver * r
) 
```





**Parameters:**


* `r` DNS resolver instance. 



**Returns:**

true if ready to wake from wait. 





        

<hr>



### function encode\_qname 

_Encode a hostname as a DNS QNAME (label-length format)._ 
```C++
static size_t encode_qname (
    uint8_t * dest,
    size_t dest_capacity,
    const char * src
) 
```





**Parameters:**


* `dest` [out] Destination buffer (capacity &gt;= 2 bytes). 
* `dest_capacity` Destination buffer capacity. 
* `src` Dot-separated hostname. 



**Returns:**

Total bytes written, or 0 on overflow. 





        

<hr>



### function get\_base\_hostname\_len 

_Get length of base hostname (excluding trailing ".local" if present)._ 
```C++
static size_t get_base_hostname_len (
    const char * hostname
) 
```





**Parameters:**


* `hostname` Non-NULL input hostname string. 



**Returns:**

Length of base label in bytes. 





        

<hr>



### function match\_dns\_label 

_Match a single DNS label against expected ASCII bytes (case-insensitive)._ 
```C++
static bool match_dns_label (
    const uint8_t * buf,
    size_t buf_len,
    size_t * pos,
    size_t * end_pos,
    uint8_t * hops,
    const char * label,
    size_t label_len
) 
```





**Parameters:**


* `buf` Packet buffer. 
* `buf_len` Buffer length. 
* `pos` [in/out] Parse offset. 
* `end_pos` [in/out] Inline end offset saved on first pointer jump. 
* `hops` [in/out] Cumulative pointer hop counter. 
* `label` Expected label string. 
* `label_len` Expected label byte length. 



**Returns:**

true if label matches and fits within buffer. 





        

<hr>



### function match\_qname\_local 

_Check if QNAME matches hostname.local (case-insensitive, compressed)._ 
```C++
static bool match_qname_local (
    const uint8_t * buf,
    size_t buf_len,
    size_t * pos,
    const char * hostname
) 
```





**Parameters:**


* `buf` Packet buffer. 
* `buf_len` Buffer length. 
* `pos` [in/out] Parse position. 
* `hostname` Expected hostname. 



**Returns:**

true if match. 





        

<hr>



### function mdns\_deref\_pointers 

_Dereference up to 8 strictly backward 0xC0 DNS compression pointers._ 
```C++
static bool mdns_deref_pointers (
    const uint8_t * buf,
    size_t buf_len,
    size_t * p,
    size_t * end_pos,
    uint8_t * hops
) 
```





**Parameters:**


* `buf` Packet buffer. 
* `buf_len` Buffer length. 
* `p` [in/out] Active read cursor. 
* `end_pos` [in/out] Inline end offset saved on first jump. 
* `hops` [in/out] Cumulative pointer hop counter. 



**Returns:**

true if cursor resolves to an in-bounds non-pointer byte. 





        

<hr>



### function mdns\_handle\_packet 

_Inspect an incoming mDNS UDP packet and reply if a question matches._ 
```C++
static void mdns_handle_packet (
    const SYN_Mdns * mdns,
    const uint8_t * buf,
    size_t rx_len
) 
```





**Parameters:**


* `mdns` mDNS responder instance. 
* `buf` Received packet buffer. 
* `rx_len` Received packet length (&gt; 12). 




        

<hr>



### function mdns\_send\_response 

_Build and multicast an mDNS IPv4 A-record response for mdns-&gt;hostname._ 
```C++
static void mdns_send_response (
    const SYN_Mdns * mdns
) 
```





**Parameters:**


* `mdns` mDNS responder instance. 




        

<hr>



### function parse\_qname 

_Skip a QNAME in a DNS packet (handles compression pointers)._ 
```C++
static bool parse_qname (
    const uint8_t * buf,
    size_t buf_len,
    size_t * pos
) 
```





**Parameters:**


* `buf` Packet buffer. 
* `buf_len` Buffer length. 
* `pos` [in/out] Current parse position. 



**Returns:**

true on success. 





        

<hr>



### function parse\_response 

_Parse a DNS response packet (validated &gt;= 12 bytes with matching TXID) to extract the first IPv4 A record._ 
```C++
static SYN_Status parse_response (
    const uint8_t * buf,
    size_t rx_len,
    SYN_SockAddr * addr_out
) 
```





**Parameters:**


* `buf` Received UDP packet buffer. 
* `rx_len` Received packet length (&gt;= 12). 
* `addr_out` [out] Output structure for resolved IPv4 address. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on format/parsing error. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dns.c`

