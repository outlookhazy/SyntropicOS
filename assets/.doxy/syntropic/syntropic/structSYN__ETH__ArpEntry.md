

# Struct SYN\_ETH\_ArpEntry



[**ClassList**](annotated.md) **>** [**SYN\_ETH\_ArpEntry**](structSYN__ETH__ArpEntry.md)



[More...](#detailed-description)

* `#include <syn_eth.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**ip**](#variable-ip)  <br> |
|  uint32\_t | [**last\_seen\_ms**](#variable-last_seen_ms)  <br> |
|  uint8\_t | [**mac**](#variable-mac)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**valid**](#variable-valid)  <br> |












































## Detailed Description


ARP Table Cache Entry. 


    
## Public Attributes Documentation




### variable ip 

```C++
uint32_t SYN_ETH_ArpEntry::ip;
```



32-bit IPv4 address 


        

<hr>



### variable last\_seen\_ms 

```C++
uint32_t SYN_ETH_ArpEntry::last_seen_ms;
```



Tick timestamp of last update 


        

<hr>



### variable mac 

```C++
uint8_t SYN_ETH_ArpEntry::mac[SYN_ETH_MAC_LEN];
```



Resolved 6-byte MAC address 


        

<hr>



### variable valid 

```C++
bool SYN_ETH_ArpEntry::valid;
```



True if entry is active/valid 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_eth.h`

