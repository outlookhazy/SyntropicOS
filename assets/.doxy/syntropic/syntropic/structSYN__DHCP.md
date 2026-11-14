

# Struct SYN\_DHCP



[**ClassList**](annotated.md) **>** [**SYN\_DHCP**](structSYN__DHCP.md)



[More...](#detailed-description)

* `#include <syn_dhcp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**acks\_received**](#variable-acks_received)  <br> |
|  uint32\_t | [**discovers\_sent**](#variable-discovers_sent)  <br> |
|  uint32\_t | [**gateway**](#variable-gateway)  <br> |
|  uint32\_t | [**lease\_time\_sec**](#variable-lease_time_sec)  <br> |
|  uint32\_t | [**offered\_ip**](#variable-offered_ip)  <br> |
|  uint32\_t | [**requests\_sent**](#variable-requests_sent)  <br> |
|  uint32\_t | [**server\_ip**](#variable-server_ip)  <br> |
|  [**SYN\_DHCP\_State**](syn__dhcp_8h.md#enum-syn_dhcp_state) | [**state**](#variable-state)  <br> |
|  uint32\_t | [**subnet\_mask**](#variable-subnet_mask)  <br> |
|  uint32\_t | [**xid**](#variable-xid)  <br> |












































## Detailed Description


DHCP Client Context Descriptor. 


    
## Public Attributes Documentation




### variable acks\_received 

```C++
uint32_t SYN_DHCP::acks_received;
```



Telemetry counter 


        

<hr>



### variable discovers\_sent 

```C++
uint32_t SYN_DHCP::discovers_sent;
```



Telemetry counter 


        

<hr>



### variable gateway 

```C++
uint32_t SYN_DHCP::gateway;
```



Router gateway IP 


        

<hr>



### variable lease\_time\_sec 

```C++
uint32_t SYN_DHCP::lease_time_sec;
```



Lease duration 


        

<hr>



### variable offered\_ip 

```C++
uint32_t SYN_DHCP::offered_ip;
```



IP offered by server 


        

<hr>



### variable requests\_sent 

```C++
uint32_t SYN_DHCP::requests_sent;
```



Telemetry counter 


        

<hr>



### variable server\_ip 

```C++
uint32_t SYN_DHCP::server_ip;
```



DHCP Server IP 


        

<hr>



### variable state 

```C++
SYN_DHCP_State SYN_DHCP::state;
```



Active state machine state 


        

<hr>



### variable subnet\_mask 

```C++
uint32_t SYN_DHCP::subnet_mask;
```



Subnet mask 


        

<hr>



### variable xid 

```C++
uint32_t SYN_DHCP::xid;
```



Transaction ID 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dhcp.h`

