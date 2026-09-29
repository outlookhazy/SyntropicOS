

# Struct SYN\_Mdns



[**ClassList**](annotated.md) **>** [**SYN\_Mdns**](structSYN__Mdns.md)



_mDNS responder instance._ 

* `#include <syn_dns.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const char \* | [**hostname**](#variable-hostname)  <br> |
|  uint8\_t | [**ip**](#variable-ip)  <br> |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**sock**](#variable-sock)  <br> |












































## Public Attributes Documentation




### variable hostname 

```C++
const char* SYN_Mdns::hostname;
```



Responds to hostname.local 
 


        

<hr>



### variable ip 

```C++
uint8_t SYN_Mdns::ip[4];
```



IPv4 address to respond with 
 


        

<hr>



### variable sock 

```C++
SYN_Socket SYN_Mdns::sock;
```



UDP socket for mDNS 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dns.h`

