

# Struct SYN\_DnsResolver



[**ClassList**](annotated.md) **>** [**SYN\_DnsResolver**](structSYN__DnsResolver.md)



_DNS resolver context._ 

* `#include <syn_dns.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_SockAddr**](structSYN__SockAddr.md) \* | [**addr\_out**](#variable-addr_out)  <br> |
|  uint8\_t | [**buf**](#variable-buf)  <br> |
|  const [**SYN\_SockAddr**](structSYN__SockAddr.md) \* | [**dns\_server**](#variable-dns_server)  <br> |
|  const char \* | [**hostname**](#variable-hostname)  <br> |
|  size\_t | [**query\_len**](#variable-query_len)  <br> |
|  [**SYN\_SockAddr**](structSYN__SockAddr.md) | [**server\_addr**](#variable-server_addr)  <br> |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**sock**](#variable-sock)  <br> |
|  uint32\_t | [**start\_ms**](#variable-start_ms)  <br> |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**status**](#variable-status)  <br> |
|  uint32\_t | [**timeout\_ms**](#variable-timeout_ms)  <br> |
|  uint16\_t | [**txid**](#variable-txid)  <br> |












































## Public Attributes Documentation




### variable addr\_out 

```C++
SYN_SockAddr* SYN_DnsResolver::addr_out;
```



Resolved output address 
 


        

<hr>



### variable buf 

```C++
uint8_t SYN_DnsResolver::buf[256];
```



Message buffer 
 


        

<hr>



### variable dns\_server 

```C++
const SYN_SockAddr* SYN_DnsResolver::dns_server;
```



DNS server (e.g. 8.8.8.8) 


        

<hr>



### variable hostname 

```C++
const char* SYN_DnsResolver::hostname;
```



Hostname to resolve 
 


        

<hr>



### variable query\_len 

```C++
size_t SYN_DnsResolver::query_len;
```



Sent query length 
 


        

<hr>



### variable server\_addr 

```C++
SYN_SockAddr SYN_DnsResolver::server_addr;
```



Server address 
 


        

<hr>



### variable sock 

```C++
SYN_Socket SYN_DnsResolver::sock;
```



UDP socket 
 


        

<hr>



### variable start\_ms 

```C++
uint32_t SYN_DnsResolver::start_ms;
```



Start timestamp 
 


        

<hr>



### variable status 

```C++
SYN_Status SYN_DnsResolver::status;
```



Final status of query 
 


        

<hr>



### variable timeout\_ms 

```C++
uint32_t SYN_DnsResolver::timeout_ms;
```



Resolution timeout 
 


        

<hr>



### variable txid 

```C++
uint16_t SYN_DnsResolver::txid;
```



Generated transaction ID 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dns.h`

