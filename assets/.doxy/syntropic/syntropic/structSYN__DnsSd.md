

# Struct SYN\_DnsSd



[**ClassList**](annotated.md) **>** [**SYN\_DnsSd**](structSYN__DnsSd.md)



_DNS-SD Daemon Context._ 

* `#include <syn_dnssd.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  size\_t | [**service\_count**](#variable-service_count)  <br> |
|  [**SYN\_DnsSd\_Service**](structSYN__DnsSd__Service.md) | [**services**](#variable-services)  <br> |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**sock**](#variable-sock)  <br> |












































## Public Attributes Documentation




### variable service\_count 

```C++
size_t SYN_DnsSd::service_count;
```



Registered count 
 


        

<hr>



### variable services 

```C++
SYN_DnsSd_Service SYN_DnsSd::services[SYN_DNSSD_MAX_SERVICES];
```



Service registry 
 


        

<hr>



### variable sock 

```C++
SYN_Socket SYN_DnsSd::sock;
```



Multicast UDP socket 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dnssd.h`

