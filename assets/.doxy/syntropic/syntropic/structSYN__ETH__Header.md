

# Struct SYN\_ETH\_Header



[**ClassList**](annotated.md) **>** [**SYN\_ETH\_Header**](structSYN__ETH__Header.md)



[More...](#detailed-description)

* `#include <syn_eth.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**dst\_mac**](#variable-dst_mac)  <br> |
|  uint16\_t | [**ethertype**](#variable-ethertype)  <br> |
|  uint8\_t | [**src\_mac**](#variable-src_mac)  <br> |












































## Detailed Description


Parsed Ethernet II Header. 


    
## Public Attributes Documentation




### variable dst\_mac 

```C++
uint8_t SYN_ETH_Header::dst_mac[SYN_ETH_MAC_LEN];
```



Destination MAC address 


        

<hr>



### variable ethertype 

```C++
uint16_t SYN_ETH_Header::ethertype;
```



16-bit EtherType field 


        

<hr>



### variable src\_mac 

```C++
uint8_t SYN_ETH_Header::src_mac[SYN_ETH_MAC_LEN];
```



Source MAC address 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_eth.h`

