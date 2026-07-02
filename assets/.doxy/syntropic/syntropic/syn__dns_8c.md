

# File syn\_dns.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_dns.c**](syn__dns_8c.md)

[Go to the source code of this file](syn__dns_8c_source.md)

_UDP DNS resolver and mDNS responder implementation._ 

* `#include "syn_dns.h"`
* `#include "../util/syn_assert.h"`
* `#include "../port/syn_port_system.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_dns\_resolve\_task**](#function-syn_dns_resolve_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative task for resolving a hostname via DNS._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mdns\_init**](#function-syn_mdns_init) ([**SYN\_Mdns**](structSYN__Mdns.md) \* mdns, const char \* hostname, const uint8\_t ip) <br>_Initialize the mDNS responder._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_mdns\_task**](#function-syn_mdns_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative task for responding to local mDNS queries._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**encode\_qname**](#function-encode_qname) (uint8\_t \* dest, const char \* src) <br>_Encode a hostname as a DNS QNAME (label-length format)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**match\_qname\_local**](#function-match_qname_local) (const uint8\_t \* buf, size\_t buf\_len, size\_t \* pos, const char \* hostname) <br>_Check if QNAME matches hostname.local._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**parse\_qname**](#function-parse_qname) (const uint8\_t \* buf, size\_t buf\_len, size\_t \* pos) <br>_Skip a QNAME in a DNS packet (handles compression pointers)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**parse\_response**](#function-parse_response) (const uint8\_t \* buf, size\_t rx\_len, [**SYN\_SockAddr**](structSYN__SockAddr.md) \* addr\_out, uint16\_t expected\_txid) <br>_Parse a DNS response packet to extract the resolved IP._  |


























## Public Functions Documentation




### function syn\_dns\_resolve\_task 

_Cooperative task for resolving a hostname via DNS._ 
```C++
SYN_PT_Status syn_dns_resolve_task (
    SYN_PT * pt,
    SYN_Task * task
) 
```



Register this as a SYN\_TaskFunc with user\_data pointing to a [**SYN\_DnsResolver**](structSYN__DnsResolver.md) instance. The task runs non-blocking UDP queries, yielding via PT\_YIELD until resolution completes or times out.




**Parameters:**


* `pt` Protothread pointer. 
* `task` Task structure whose user\_data points to a [**SYN\_DnsResolver**](structSYN__DnsResolver.md). 



**Returns:**

PT\_WAITING while running, PT\_EXITED when done, or PT\_ENDED on early exit. 





        

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

PT\_WAITING while running, PT\_EXITED when done. 





        

<hr>
## Public Static Functions Documentation




### function encode\_qname 

_Encode a hostname as a DNS QNAME (label-length format)._ 
```C++
static size_t encode_qname (
    uint8_t * dest,
    const char * src
) 
```





**Parameters:**


* `dest` [out] Destination buffer. 
* `src` Dot-separated hostname. 



**Returns:**

Total bytes written. 





        

<hr>



### function match\_qname\_local 

_Check if QNAME matches hostname.local._ 
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

_Parse a DNS response packet to extract the resolved IP._ 
```C++
static SYN_Status parse_response (
    const uint8_t * buf,
    size_t rx_len,
    SYN_SockAddr * addr_out,
    uint16_t expected_txid
) 
```



Checks transaction ID, question count, and pulls the first IPv4 A record.




**Parameters:**


* `buf` Received UDP packet buffer. 
* `rx_len` Received packet length. 
* `addr_out` [out] Output structure for resolved IPv4 address. 
* `expected_txid` Transaction ID to verify. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on format/parsing error. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dns.c`

