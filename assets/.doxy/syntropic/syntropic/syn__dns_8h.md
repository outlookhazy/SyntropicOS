

# File syn\_dns.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_dns.h**](syn__dns_8h.md)

[Go to the source code of this file](syn__dns_8h_source.md)

_UDP DNS resolver and mDNS responder._ 

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_socket.h"`
* `#include "../pt/syn_pt.h"`
* `#include "../sched/syn_task.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_DnsResolver**](structSYN__DnsResolver.md) <br>_DNS resolver context._  |
| struct | [**SYN\_Mdns**](structSYN__Mdns.md) <br>_mDNS responder instance._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_dns\_resolve\_task**](#function-syn_dns_resolve_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative task for resolving a hostname via DNS._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mdns\_init**](#function-syn_mdns_init) ([**SYN\_Mdns**](structSYN__Mdns.md) \* mdns, const char \* hostname, const uint8\_t ip) <br>_Initialize the mDNS responder._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_mdns\_task**](#function-syn_mdns_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative task for responding to local mDNS queries._  |




























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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dns.h`

