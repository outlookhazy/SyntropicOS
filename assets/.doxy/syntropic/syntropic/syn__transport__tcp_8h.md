

# File syn\_transport\_tcp.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_transport\_tcp.h**](syn__transport__tcp_8h.md)

[Go to the source code of this file](syn__transport__tcp_8h_source.md)

_Bridge for syn\_transport over a TCP socket._ 

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_socket.h"`
* `#include "syn_transport.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_TransportTcp**](structSYN__TransportTcp.md) <br>_TCP bridge transport layer context._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_transport\_tcp\_init**](#function-syn_transport_tcp_init) ([**SYN\_Transport**](structSYN__Transport.md) \* t, [**SYN\_TransportTcp**](structSYN__TransportTcp.md) \* tcp, [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock) <br>_Initialize a TCP transport bridge._  |




























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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_transport_tcp.h`

