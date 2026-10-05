

# Struct SYN\_DnsSd\_Service



[**ClassList**](annotated.md) **>** [**SYN\_DnsSd\_Service**](structSYN__DnsSd__Service.md)



_DNS-SD Service Definition._ 

* `#include <syn_dnssd.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const char \* | [**hostname**](#variable-hostname)  <br> |
|  const char \* | [**instance\_name**](#variable-instance_name)  <br> |
|  uint8\_t | [**ip**](#variable-ip)  <br> |
|  uint16\_t | [**port**](#variable-port)  <br> |
|  const char \* | [**service\_type**](#variable-service_type)  <br> |
|  size\_t | [**txt\_count**](#variable-txt_count)  <br> |
|  const char \* | [**txt\_records**](#variable-txt_records)  <br> |












































## Public Attributes Documentation




### variable hostname 

```C++
const char* SYN_DnsSd_Service::hostname;
```



Target Hostname without .local (e.g. "node") 
 


        

<hr>



### variable instance\_name 

```C++
const char* SYN_DnsSd_Service::instance_name;
```



Service Instance Name (e.g. "Syntropic Node") 


        

<hr>



### variable ip 

```C++
uint8_t SYN_DnsSd_Service::ip[4];
```



IPv4 address to announce 
 


        

<hr>



### variable port 

```C++
uint16_t SYN_DnsSd_Service::port;
```



TCP/UDP Port number (e.g. 80) 
 


        

<hr>



### variable service\_type 

```C++
const char* SYN_DnsSd_Service::service_type;
```



Service Type (e.g. "\_http.\_tcp") 
 


        

<hr>



### variable txt\_count 

```C++
size_t SYN_DnsSd_Service::txt_count;
```



Number of TXT strings 
 


        

<hr>



### variable txt\_records 

```C++
const char* SYN_DnsSd_Service::txt_records[SYN_DNSSD_MAX_TXT_RECORDS];
```



Key-value TXT strings 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dnssd.h`

