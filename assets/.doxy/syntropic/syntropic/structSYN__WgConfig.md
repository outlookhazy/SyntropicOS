

# Struct SYN\_WgConfig



[**ClassList**](annotated.md) **>** [**SYN\_WgConfig**](structSYN__WgConfig.md)



_WireGuard peer configuration — set once at init._ 

* `#include <syn_wg.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_SockAddr**](structSYN__SockAddr.md) | [**endpoint**](#variable-endpoint)  <br> |
|  uint16\_t | [**keepalive\_interval\_s**](#variable-keepalive_interval_s)  <br> |
|  uint8\_t | [**peer\_public\_key**](#variable-peer_public_key)  <br> |
|  uint8\_t | [**preshared\_key**](#variable-preshared_key)  <br> |
|  uint8\_t | [**private\_key**](#variable-private_key)  <br> |












































## Public Attributes Documentation




### variable endpoint 

```C++
SYN_SockAddr SYN_WgConfig::endpoint;
```



Server IP:port 
 


        

<hr>



### variable keepalive\_interval\_s 

```C++
uint16_t SYN_WgConfig::keepalive_interval_s;
```



Persistent keepalive (0=off) 
 


        

<hr>



### variable peer\_public\_key 

```C++
uint8_t SYN_WgConfig::peer_public_key[32];
```



Server's public key 
 


        

<hr>



### variable preshared\_key 

```C++
uint8_t SYN_WgConfig::preshared_key[32];
```



Optional PSK (zero if unused) 
 


        

<hr>



### variable private\_key 

```C++
uint8_t SYN_WgConfig::private_key[32];
```



Our Curve25519 private key 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_wg.h`

