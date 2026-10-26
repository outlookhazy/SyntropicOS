

# Struct SYN\_Heartbeat



[**ClassList**](annotated.md) **>** [**SYN\_Heartbeat**](structSYN__Heartbeat.md)



_Heartbeat monitor — send keepalives, track peer liveness._ 

* `#include <syn_heartbeat.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  void \* | [**cb\_ctx**](#variable-cb_ctx)  <br> |
|  [**SYN\_ErrLog**](structSYN__ErrLog.md) \* | [**errlog**](#variable-errlog)  <br> |
|  uint32\_t | [**interval\_ms**](#variable-interval_ms)  <br> |
|  uint32\_t | [**last\_tx\_tick**](#variable-last_tx_tick)  <br> |
|  [**SYN\_HB\_Callback**](syn__heartbeat_8h.md#typedef-syn_hb_callback) | [**on\_peer\_found**](#variable-on_peer_found)  <br> |
|  [**SYN\_HB\_Callback**](syn__heartbeat_8h.md#typedef-syn_hb_callback) | [**on\_peer\_lost**](#variable-on_peer_lost)  <br> |
|  uint8\_t | [**peer\_capacity**](#variable-peer_capacity)  <br> |
|  uint8\_t | [**peer\_count**](#variable-peer_count)  <br> |
|  [**SYN\_HB\_Peer**](structSYN__HB__Peer.md) \* | [**peers**](#variable-peers)  <br> |
|  [**SYN\_Router**](structSYN__Router.md) \* | [**router**](#variable-router)  <br> |
|  uint32\_t | [**timeout\_ms**](#variable-timeout_ms)  <br> |












































## Public Attributes Documentation




### variable cb\_ctx 

```C++
void* SYN_Heartbeat::cb_ctx;
```



Context for callbacks 
 


        

<hr>



### variable errlog 

```C++
SYN_ErrLog* SYN_Heartbeat::errlog;
```



Optional error logging 
 


        

<hr>



### variable interval\_ms 

```C++
uint32_t SYN_Heartbeat::interval_ms;
```



How often to send our heartbeat 


        

<hr>



### variable last\_tx\_tick 

```C++
uint32_t SYN_Heartbeat::last_tx_tick;
```



When we last sent 
 


        

<hr>



### variable on\_peer\_found 

```C++
SYN_HB_Callback SYN_Heartbeat::on_peer_found;
```



Called when peer reappears 
 


        

<hr>



### variable on\_peer\_lost 

```C++
SYN_HB_Callback SYN_Heartbeat::on_peer_lost;
```



Called when peer goes dead 
 


        

<hr>



### variable peer\_capacity 

```C++
uint8_t SYN_Heartbeat::peer_capacity;
```



Peer array capacity 
 


        

<hr>



### variable peer\_count 

```C++
uint8_t SYN_Heartbeat::peer_count;
```



Number of tracked peers 
 


        

<hr>



### variable peers 

```C++
SYN_HB_Peer* SYN_Heartbeat::peers;
```



Caller-owned peer array 
 


        

<hr>



### variable router 

```C++
SYN_Router* SYN_Heartbeat::router;
```



Packet router 
 


        

<hr>



### variable timeout\_ms 

```C++
uint32_t SYN_Heartbeat::timeout_ms;
```



Peer considered dead after this 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_heartbeat.h`

