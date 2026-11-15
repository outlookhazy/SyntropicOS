

# File syn\_transport\_udp.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_transport\_udp.c**](syn__transport__udp_8c.md)

[Go to the source code of this file](syn__transport__udp_8c_source.md)

_Bridge implementation connecting syn\_port\_udp\_\* to native syn\_udp stack._ 

* `#include "syntropic/net/syn_transport_udp.h"`
* `#include "port/syn_port_eth.h"`
* `#include "syntropic/util/syn_assert.h"`
* `#include "syntropic/util/syn_pack.h"`
* `#include <string.h>`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_TransportEthTxFn**](syn__transport__udp_8h.md#typedef-syn_transportethtxfn) | [**g\_eth\_tx\_fn**](#variable-g_eth_tx_fn)   = `[**syn\_port\_eth\_tx**](syn__transport__udp_8c.md#function-syn_port_eth_tx)`<br> |
|  [**SYN\_UDP**](structSYN__UDP.md) \* | [**g\_native\_udp**](#variable-g_native_udp)   = `NULL`<br> |
|  uint8\_t | [**g\_udp\_tx\_buf**](#variable-g_udp_tx_buf)  <br> |














## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_eth\_tx**](#function-syn_port_eth_tx) (const void \* frame, size\_t len) <br>_Transmit raw Ethernet frame over PHY._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) void | [**syn\_port\_sock\_close**](#function-syn_port_sock_close) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock) <br>_Close a socket and release resources._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**syn\_port\_udp\_open**](#function-syn_port_udp_open) (uint16\_t port) <br>_Open a UDP socket bound to the given port._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_port\_udp\_readable**](#function-syn_port_udp_readable) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock) <br>_Check if a UDP socket has a datagram available._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) int | [**syn\_port\_udp\_recvfrom**](#function-syn_port_udp_recvfrom) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock, void \* buf, size\_t max\_len, [**SYN\_SockAddr**](structSYN__SockAddr.md) \* from, uint32\_t timeout\_ms) <br>_Receive a UDP packet._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) int | [**syn\_port\_udp\_sendto**](#function-syn_port_udp_sendto) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock, const void \* data, size\_t len, const [**SYN\_SockAddr**](structSYN__SockAddr.md) \* to) <br>_Send a UDP packet to a destination address._  |
|  void | [**syn\_transport\_udp\_close**](#function-syn_transport_udp_close) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock\_handle) <br>_Close and unbind a native UDP socket handle._  |
|  [**SYN\_UDP**](structSYN__UDP.md) \* | [**syn\_transport\_udp\_get\_instance**](#function-syn_transport_udp_get_instance) (void) <br>_Get the active native_ [_**SYN\_UDP**_](structSYN__UDP.md) _stack instance._ |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**syn\_transport\_udp\_open**](#function-syn_transport_udp_open) (uint16\_t port) <br>_Open and bind a native UDP socket handle (assigning ephemeral port when_ `port` _is 0)._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_transport\_udp\_readable**](#function-syn_transport_udp_readable) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock\_handle) <br>_Check whether a bound native UDP socket has queued receive bytes._  |
|  int | [**syn\_transport\_udp\_recvfrom**](#function-syn_transport_udp_recvfrom) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock\_handle, void \* buf, size\_t max\_len, [**SYN\_SockAddr**](structSYN__SockAddr.md) \* from, uint32\_t timeout\_ms) <br>_Non-blocking receive from a bound native UDP socket handle._  |
|  int | [**syn\_transport\_udp\_sendto**](#function-syn_transport_udp_sendto) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock\_handle, const void \* data, size\_t len, const [**SYN\_SockAddr**](structSYN__SockAddr.md) \* to) <br>_Transmit a UDP datagram via the bound native UDP instance and Ethernet port._  |
|  void | [**syn\_transport\_udp\_set\_eth\_tx**](#function-syn_transport_udp_set_eth_tx) ([**SYN\_TransportEthTxFn**](syn__transport__udp_8h.md#typedef-syn_transportethtxfn) tx\_fn) <br>_Override the Ethernet transmit function used by syn\_transport\_udp\_sendto._  |
|  void | [**syn\_transport\_udp\_set\_instance**](#function-syn_transport_udp_set_instance) ([**SYN\_UDP**](structSYN__UDP.md) \* udp) <br>_Bind native syn\_udp instance to the syn\_port\_udp\_\* socket interface._  |




























## Public Static Attributes Documentation




### variable g\_eth\_tx\_fn 

```C++
SYN_TransportEthTxFn g_eth_tx_fn;
```



Active Ethernet transmit function (defaults to syn\_port\_eth\_tx) 


        

<hr>



### variable g\_native\_udp 

```C++
SYN_UDP* g_native_udp;
```



Global native UDP stack pointer 


        

<hr>



### variable g\_udp\_tx\_buf 

```C++
uint8_t g_udp_tx_buf[SYN_ETH_MAX_FRAME_LEN];
```



Internal static transmission buffer 


        

<hr>
## Public Functions Documentation




### function syn\_port\_eth\_tx 

_Transmit raw Ethernet frame over PHY._ 
```C++
SYN_WEAK  SYN_Status syn_port_eth_tx (
    const void * frame,
    size_t len
) 
```





**Parameters:**


* `frame` Pointer to frame bytes. 
* `len` Frame length (60..1514 bytes). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_sock\_close 

_Close a socket and release resources._ 
```C++
SYN_WEAK void syn_port_sock_close (
    SYN_Socket sock
) 
```





**Parameters:**


* `sock` Socket handle to close. 




        

<hr>



### function syn\_port\_udp\_open 

_Open a UDP socket bound to the given port._ 
```C++
SYN_WEAK  SYN_Socket syn_port_udp_open (
    uint16_t port
) 
```





**Parameters:**


* `port` Local port to bind to, or 0 for ephemeral. 



**Returns:**

Socket handle on success, SYN\_SOCKET\_INVALID on failure. 





        

<hr>



### function syn\_port\_udp\_readable 

_Check if a UDP socket has a datagram available._ 
```C++
SYN_WEAK  bool syn_port_udp_readable (
    SYN_Socket sock
) 
```





**Parameters:**


* `sock` Socket handle. 



**Returns:**

true if readable without blocking, false otherwise. 





        

<hr>



### function syn\_port\_udp\_recvfrom 

_Receive a UDP packet._ 
```C++
SYN_WEAK int syn_port_udp_recvfrom (
    SYN_Socket sock,
    void * buf,
    size_t max_len,
    SYN_SockAddr * from,
    uint32_t timeout_ms
) 
```





**Parameters:**


* `sock` Socket handle. 
* `buf` Receive buffer. 
* `max_len` Buffer capacity. 
* `from` [out] Source address. 
* `timeout_ms` Receive timeout in milliseconds. **If 0, must return immediately (non-blocking poll).** Cooperative tasks (protothreads) rely on this to avoid blocking the scheduler. Port implementations MUST NOT treat 0 as "block forever." 



**Returns:**

Number of bytes received, 0 if no data available (non-blocking), or -1 on error. 





        

<hr>



### function syn\_port\_udp\_sendto 

_Send a UDP packet to a destination address._ 
```C++
SYN_WEAK int syn_port_udp_sendto (
    SYN_Socket sock,
    const void * data,
    size_t len,
    const SYN_SockAddr * to
) 
```





**Parameters:**


* `sock` Socket handle. 
* `data` Payload to send. 
* `len` Payload length. 
* `to` Destination address (IP + port). 



**Returns:**

Number of bytes sent, or -1 on error. 





        

<hr>



### function syn\_transport\_udp\_close 

_Close and unbind a native UDP socket handle._ 
```C++
void syn_transport_udp_close (
    SYN_Socket sock_handle
) 
```





**Parameters:**


* `sock_handle` Native UDP socket handle. 




        

<hr>



### function syn\_transport\_udp\_get\_instance 

_Get the active native_ [_**SYN\_UDP**_](structSYN__UDP.md) _stack instance._
```C++
SYN_UDP * syn_transport_udp_get_instance (
    void
) 
```





**Returns:**

Pointer to [**SYN\_UDP**](structSYN__UDP.md) instance, or NULL.


Get the active native [**SYN\_UDP**](structSYN__UDP.md) stack instance.




**Returns:**

Pointer to global [**SYN\_UDP**](structSYN__UDP.md) instance, or NULL if unavailable. 





        

<hr>



### function syn\_transport\_udp\_open 

_Open and bind a native UDP socket handle (assigning ephemeral port when_ `port` _is 0)._
```C++
SYN_Socket syn_transport_udp_open (
    uint16_t port
) 
```





**Parameters:**


* `port` Local UDP port, or 0 for dynamic ephemeral port (49152..65535). 



**Returns:**

Socket handle (&gt;= 100), or SYN\_SOCKET\_INVALID on failure. 





        

<hr>



### function syn\_transport\_udp\_readable 

_Check whether a bound native UDP socket has queued receive bytes._ 
```C++
bool syn_transport_udp_readable (
    SYN_Socket sock_handle
) 
```





**Parameters:**


* `sock_handle` Native UDP socket handle. 



**Returns:**

true if unread UDP bytes are queued, false otherwise. 





        

<hr>



### function syn\_transport\_udp\_recvfrom 

_Non-blocking receive from a bound native UDP socket handle._ 
```C++
int syn_transport_udp_recvfrom (
    SYN_Socket sock_handle,
    void * buf,
    size_t max_len,
    SYN_SockAddr * from,
    uint32_t timeout_ms
) 
```





**Parameters:**


* `sock_handle` Native UDP socket handle. 
* `buf` [out] Destination buffer. 
* `max_len` Destination buffer capacity. 
* `from` [out] Optional sender endpoint. 
* `timeout_ms` Unused in non-blocking native bridge (0). 



**Returns:**

Bytes copied (&gt; 0), 0 if no datagram pending, or -1 on invalid handle/buffer. 





        

<hr>



### function syn\_transport\_udp\_sendto 

_Transmit a UDP datagram via the bound native UDP instance and Ethernet port._ 
```C++
int syn_transport_udp_sendto (
    SYN_Socket sock_handle,
    const void * data,
    size_t len,
    const SYN_SockAddr * to
) 
```





**Note:**

Outbound Ethernet frames are assembled in a shared module-static 1514-byte buffer (`g_udp_tx_buf [1514]`). Callers must invoke this function only from the single cooperative scheduler/network task context (not from ISRs or concurrent preemptive threads).




**Parameters:**


* `sock_handle` Native UDP socket handle. 
* `data` Datagram payload bytes. 
* `len` Payload byte length. 
* `to` Destination IPv4 endpoint. 



**Returns:**

Number of payload bytes transmitted, or -1 on error. 





        

<hr>



### function syn\_transport\_udp\_set\_eth\_tx 

_Override the Ethernet transmit function used by syn\_transport\_udp\_sendto._ 
```C++
void syn_transport_udp_set_eth_tx (
    SYN_TransportEthTxFn tx_fn
) 
```





**Parameters:**


* `tx_fn` Custom transmit callback, or NULL to restore default syn\_port\_eth\_tx. 




        

<hr>



### function syn\_transport\_udp\_set\_instance 

_Bind native syn\_udp instance to the syn\_port\_udp\_\* socket interface._ 
```C++
void syn_transport_udp_set_instance (
    SYN_UDP * udp
) 
```





**Parameters:**


* `udp` Pointer to native [**SYN\_UDP**](structSYN__UDP.md) stack instance. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_transport_udp.c`

