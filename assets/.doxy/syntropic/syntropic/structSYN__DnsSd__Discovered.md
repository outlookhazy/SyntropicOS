

# Struct SYN\_DnsSd\_Discovered



[**ClassList**](annotated.md) **>** [**SYN\_DnsSd\_Discovered**](structSYN__DnsSd__Discovered.md)



_Discovered Service Information received from mDNS response._ 

* `#include <syn_dnssd.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  char | [**hostname**](#variable-hostname)  <br> |
|  char | [**instance\_name**](#variable-instance_name)  <br> |
|  uint8\_t | [**ip**](#variable-ip)  <br> |
|  uint16\_t | [**port**](#variable-port)  <br> |
|  char | [**service\_type**](#variable-service_type)  <br> |
|  char | [**txt**](#variable-txt)  <br> |












































## Public Attributes Documentation




### variable hostname 

```C++
char SYN_DnsSd_Discovered::hostname[64];
```



Discovered Target Hostname (e.g. "node") 
 


        

<hr>



### variable instance\_name 

```C++
char SYN_DnsSd_Discovered::instance_name[64];
```



Discovered Instance Name (e.g. "Syntropic Node") 


        

<hr>



### variable ip 

```C++
uint8_t SYN_DnsSd_Discovered::ip[4];
```



IPv4 address 
 


        

<hr>



### variable port 

```C++
uint16_t SYN_DnsSd_Discovered::port;
```



Port number 
 


        

<hr>



### variable service\_type 

```C++
char SYN_DnsSd_Discovered::service_type[64];
```



Discovered Service Type (e.g. "\_http.\_tcp") 
 


        

<hr>



### variable txt 

```C++
char SYN_DnsSd_Discovered::txt[128];
```



First TXT record or summary 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dnssd.h`

