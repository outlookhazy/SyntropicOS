

# Struct SYN\_WG



[**ClassList**](annotated.md) **>** [**SYN\_WG**](structSYN__WG.md)



_WireGuard client context — caller-owned._ [More...](#detailed-description)

* `#include <syn_wg.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_WgConfig**](structSYN__WgConfig.md) | [**config**](#variable-config)  <br> |
|  uint8\_t | [**hs\_chaining\_key**](#variable-hs_chaining_key)  <br> |
|  uint8\_t | [**hs\_ephemeral\_priv**](#variable-hs_ephemeral_priv)  <br> |
|  uint8\_t | [**hs\_hash**](#variable-hs_hash)  <br> |
|  uint32\_t | [**last\_handshake\_ms**](#variable-last_handshake_ms)  <br> |
|  uint32\_t | [**last\_recv\_ms**](#variable-last_recv_ms)  <br> |
|  uint32\_t | [**last\_sent\_ms**](#variable-last_sent_ms)  <br> |
|  void(\* | [**on\_recv**](#variable-on_recv)  <br>_Callback for decrypted IP packets received from the tunnel._  |
|  uint8\_t | [**public\_key**](#variable-public_key)  <br> |
|  uint8\_t \* | [**rx\_buf**](#variable-rx_buf)  <br> |
|  size\_t | [**rx\_buf\_size**](#variable-rx_buf_size)  <br> |
|  [**SYN\_WgSession**](structSYN__WgSession.md) | [**session**](#variable-session)  <br> |
|  [**SYN\_SNTP**](structSYN__SNTP.md) \* | [**sntp**](#variable-sntp)  <br> |
|  [**SYN\_WgState**](syn__wg_8h.md#enum-syn_wgstate) | [**state**](#variable-state)  <br> |
|  uint8\_t \* | [**tx\_buf**](#variable-tx_buf)  <br> |
|  size\_t | [**tx\_buf\_size**](#variable-tx_buf_size)  <br> |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**udp\_sock**](#variable-udp_sock)  <br> |
|  void \* | [**user\_ctx**](#variable-user_ctx)  <br> |












































## Detailed Description


Contains config, session state, handshake scratch, timers, and buffer pointers. Approximately ~350 bytes + buffer pointers. 


    
## Public Attributes Documentation




### variable config 

```C++
SYN_WgConfig SYN_WG::config;
```



Peer configuration 
 


        

<hr>



### variable hs\_chaining\_key 

```C++
uint8_t SYN_WG::hs_chaining_key[32];
```



Noise chaining key (CK) 
 


        

<hr>



### variable hs\_ephemeral\_priv 

```C++
uint8_t SYN_WG::hs_ephemeral_priv[32];
```



Ephemeral private key 
 


        

<hr>



### variable hs\_hash 

```C++
uint8_t SYN_WG::hs_hash[32];
```



Noise handshake hash (H) 
 


        

<hr>



### variable last\_handshake\_ms 

```C++
uint32_t SYN_WG::last_handshake_ms;
```



Tick of last handshake initiation 
 


        

<hr>



### variable last\_recv\_ms 

```C++
uint32_t SYN_WG::last_recv_ms;
```



Tick of last data received 
 


        

<hr>



### variable last\_sent\_ms 

```C++
uint32_t SYN_WG::last_sent_ms;
```



Tick of last data sent 
 


        

<hr>



### variable on\_recv 

_Callback for decrypted IP packets received from the tunnel._ 
```C++
void(* SYN_WG::on_recv) (const uint8_t *ip_packet, size_t len, void *ctx);
```





**Parameters:**


* `ip_packet` Raw IP packet data. 
* `len` Packet length. 
* `ctx` User context pointer. 




        

<hr>



### variable public\_key 

```C++
uint8_t SYN_WG::public_key[32];
```



Our public key (from private) 
 


        

<hr>



### variable rx\_buf 

```C++
uint8_t* SYN_WG::rx_buf;
```



Receive buffer 
 


        

<hr>



### variable rx\_buf\_size 

```C++
size_t SYN_WG::rx_buf_size;
```



Receive buffer capacity 
 


        

<hr>



### variable session 

```C++
SYN_WgSession SYN_WG::session;
```



Currently active transport session 


        

<hr>



### variable sntp 

```C++
SYN_SNTP* SYN_WG::sntp;
```



NTP time source (for TAI64N) 
 


        

<hr>



### variable state 

```C++
SYN_WgState SYN_WG::state;
```



Connection state 
 


        

<hr>



### variable tx\_buf 

```C++
uint8_t* SYN_WG::tx_buf;
```



Transmit buffer 
 


        

<hr>



### variable tx\_buf\_size 

```C++
size_t SYN_WG::tx_buf_size;
```



Transmit buffer capacity 
 


        

<hr>



### variable udp\_sock 

```C++
SYN_Socket SYN_WG::udp_sock;
```



UDP socket to the server 
 


        

<hr>



### variable user\_ctx 

```C++
void* SYN_WG::user_ctx;
```



User context for on\_recv 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_wg.h`

