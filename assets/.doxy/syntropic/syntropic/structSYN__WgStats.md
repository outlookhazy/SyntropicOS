

# Struct SYN\_WgStats



[**ClassList**](annotated.md) **>** [**SYN\_WgStats**](structSYN__WgStats.md)



_WireGuard tunnel statistics structure._ 

* `#include <syn_wg.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**handshake\_age\_sec**](#variable-handshake_age_sec)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_established**](#variable-is_established)  <br> |
|  uint64\_t | [**rx\_bytes**](#variable-rx_bytes)  <br> |
|  uint64\_t | [**tx\_bytes**](#variable-tx_bytes)  <br> |












































## Public Attributes Documentation




### variable handshake\_age\_sec 

```C++
uint32_t SYN_WgStats::handshake_age_sec;
```



Seconds since last successful handshake 


        

<hr>



### variable is\_established 

```C++
bool SYN_WgStats::is_established;
```



Tunnel connection status 


        

<hr>



### variable rx\_bytes 

```C++
uint64_t SYN_WgStats::rx_bytes;
```



Total encrypted bytes received 


        

<hr>



### variable tx\_bytes 

```C++
uint64_t SYN_WgStats::tx_bytes;
```



Total encrypted bytes transmitted 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_wg.h`

