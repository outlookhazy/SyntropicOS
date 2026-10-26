

# File syn\_transport\_udp.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_transport\_udp.c**](syn__transport__udp_8c.md)

[Go to the source code of this file](syn__transport__udp_8c_source.md)

_Bridge implementation connecting syn\_port\_udp\_\* to native syn\_udp stack._ 

* `#include "syntropic/net/syn_transport_udp.h"`
* `#include "port/syn_port_eth.h"`
* `#include "syntropic/util/syn_pack.h"`
* `#include <string.h>`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_UDP**](structSYN__UDP.md) \* | [**g\_native\_udp**](#variable-g_native_udp)   = `NULL`<br> |
|  uint8\_t | [**g\_udp\_tx\_buf**](#variable-g_udp_tx_buf)  <br> |














## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**syn\_port\_udp\_open**](#function-syn_port_udp_open) (uint16\_t port) <br>_Open a UDP socket bound to the given port._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) int | [**syn\_port\_udp\_recvfrom**](#function-syn_port_udp_recvfrom) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock, void \* buf, size\_t max\_len, [**SYN\_SockAddr**](structSYN__SockAddr.md) \* from, uint32\_t timeout\_ms) <br>_Receive a UDP packet._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) int | [**syn\_port\_udp\_sendto**](#function-syn_port_udp_sendto) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock, const void \* data, size\_t len, const [**SYN\_SockAddr**](structSYN__SockAddr.md) \* to) <br>_Send a UDP packet to a destination address._  |
|  [**SYN\_UDP**](structSYN__UDP.md) \* | [**syn\_transport\_udp\_get\_instance**](#function-syn_transport_udp_get_instance) (void) <br>_Get the active native_ [_**SYN\_UDP**_](structSYN__UDP.md) _stack instance._ |
|  void | [**syn\_transport\_udp\_set\_instance**](#function-syn_transport_udp_set_instance) ([**SYN\_UDP**](structSYN__UDP.md) \* udp) <br>_Bind native syn\_udp instance to the syn\_port\_udp\_\* socket interface._  |




























## Public Static Attributes Documentation




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

