

# File syn\_transport\_udp.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_transport\_udp.h**](syn__transport__udp_8h.md)

[Go to the source code of this file](syn__transport__udp_8h_source.md)

_Bridge binding syn\_port\_udp\_\* API to native syn\_udp stack._ 

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/net/syn_udp.h"`
* `#include "syntropic/port/syn_port_socket.h"`

















## Public Types

| Type | Name |
| ---: | :--- |
| typedef [**SYN\_Status**](syn__defs_8h.md#enum-syn_status)(\* | [**SYN\_TransportEthTxFn**](#typedef-syn_transportethtxfn)  <br>_Ethernet frame transmit callback signature for the native UDP bridge._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_transport\_udp\_close**](#function-syn_transport_udp_close) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock\_handle) <br>_Close and unbind a native UDP socket handle._  |
|  [**SYN\_UDP**](structSYN__UDP.md) \* | [**syn\_transport\_udp\_get\_instance**](#function-syn_transport_udp_get_instance) (void) <br>_Get the active native_ [_**SYN\_UDP**_](structSYN__UDP.md) _stack instance._ |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**syn\_transport\_udp\_open**](#function-syn_transport_udp_open) (uint16\_t port) <br>_Open and bind a native UDP socket handle (assigning ephemeral port when_ `port` _is 0)._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_transport\_udp\_readable**](#function-syn_transport_udp_readable) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock\_handle) <br>_Check whether a bound native UDP socket has queued receive bytes._  |
|  int | [**syn\_transport\_udp\_recvfrom**](#function-syn_transport_udp_recvfrom) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock\_handle, void \* buf, size\_t max\_len, [**SYN\_SockAddr**](structSYN__SockAddr.md) \* from, uint32\_t timeout\_ms) <br>_Non-blocking receive from a bound native UDP socket handle._  |
|  int | [**syn\_transport\_udp\_sendto**](#function-syn_transport_udp_sendto) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock\_handle, const void \* data, size\_t len, const [**SYN\_SockAddr**](structSYN__SockAddr.md) \* to) <br>_Transmit a UDP datagram via the bound native UDP instance and Ethernet port._  |
|  void | [**syn\_transport\_udp\_set\_eth\_tx**](#function-syn_transport_udp_set_eth_tx) ([**SYN\_TransportEthTxFn**](syn__transport__udp_8h.md#typedef-syn_transportethtxfn) tx\_fn) <br>_Override the Ethernet transmit function used by syn\_transport\_udp\_sendto._  |
|  void | [**syn\_transport\_udp\_set\_instance**](#function-syn_transport_udp_set_instance) ([**SYN\_UDP**](structSYN__UDP.md) \* udp) <br>_Bind native syn\_udp instance to the syn\_port\_udp\_\* socket interface._  |




























## Public Types Documentation




### typedef SYN\_TransportEthTxFn 

_Ethernet frame transmit callback signature for the native UDP bridge._ 
```C++
typedef SYN_Status(* SYN_TransportEthTxFn) (const void *frame, size_t len);
```





**Parameters:**


* `frame` Pointer to Ethernet frame bytes. 
* `len` Ethernet frame byte length. 



**Returns:**

SYN\_OK on success, error status otherwise. 





        

<hr>
## Public Functions Documentation




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
The documentation for this class was generated from the following file `src/syntropic/net/syn_transport_udp.h`

