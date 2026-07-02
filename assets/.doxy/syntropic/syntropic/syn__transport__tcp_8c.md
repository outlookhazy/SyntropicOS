

# File syn\_transport\_tcp.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_transport\_tcp.c**](syn__transport__tcp_8c.md)

[Go to the source code of this file](syn__transport__tcp_8c_source.md)

_Bridge for syn\_transport over a TCP socket._ 

* `#include "syn_transport_tcp.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_transport\_tcp\_init**](#function-syn_transport_tcp_init) ([**SYN\_Transport**](structSYN__Transport.md) \* t, [**SYN\_TransportTcp**](structSYN__TransportTcp.md) \* tcp, [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock) <br>_Initialize a TCP transport bridge._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**tcp\_recv**](#function-tcp_recv) (uint8\_t \* data, size\_t max\_len, size\_t \* out\_len, void \* ctx) <br>_TCP transport receive callback — length-prefixed framing._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**tcp\_send**](#function-tcp_send) (const uint8\_t \* data, size\_t len, void \* ctx) <br>_TCP transport send callback — length-prefixed framing._  |


























## Public Functions Documentation




### function syn\_transport\_tcp\_init 

_Initialize a TCP transport bridge._ 
```C++
void syn_transport_tcp_init (
    SYN_Transport * t,
    SYN_TransportTcp * tcp,
    SYN_Socket sock
) 
```



Configures the transport interface vtable with the socket-specific operations.




**Parameters:**


* `t` Pointer to the parent transport structure to configure. 
* `tcp` Pointer to the TCP transport context structure to associate. 
* `sock` Connected TCP socket to bind to this transport. 




        

<hr>
## Public Static Functions Documentation




### function tcp\_recv 

_TCP transport receive callback — length-prefixed framing._ 
```C++
static bool tcp_recv (
    uint8_t * data,
    size_t max_len,
    size_t * out_len,
    void * ctx
) 
```





**Parameters:**


* `data` [out] Buffer for received data. 
* `max_len` Buffer capacity. 
* `out_len` [out] Received length. 
* `ctx` Transport context ([**SYN\_TransportTcp**](structSYN__TransportTcp.md) \*). 



**Returns:**

true if a complete packet was received. 





        

<hr>



### function tcp\_send 

_TCP transport send callback — length-prefixed framing._ 
```C++
static bool tcp_send (
    const uint8_t * data,
    size_t len,
    void * ctx
) 
```





**Parameters:**


* `data` Packet data. 
* `len` Packet length. 
* `ctx` Transport context ([**SYN\_TransportTcp**](structSYN__TransportTcp.md) \*). 



**Returns:**

true if sent. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_transport_tcp.c`

