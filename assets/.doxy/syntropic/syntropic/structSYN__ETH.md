

# Struct SYN\_ETH



[**ClassList**](annotated.md) **>** [**SYN\_ETH**](structSYN__ETH.md)



[More...](#detailed-description)

* `#include <syn_eth.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_ETH\_ArpEntry**](structSYN__ETH__ArpEntry.md) | [**arp\_cache**](#variable-arp_cache)  <br> |
|  uint32\_t | [**arp\_replies**](#variable-arp_replies)  <br> |
|  uint32\_t | [**arp\_requests**](#variable-arp_requests)  <br> |
|  uint32\_t | [**frames\_rx**](#variable-frames_rx)  <br> |
|  uint32\_t | [**frames\_tx**](#variable-frames_tx)  <br> |
|  uint32\_t | [**gateway**](#variable-gateway)  <br> |
|  uint32\_t | [**ip\_addr**](#variable-ip_addr)  <br> |
|  uint8\_t | [**mac\_addr**](#variable-mac_addr)  <br> |
|  uint32\_t | [**netmask**](#variable-netmask)  <br> |












































## Detailed Description


Native Ethernet Interface Engine. 


    
## Public Attributes Documentation




### variable arp\_cache 

```C++
SYN_ETH_ArpEntry SYN_ETH::arp_cache[SYN_ETH_ARP_CACHE_SIZE];
```



ARP cache table array 


        

<hr>



### variable arp\_replies 

```C++
uint32_t SYN_ETH::arp_replies;
```



Total ARP replies processed 


        

<hr>



### variable arp\_requests 

```C++
uint32_t SYN_ETH::arp_requests;
```



Total ARP requests processed 


        

<hr>



### variable frames\_rx 

```C++
uint32_t SYN_ETH::frames_rx;
```



Total frames received 


        

<hr>



### variable frames\_tx 

```C++
uint32_t SYN_ETH::frames_tx;
```



Total frames transmitted 


        

<hr>



### variable gateway 

```C++
uint32_t SYN_ETH::gateway;
```



Default gateway IP address 


        

<hr>



### variable ip\_addr 

```C++
uint32_t SYN_ETH::ip_addr;
```



Interface IPv4 address 


        

<hr>



### variable mac\_addr 

```C++
uint8_t SYN_ETH::mac_addr[SYN_ETH_MAC_LEN];
```



Interface MAC address 


        

<hr>



### variable netmask 

```C++
uint32_t SYN_ETH::netmask;
```



Interface subnet mask 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_eth.h`

