

# Struct SYN\_NETCFG



[**ClassList**](annotated.md) **>** [**SYN\_NETCFG**](structSYN__NETCFG.md)



[More...](#detailed-description)

* `#include <syn_netcfg.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**assigned\_gateway**](#variable-assigned_gateway)  <br> |
|  uint32\_t | [**assigned\_ip**](#variable-assigned_ip)  <br> |
|  uint32\_t | [**assigned\_netmask**](#variable-assigned_netmask)  <br> |
|  [**SYN\_AUTOIP**](structSYN__AUTOIP.md) | [**autoip**](#variable-autoip)  <br> |
|  [**SYN\_DHCP**](structSYN__DHCP.md) | [**dhcp**](#variable-dhcp)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_bound**](#variable-is_bound)  <br> |
|  [**SYN\_NETCFG\_LinkCb**](syn__netcfg_8h.md#typedef-syn_netcfg_linkcb) | [**link\_cb**](#variable-link_cb)  <br> |
|  [**SYN\_NETCFG\_LinkState**](syn__netcfg_8h.md#enum-syn_netcfg_linkstate) | [**link\_state**](#variable-link_state)  <br> |
|  [**SYN\_NETCFG\_Mode**](syn__netcfg_8h.md#enum-syn_netcfg_mode) | [**mode**](#variable-mode)  <br> |
|  uint32\_t | [**static\_gateway**](#variable-static_gateway)  <br> |
|  uint32\_t | [**static\_ip**](#variable-static_ip)  <br> |
|  uint32\_t | [**static\_netmask**](#variable-static_netmask)  <br> |
|  void \* | [**user\_data**](#variable-user_data)  <br> |












































## Detailed Description


Network IP Manager Context. 


    
## Public Attributes Documentation




### variable assigned\_gateway 

```C++
uint32_t SYN_NETCFG::assigned_gateway;
```



Currently bound active gateway IP 


        

<hr>



### variable assigned\_ip 

```C++
uint32_t SYN_NETCFG::assigned_ip;
```



Currently bound active IPv4 address 


        

<hr>



### variable assigned\_netmask 

```C++
uint32_t SYN_NETCFG::assigned_netmask;
```



Currently bound active subnet mask 


        

<hr>



### variable autoip 

```C++
SYN_AUTOIP SYN_NETCFG::autoip;
```



Embedded AutoIP (Link-Local) context 


        

<hr>



### variable dhcp 

```C++
SYN_DHCP SYN_NETCFG::dhcp;
```



Embedded DHCP client context 


        

<hr>



### variable is\_bound 

```C++
bool SYN_NETCFG::is_bound;
```



True if IP configuration is active and bound 


        

<hr>



### variable link\_cb 

```C++
SYN_NETCFG_LinkCb SYN_NETCFG::link_cb;
```



Optional application link state change callback 


        

<hr>



### variable link\_state 

```C++
SYN_NETCFG_LinkState SYN_NETCFG::link_state;
```



Physical link state (Down/Up) 


        

<hr>



### variable mode 

```C++
SYN_NETCFG_Mode SYN_NETCFG::mode;
```



IP configuration mode (Static, DHCP, AutoIP, Hybrid) 


        

<hr>



### variable static\_gateway 

```C++
uint32_t SYN_NETCFG::static_gateway;
```



Configured static default gateway IP 


        

<hr>



### variable static\_ip 

```C++
uint32_t SYN_NETCFG::static_ip;
```



Configured static IPv4 address 


        

<hr>



### variable static\_netmask 

```C++
uint32_t SYN_NETCFG::static_netmask;
```



Configured static subnet mask 


        

<hr>



### variable user\_data 

```C++
void* SYN_NETCFG::user_data;
```



User context pointer passed to callback 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_netcfg.h`

