

# File syn\_tcp.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_tcp.h**](syn__tcp_8h.md)

[Go to the source code of this file](syn__tcp_8h_source.md)

_Zero-Alloc Native TCP State Machine & Segment Engine._ 

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/net/syn_eth.h"`
* `#include "syntropic/pt/syn_pt.h"`
* `#include "syntropic/sched/syn_task.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`
* `#include <string.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_TCP**](structSYN__TCP.md) <br>_Top-level TCP stack instance container._  |
| struct | [**SYN\_TcpConn**](structSYN__TcpConn.md) <br>_Single TCP socket connection control block._  |
| struct | [**SYN\_TcpHeader**](structSYN__TcpHeader.md) <br>_TCP packet header structure._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_TcpState**](#enum-syn_tcpstate)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**syn\_tcp\_checksum**](#function-syn_tcp_checksum) (uint32\_t src\_ip, uint32\_t dst\_ip, const uint8\_t \* tcp\_seg, size\_t len) <br>_Compute TCP 16-bit 1's complement checksum over pseudo-header and segment._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_tcp\_close**](#function-syn_tcp_close) ([**SYN\_TCP**](structSYN__TCP.md) \* tcp, [**SYN\_TcpConn**](structSYN__TcpConn.md) \* conn) <br>_Close a TCP connection and re-arm passive listener if configured._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_tcp\_init**](#function-syn_tcp_init) ([**SYN\_TCP**](structSYN__TCP.md) \* tcp, [**SYN\_ETH**](structSYN__ETH.md) \* eth) <br>_Initialize TCP Stack._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_tcp\_listen**](#function-syn_tcp_listen) ([**SYN\_TCP**](structSYN__TCP.md) \* tcp, uint16\_t port) <br>_Bind TCP listener to port._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_tcp\_process\_packet**](#function-syn_tcp_process_packet) ([**SYN\_TCP**](structSYN__TCP.md) \* tcp, const uint8\_t \* ip\_packet, size\_t len, uint8\_t \* tx\_out, size\_t \* tx\_len) <br>_Process incoming TCP IPv4 segment._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_tcp\_pt\_args\_valid**](#function-syn_tcp_pt_args_valid) ([**SYN\_PT**](structSYN__PT.md) \* pt, const [**SYN\_Task**](structSYN__Task.md) \* task, const [**SYN\_TcpConn**](structSYN__TcpConn.md) \* conn, const void \* buf, const size\_t \* bytes\_read) <br>_Validate pointer arguments for PT\_TCP\_BLOCK\_READ without triggering -Waddress._  |
|  void | [**syn\_tcp\_pt\_recv**](#function-syn_tcp_pt_recv) ([**SYN\_TcpConn**](structSYN__TcpConn.md) \* conn, void \* buf, size\_t max\_len, size\_t \* bytes\_read) <br>_Drain up to_ `max_len` _received payload bytes from a woken TCP connection._ |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**PT\_TCP\_BLOCK\_READ**](syn__tcp_8h.md#define-pt_tcp_block_read) (pt, task, conn, buf, max\_len, bytes\_read) `/* multi line expression */`<br>_Protothread helper macro to block task until TCP data is available._  |
| define  | [**SYN\_TCP\_BUF\_SIZE**](syn__tcp_8h.md#define-syn_tcp_buf_size)  `512`<br> |
| define  | [**SYN\_TCP\_FLAG\_ACK**](syn__tcp_8h.md#define-syn_tcp_flag_ack)  `0x10`<br> |
| define  | [**SYN\_TCP\_FLAG\_FIN**](syn__tcp_8h.md#define-syn_tcp_flag_fin)  `0x01`<br> |
| define  | [**SYN\_TCP\_FLAG\_PSH**](syn__tcp_8h.md#define-syn_tcp_flag_psh)  `0x08`<br> |
| define  | [**SYN\_TCP\_FLAG\_RST**](syn__tcp_8h.md#define-syn_tcp_flag_rst)  `0x04`<br> |
| define  | [**SYN\_TCP\_FLAG\_SYN**](syn__tcp_8h.md#define-syn_tcp_flag_syn)  `0x02`<br> |
| define  | [**SYN\_TCP\_MAX\_CONNS**](syn__tcp_8h.md#define-syn_tcp_max_conns)  `1`<br> |
| define  | [**SYN\_TCP\_SYN\_TIMEOUT\_MS**](syn__tcp_8h.md#define-syn_tcp_syn_timeout_ms)  `3000U`<br> |

## Public Types Documentation




### enum SYN\_TcpState 

```C++
enum SYN_TcpState {
    SYN_TCP_CLOSED = 0,
    SYN_TCP_LISTEN = 1,
    SYN_TCP_SYN_RCVD = 2,
    SYN_TCP_ESTABLISHED = 3,
    SYN_TCP_FIN_WAIT = 4,
    SYN_TCP_CLOSE_WAIT = 5
};
```



TCP Connection State Machine Enum. 


        

<hr>
## Public Functions Documentation




### function syn\_tcp\_checksum 

_Compute TCP 16-bit 1's complement checksum over pseudo-header and segment._ 
```C++
uint16_t syn_tcp_checksum (
    uint32_t src_ip,
    uint32_t dst_ip,
    const uint8_t * tcp_seg,
    size_t len
) 
```





**Parameters:**


* `src_ip` Source 32-bit IPv4 address. 
* `dst_ip` Destination 32-bit IPv4 address. 
* `tcp_seg` Pointer to raw TCP segment bytes. 
* `len` TCP segment length in bytes. 



**Returns:**

Computed 16-bit TCP checksum. 





        

<hr>



### function syn\_tcp\_close 

_Close a TCP connection and re-arm passive listener if configured._ 
```C++
SYN_Status syn_tcp_close (
    SYN_TCP * tcp,
    SYN_TcpConn * conn
) 
```





**Parameters:**


* `tcp` Pointer to TCP stack container. 
* `conn` Pointer to TCP connection slot. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on NULL pointer. 





        

<hr>



### function syn\_tcp\_init 

_Initialize TCP Stack._ 
```C++
SYN_Status syn_tcp_init (
    SYN_TCP * tcp,
    SYN_ETH * eth
) 
```





**Parameters:**


* `tcp` Pointer to TCP stack container. 
* `eth` Pointer to Ethernet interface engine instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_tcp\_listen 

_Bind TCP listener to port._ 
```C++
SYN_Status syn_tcp_listen (
    SYN_TCP * tcp,
    uint16_t port
) 
```





**Parameters:**


* `tcp` Pointer to TCP stack container. 
* `port` Port number to listen on. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_tcp\_process\_packet 

_Process incoming TCP IPv4 segment._ 
```C++
SYN_Status syn_tcp_process_packet (
    SYN_TCP * tcp,
    const uint8_t * ip_packet,
    size_t len,
    uint8_t * tx_out,
    size_t * tx_len
) 
```





**Parameters:**


* `tcp` Pointer to TCP stack container. 
* `ip_packet` Pointer to raw Ethernet II + IPv4 + TCP frame. 
* `len` Frame byte length (&gt;= 54 bytes). 
* `tx_out` Output buffer for immediate TCP reply/RST frame (at least 54 bytes). 
* `tx_len` Pointer to receive output frame byte length (0 if none generated). 



**Returns:**

SYN\_OK on matched segment processing, SYN\_ERROR on invalid/unmatched segment. 





        

<hr>
## Public Static Functions Documentation




### function syn\_tcp\_pt\_args\_valid 

_Validate pointer arguments for PT\_TCP\_BLOCK\_READ without triggering -Waddress._ 
```C++
static inline bool syn_tcp_pt_args_valid (
    SYN_PT * pt,
    const SYN_Task * task,
    const SYN_TcpConn * conn,
    const void * buf,
    const size_t * bytes_read
) 
```





**Parameters:**


* `pt` Protothread state pointer. 
* `task` Task instance pointer. 
* `conn` TCP connection pointer. 
* `buf` Destination read buffer pointer. 
* `bytes_read` Output byte count pointer. 



**Returns:**

true if all pointers are non-NULL, false otherwise (reinitializing pt if non-NULL). 





        

<hr>



### function syn\_tcp\_pt\_recv 

_Drain up to_ `max_len` _received payload bytes from a woken TCP connection._
```C++
static inline void syn_tcp_pt_recv (
    SYN_TcpConn * conn,
    void * buf,
    size_t max_len,
    size_t * bytes_read
) 
```





**Parameters:**


* `conn` TCP connection pointer. 
* `buf` Destination read buffer pointer. 
* `max_len` Maximum bytes to copy into `buf`. 
* `bytes_read` Output byte count pointer (set to 0 if connection closed or empty). 




        

<hr>
## Macro Definition Documentation





### define PT\_TCP\_BLOCK\_READ 

_Protothread helper macro to block task until TCP data is available._ 
```C++
#define PT_TCP_BLOCK_READ (
    pt,
    task,
    conn,
    buf,
    max_len,
    bytes_read
) `/* multi line expression */`
```





**Parameters:**


* `pt` Protothread state. 
* `task` Task instance. 
* `conn` TCP connection pointer. 
* `buf` Destination read buffer. 
* `max_len` Maximum bytes to read. 
* `bytes_read` Output variable receiving count of read bytes. 




        

<hr>



### define SYN\_TCP\_BUF\_SIZE 

```C++
#define SYN_TCP_BUF_SIZE `512`
```



TCP RX/TX internal payload buffer size in bytes 


        

<hr>



### define SYN\_TCP\_FLAG\_ACK 

```C++
#define SYN_TCP_FLAG_ACK `0x10`
```



Acknowledgment field significant flag 


        

<hr>



### define SYN\_TCP\_FLAG\_FIN 

```C++
#define SYN_TCP_FLAG_FIN `0x01`
```



TCP Header Flags Finish flag: No more data from sender 


        

<hr>



### define SYN\_TCP\_FLAG\_PSH 

```C++
#define SYN_TCP_FLAG_PSH `0x08`
```



Push function flag 


        

<hr>



### define SYN\_TCP\_FLAG\_RST 

```C++
#define SYN_TCP_FLAG_RST `0x04`
```



Reset connection flag 


        

<hr>



### define SYN\_TCP\_FLAG\_SYN 

```C++
#define SYN_TCP_FLAG_SYN `0x02`
```



Synchronize sequence numbers flag 


        

<hr>



### define SYN\_TCP\_MAX\_CONNS 

```C++
#define SYN_TCP_MAX_CONNS `1`
```



Maximum concurrent TCP connections 


        

<hr>



### define SYN\_TCP\_SYN\_TIMEOUT\_MS 

```C++
#define SYN_TCP_SYN_TIMEOUT_MS `3000U`
```



Half-open SYN\_RCVD slot reclamation timeout in ms 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_tcp.h`

