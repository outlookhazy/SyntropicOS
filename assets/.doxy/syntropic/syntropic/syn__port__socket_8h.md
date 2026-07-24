

# File syn\_port\_socket.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_socket.h**](syn__port__socket_8h.md)

[Go to the source code of this file](syn__port__socket_8h_source.md)

_TCP socket port interface — implement these for your platform._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include "../common/syn_defs.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_SockAddr**](structSYN__SockAddr.md) <br> |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef int | [**SYN\_Socket**](#typedef-syn_socket)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**syn\_port\_sock\_accept**](#function-syn_port_sock_accept) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) listener, uint32\_t timeout\_ms) <br>_Accept an incoming connection on a listener socket._  |
|  void | [**syn\_port\_sock\_close**](#function-syn_port_sock_close) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock) <br>_Close a socket and release resources._  |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**syn\_port\_sock\_connect**](#function-syn_port_sock_connect) (const [**SYN\_SockAddr**](structSYN__SockAddr.md) \* addr) <br>_Connect to a remote host by IP address._  |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**syn\_port\_sock\_connect\_host**](#function-syn_port_sock_connect_host) (const char \* host, uint16\_t port) <br>_Connect to a remote host by hostname._  |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**syn\_port\_sock\_listen**](#function-syn_port_sock_listen) (uint16\_t port, int backlog) <br>_Create a listening TCP socket on the given port._  |
|  int | [**syn\_port\_sock\_recv**](#function-syn_port_sock_recv) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock, void \* buf, size\_t max\_len, uint32\_t timeout\_ms) <br>_Receive data from a connected socket._  |
|  int | [**syn\_port\_sock\_send**](#function-syn_port_sock_send) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock, const void \* data, size\_t len) <br>_Send data over a connected socket._  |
|  int | [**syn\_port\_sock\_send\_all**](#function-syn_port_sock_send_all) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock, const void \* data, size\_t len) <br>_Send all data over a connected socket._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_udp\_join\_multicast**](#function-syn_port_udp_join_multicast) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock, const char \* multicast\_ip) <br>_Join a multicast group._  |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**syn\_port\_udp\_open**](#function-syn_port_udp_open) (uint16\_t port) <br>_Open a UDP socket bound to the given port._  |
|  int | [**syn\_port\_udp\_recvfrom**](#function-syn_port_udp_recvfrom) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock, void \* buf, size\_t max\_len, [**SYN\_SockAddr**](structSYN__SockAddr.md) \* from, uint32\_t timeout\_ms) <br>_Receive a UDP packet._  |
|  int | [**syn\_port\_udp\_sendto**](#function-syn_port_udp_sendto) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock, const void \* data, size\_t len, const [**SYN\_SockAddr**](structSYN__SockAddr.md) \* to) <br>_Send a UDP packet to a destination address._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_SOCKET\_INVALID**](syn__port__socket_8h.md#define-syn_socket_invalid)  `(-1)`<br>_Sentinel value for an invalid/uninitialized socket._  |

## Detailed Description


Minimal BSD-like socket abstraction for TCP client connections. Higher-level modules (HTTP client, MQTT, OTA) use this interface without knowing the underlying TCP/IP stack.


Implement these functions by wrapping your platform's socket API:
* lwIP (raw or socket mode)
* ESP-IDF (POSIX sockets via lwIP)
* POSIX (Linux/macOS for development)
* Vendor TCP/IP stack






**
**


```C++
SYN_Socket sock = syn_port_sock_connect_host("example.com", 80);
if (sock < 0) { handle error }

syn_port_sock_send(sock, "GET / HTTP/1.1\r\n\r\n", 18);

uint8_t buf[256];
int n = syn_port_sock_recv(sock, buf, sizeof(buf), 5000);

syn_port_sock_close(sock);
```
 





    
## Public Types Documentation




### typedef SYN\_Socket 

```C++
typedef int SYN_Socket;
```



Socket handle. Negative values indicate error / invalid. 


        

<hr>
## Public Functions Documentation




### function syn\_port\_sock\_accept 

_Accept an incoming connection on a listener socket._ 
```C++
SYN_Socket syn_port_sock_accept (
    SYN_Socket listener,
    uint32_t timeout_ms
) 
```



Blocks until a connection arrives or the timeout expires.




**Parameters:**


* `listener` Listener socket from [**syn\_port\_sock\_listen()**](syn__port__socket_8h.md#function-syn_port_sock_listen). 
* `timeout_ms` Timeout in milliseconds (0 = block indefinitely). 



**Returns:**

Connected socket handle, or SYN\_SOCKET\_INVALID on timeout/error. 





        

<hr>



### function syn\_port\_sock\_close 

_Close a socket and release resources._ 
```C++
void syn_port_sock_close (
    SYN_Socket sock
) 
```





**Parameters:**


* `sock` Socket handle to close. 




        

<hr>



### function syn\_port\_sock\_connect 

_Connect to a remote host by IP address._ 
```C++
SYN_Socket syn_port_sock_connect (
    const SYN_SockAddr * addr
) 
```



Opens a TCP connection. Blocking.




**Parameters:**


* `addr` Remote address (IPv4 + port). 



**Returns:**

Socket handle on success, SYN\_SOCKET\_INVALID on failure. 





        

<hr>



### function syn\_port\_sock\_connect\_host 

_Connect to a remote host by hostname._ 
```C++
SYN_Socket syn_port_sock_connect_host (
    const char * host,
    uint16_t port
) 
```



Performs DNS resolution and opens a TCP connection. Blocking.




**Parameters:**


* `host` Hostname or dotted-decimal IP string. 
* `port` Port number. 



**Returns:**

Socket handle on success, SYN\_SOCKET\_INVALID on failure. 





        

<hr>



### function syn\_port\_sock\_listen 

_Create a listening TCP socket on the given port._ 
```C++
SYN_Socket syn_port_sock_listen (
    uint16_t port,
    int backlog
) 
```



Binds to all interfaces and begins listening for connections.




**Parameters:**


* `port` Port number to listen on. 
* `backlog` Maximum pending connections (typically 1–5). 



**Returns:**

Listener socket handle, or SYN\_SOCKET\_INVALID on failure. 





        

<hr>



### function syn\_port\_sock\_recv 

_Receive data from a connected socket._ 
```C++
int syn_port_sock_recv (
    SYN_Socket sock,
    void * buf,
    size_t max_len,
    uint32_t timeout_ms
) 
```



Returns available data up to max\_len. Blocks up to timeout\_ms.




**Parameters:**


* `sock` Socket handle. 
* `buf` Receive buffer. 
* `max_len` Buffer capacity. 
* `timeout_ms` Timeout in milliseconds (0 = non-blocking). 



**Returns:**

Number of bytes received, 0 if connection closed, -1 on error/timeout. 





        

<hr>



### function syn\_port\_sock\_send 

_Send data over a connected socket._ 
```C++
int syn_port_sock_send (
    SYN_Socket sock,
    const void * data,
    size_t len
) 
```



May send fewer bytes than requested (partial send).




**Parameters:**


* `sock` Socket handle. 
* `data` Data to send. 
* `len` Number of bytes to send. 



**Returns:**

Number of bytes sent, or -1 on error. 





        

<hr>



### function syn\_port\_sock\_send\_all 

_Send all data over a connected socket._ 
```C++
int syn_port_sock_send_all (
    SYN_Socket sock,
    const void * data,
    size_t len
) 
```



Loops internally until all bytes are sent or an error occurs.




**Parameters:**


* `sock` Socket handle. 
* `data` Data to send. 
* `len` Number of bytes to send. 



**Returns:**

Number of bytes sent (== len on success), or -1 on error. 





        

<hr>



### function syn\_port\_udp\_join\_multicast 

_Join a multicast group._ 
```C++
SYN_Status syn_port_udp_join_multicast (
    SYN_Socket sock,
    const char * multicast_ip
) 
```





**Parameters:**


* `sock` UDP socket handle. 
* `multicast_ip` Multicast IP string (e.g. "224.0.0.251"). 



**Returns:**

SYN\_OK on success, SYN\_ERROR on failure. 





        

<hr>



### function syn\_port\_udp\_open 

_Open a UDP socket bound to the given port._ 
```C++
SYN_Socket syn_port_udp_open (
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
int syn_port_udp_recvfrom (
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
int syn_port_udp_sendto (
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
## Macro Definition Documentation





### define SYN\_SOCKET\_INVALID 

_Sentinel value for an invalid/uninitialized socket._ 
```C++
#define SYN_SOCKET_INVALID `(-1)`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_socket.h`

