

# Struct SYN\_WgSession



[**ClassList**](annotated.md) **>** [**SYN\_WgSession**](structSYN__WgSession.md)



_Active session derived from a completed handshake._ 

* `#include <syn_wg.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**established\_ms**](#variable-established_ms)  <br> |
|  uint32\_t | [**receiver\_index**](#variable-receiver_index)  <br> |
|  uint32\_t | [**recv\_bitmap**](#variable-recv_bitmap)  <br> |
|  uint64\_t | [**recv\_counter**](#variable-recv_counter)  <br> |
|  uint8\_t | [**recv\_key**](#variable-recv_key)  <br> |
|  uint64\_t | [**send\_counter**](#variable-send_counter)  <br> |
|  uint8\_t | [**send\_key**](#variable-send_key)  <br> |
|  uint32\_t | [**sender\_index**](#variable-sender_index)  <br> |












































## Public Attributes Documentation




### variable established\_ms 

```C++
uint32_t SYN_WgSession::established_ms;
```



Tick when session was established 
 


        

<hr>



### variable receiver\_index 

```C++
uint32_t SYN_WgSession::receiver_index;
```



Peer's sender index 
 


        

<hr>



### variable recv\_bitmap 

```C++
uint32_t SYN_WgSession::recv_bitmap;
```



Anti-replay sliding window (32 bits) 
 


        

<hr>



### variable recv\_counter 

```C++
uint64_t SYN_WgSession::recv_counter;
```



Highest received nonce 
 


        

<hr>



### variable recv\_key 

```C++
uint8_t SYN_WgSession::recv_key[32];
```



ChaCha20-Poly1305 key for incoming 
 


        

<hr>



### variable send\_counter 

```C++
uint64_t SYN_WgSession::send_counter;
```



Outgoing nonce counter 
 


        

<hr>



### variable send\_key 

```C++
uint8_t SYN_WgSession::send_key[32];
```



ChaCha20-Poly1305 key for outgoing 
 


        

<hr>



### variable sender\_index 

```C++
uint32_t SYN_WgSession::sender_index;
```



Our sender index (in transport headers) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_wg.h`

