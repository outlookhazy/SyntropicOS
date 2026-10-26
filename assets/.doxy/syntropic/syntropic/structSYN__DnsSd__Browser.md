

# Struct SYN\_DnsSd\_Browser



[**ClassList**](annotated.md) **>** [**SYN\_DnsSd\_Browser**](structSYN__DnsSd__Browser.md)



_DNS-SD Service Discovery Browser Context._ 

* `#include <syn_dnssd.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  [**SYN\_DnsSd\_DiscoverCallback**](syn__dnssd_8h.md#typedef-syn_dnssd_discovercallback) | [**callback**](#variable-callback)  <br> |
|  uint32\_t | [**discovered\_count**](#variable-discovered_count)  <br> |
|  const [**SYN\_DnsSd**](structSYN__DnsSd.md) \* | [**sd**](#variable-sd)  <br> |
|  const char \* | [**service\_type**](#variable-service_type)  <br> |
|  uint32\_t | [**start\_time**](#variable-start_time)  <br> |
|  uint32\_t | [**timeout\_ms**](#variable-timeout_ms)  <br> |
|  void \* | [**user\_data**](#variable-user_data)  <br> |












































## Public Attributes Documentation




### variable active 

```C++
bool SYN_DnsSd_Browser::active;
```



True while browsing session is active 
 


        

<hr>



### variable callback 

```C++
SYN_DnsSd_DiscoverCallback SYN_DnsSd_Browser::callback;
```



Discovery result callback 
 


        

<hr>



### variable discovered\_count 

```C++
uint32_t SYN_DnsSd_Browser::discovered_count;
```



Number of services discovered 
 


        

<hr>



### variable sd 

```C++
const SYN_DnsSd* SYN_DnsSd_Browser::sd;
```



Shared DNS-SD context with open socket 


        

<hr>



### variable service\_type 

```C++
const char* SYN_DnsSd_Browser::service_type;
```



Target service type (e.g. "\_http.\_tcp") 


        

<hr>



### variable start\_time 

```C++
uint32_t SYN_DnsSd_Browser::start_time;
```



Timestamp when discovery started 
 


        

<hr>



### variable timeout\_ms 

```C++
uint32_t SYN_DnsSd_Browser::timeout_ms;
```



Maximum active browsing duration in ms 
 


        

<hr>



### variable user\_data 

```C++
void* SYN_DnsSd_Browser::user_data;
```



User callback context 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dnssd.h`

