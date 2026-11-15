

# File syn\_tcp.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_tcp.c**](syn__tcp_8c.md)

[Go to the source code of this file](syn__tcp_8c_source.md)

_Zero-Alloc Native TCP State Machine & Segment Engine Implementation._ 

* `#include "syntropic/net/syn_tcp.h"`
* `#include "syntropic/port/syn_port_system.h"`
* `#include "syntropic/sched/syn_sched.h"`
* `#include "syntropic/util/syn_assert.h"`
* `#include "syntropic/util/syn_pack.h"`
* `#include <string.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_TcpCtrlSpec**](structSYN__TcpCtrlSpec.md) <br>_Compact control-segment header parameters for 54-byte TCP frame serialization._  |






















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
|  void | [**build\_tcp\_segment**](#function-build_tcp_segment) ([**SYN\_TCP**](structSYN__TCP.md) \* tcp, [**SYN\_TcpConn**](structSYN__TcpConn.md) \* conn, uint8\_t flags, uint8\_t \* tx\_out, size\_t \* tx\_len) <br>_Build outbound control TCP segment frame (0-byte payload) into buffer._  |
|  void | [**tcp\_dispatch\_segment**](#function-tcp_dispatch_segment) ([**SYN\_TCP**](structSYN__TCP.md) \* tcp, [**SYN\_TcpConn**](structSYN__TcpConn.md) \* conn, const uint8\_t \* frame, size\_t tcp\_off, uint8\_t data\_offset, size\_t payload\_len, uint8\_t \* tx\_out, size\_t \* tx\_len) <br>_Dispatch a validated TCP segment to a matched connection state machine._  |
|  void | [**tcp\_emit\_ctrl\_frame**](#function-tcp_emit_ctrl_frame) (const [**SYN\_TCP**](structSYN__TCP.md) \* tcp, const [**SYN\_TcpCtrlSpec**](structSYN__TcpCtrlSpec.md) \* spec, uint8\_t \* tx\_out, size\_t \* tx\_len) <br>_Serialize a 54-byte 0-payload Ethernet II + IPv4 + TCP control segment._  |
|  [**SYN\_TcpConn**](structSYN__TcpConn.md) \* | [**tcp\_find\_conn**](#function-tcp_find_conn) ([**SYN\_TCP**](structSYN__TCP.md) \* tcp, uint32\_t src\_ip, uint16\_t src\_port, uint16\_t dst\_port, uint8\_t flags) <br>_Locate an active or listening TCP connection matching packet tuple._  |
|  void | [**tcp\_handle\_established**](#function-tcp_handle_established) ([**SYN\_TCP**](structSYN__TCP.md) \* tcp, [**SYN\_TcpConn**](structSYN__TcpConn.md) \* conn, uint32\_t seq\_num, uint8\_t flags, const uint8\_t \* payload, size\_t payload\_len, uint8\_t \* tx\_out, size\_t \* tx\_len) <br>_Handle payload and/or FIN on an ESTABLISHED TCP connection._  |
|  void | [**tcp\_handle\_syn\_rcvd**](#function-tcp_handle_syn_rcvd) ([**SYN\_TCP**](structSYN__TCP.md) \* tcp, [**SYN\_TcpConn**](structSYN__TcpConn.md) \* conn, const uint8\_t \* frame, size\_t tcp\_off, uint32\_t seq\_num, uint32\_t ack\_num, uint8\_t flags, const uint8\_t \* payload, size\_t payload\_len, uint8\_t \* tx\_out, size\_t \* tx\_len) <br>_Process an inbound segment on a connection in SYN\_TCP\_SYN\_RCVD state._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**tcp\_is\_listen\_port**](#function-tcp_is_listen_port) (const [**SYN\_TCP**](structSYN__TCP.md) \* tcp, uint16\_t port) <br>_Check whether a local TCP port is registered as a passive listener on this stack._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**tcp\_is\_pure\_syn**](#function-tcp_is_pure_syn) (uint8\_t flags) <br>_Check whether TCP flags represent a pure connection-initiating SYN segment._  |
|  [**SYN\_TcpConn**](structSYN__TcpConn.md) \* | [**tcp\_rearm\_closed\_listener**](#function-tcp_rearm_closed_listener) ([**SYN\_TCP**](structSYN__TCP.md) \* tcp, uint16\_t dst\_port) <br>_Re-arm a drained closed or expired half-open listener slot on SYN._  |
|  void | [**tcp\_send\_stateless\_rst**](#function-tcp_send_stateless_rst) (const [**SYN\_TCP**](structSYN__TCP.md) \* tcp, const uint8\_t \* frame, size\_t tcp\_off, size\_t payload\_len, uint8\_t \* tx\_out, size\_t \* tx\_len) <br>_Emit RFC 793 stateless RST reply for an unmatched inbound unicast non-RST segment._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**tcp\_validate\_frame**](#function-tcp_validate_frame) (const uint8\_t \* frame, size\_t len, size\_t \* tcp\_off\_out, uint8\_t \* data\_off\_out, size\_t \* payload\_len\_out) <br>_Validate Ethernet/IPv4/TCP header lengths and compute segment offsets._  |
|  void | [**tcp\_wake\_blocked**](#function-tcp_wake_blocked) ([**SYN\_TcpConn**](structSYN__TcpConn.md) \* conn) <br>_Detach and resume a task blocked on a TCP connection, if any._  |


























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




### function build\_tcp\_segment 

_Build outbound control TCP segment frame (0-byte payload) into buffer._ 
```C++
static void build_tcp_segment (
    SYN_TCP * tcp,
    SYN_TcpConn * conn,
    uint8_t flags,
    uint8_t * tx_out,
    size_t * tx_len
) 
```





**Parameters:**


* `tcp` Pointer to TCP stack. 
* `conn` Pointer to TCP connection block. 
* `flags` TCP control flags bitmask. 
* `tx_out` Output Ethernet frame buffer. 
* `tx_len` Output Ethernet frame length pointer. 




        

<hr>



### function tcp\_dispatch\_segment 

_Dispatch a validated TCP segment to a matched connection state machine._ 
```C++
static void tcp_dispatch_segment (
    SYN_TCP * tcp,
    SYN_TcpConn * conn,
    const uint8_t * frame,
    size_t tcp_off,
    uint8_t data_offset,
    size_t payload_len,
    uint8_t * tx_out,
    size_t * tx_len
) 
```





**Parameters:**


* `tcp` TCP stack container. 
* `conn` Matched connection slot. 
* `frame` Raw Ethernet frame bytes. 
* `tcp_off` Byte offset of TCP header inside frame. 
* `data_offset` TCP header byte length. 
* `payload_len` TCP payload byte count. 
* `tx_out` Outbound Ethernet frame buffer. 
* `tx_len` [out] Outbound Ethernet frame length. 




        

<hr>



### function tcp\_emit\_ctrl\_frame 

_Serialize a 54-byte 0-payload Ethernet II + IPv4 + TCP control segment._ 
```C++
static void tcp_emit_ctrl_frame (
    const SYN_TCP * tcp,
    const SYN_TcpCtrlSpec * spec,
    uint8_t * tx_out,
    size_t * tx_len
) 
```





**Parameters:**


* `tcp` Pointer to TCP stack. 
* `spec` Control header specification. 
* `tx_out` Output Ethernet frame buffer (&gt;= 54 bytes). 
* `tx_len` Output Ethernet frame length pointer. 




        

<hr>



### function tcp\_find\_conn 

_Locate an active or listening TCP connection matching packet tuple._ 
```C++
static SYN_TcpConn * tcp_find_conn (
    SYN_TCP * tcp,
    uint32_t src_ip,
    uint16_t src_port,
    uint16_t dst_port,
    uint8_t flags
) 
```





**Parameters:**


* `tcp` TCP stack container. 
* `src_ip` Peer IPv4 address. 
* `src_port` Peer TCP port. 
* `dst_port` Local destination TCP port. 
* `flags` TCP segment flags bitmask. 



**Returns:**

Matching connection slot, or NULL if none. 





        

<hr>



### function tcp\_handle\_established 

_Handle payload and/or FIN on an ESTABLISHED TCP connection._ 
```C++
static void tcp_handle_established (
    SYN_TCP * tcp,
    SYN_TcpConn * conn,
    uint32_t seq_num,
    uint8_t flags,
    const uint8_t * payload,
    size_t payload_len,
    uint8_t * tx_out,
    size_t * tx_len
) 
```





**Parameters:**


* `tcp` TCP stack container. 
* `conn` Established connection slot. 
* `seq_num` Segment sequence number. 
* `flags` TCP flags bitmask. 
* `payload` Pointer to segment payload bytes. 
* `payload_len` Segment payload byte count. 
* `tx_out` Outbound Ethernet frame buffer. 
* `tx_len` [out] Outbound Ethernet frame length. 




        

<hr>



### function tcp\_handle\_syn\_rcvd 

_Process an inbound segment on a connection in SYN\_TCP\_SYN\_RCVD state._ 
```C++
static void tcp_handle_syn_rcvd (
    SYN_TCP * tcp,
    SYN_TcpConn * conn,
    const uint8_t * frame,
    size_t tcp_off,
    uint32_t seq_num,
    uint32_t ack_num,
    uint8_t flags,
    const uint8_t * payload,
    size_t payload_len,
    uint8_t * tx_out,
    size_t * tx_len
) 
```





**Parameters:**


* `tcp` TCP stack container. 
* `conn` Connection slot in SYN\_TCP\_SYN\_RCVD. 
* `frame` Raw Ethernet frame bytes. 
* `tcp_off` Byte offset of TCP header inside frame. 
* `seq_num` Segment sequence number. 
* `ack_num` Segment acknowledgment number. 
* `flags` TCP flags bitmask. 
* `payload` Pointer to segment payload bytes. 
* `payload_len` Segment payload byte count. 
* `tx_out` Outbound Ethernet frame buffer. 
* `tx_len` [out] Outbound Ethernet frame length. 




        

<hr>



### function tcp\_is\_listen\_port 

_Check whether a local TCP port is registered as a passive listener on this stack._ 
```C++
static bool tcp_is_listen_port (
    const SYN_TCP * tcp,
    uint16_t port
) 
```





**Parameters:**


* `tcp` TCP stack container. 
* `port` Local port number. 



**Returns:**

true if port is an active listener port. 





        

<hr>



### function tcp\_is\_pure\_syn 

_Check whether TCP flags represent a pure connection-initiating SYN segment._ 
```C++
static bool tcp_is_pure_syn (
    uint8_t flags
) 
```





**Parameters:**


* `flags` TCP header flags byte. 



**Returns:**

true if SYN is set and ACK, FIN, and RST are clear. 





        

<hr>



### function tcp\_rearm\_closed\_listener 

_Re-arm a drained closed or expired half-open listener slot on SYN._ 
```C++
static SYN_TcpConn * tcp_rearm_closed_listener (
    SYN_TCP * tcp,
    uint16_t dst_port
) 
```





**Parameters:**


* `tcp` TCP stack container with listening == true. 
* `dst_port` Matched passive listener destination port. 



**Returns:**

Re-armed connection slot, or NULL if no eligible slot exists. 





        

<hr>



### function tcp\_send\_stateless\_rst 

_Emit RFC 793 stateless RST reply for an unmatched inbound unicast non-RST segment._ 
```C++
static void tcp_send_stateless_rst (
    const SYN_TCP * tcp,
    const uint8_t * frame,
    size_t tcp_off,
    size_t payload_len,
    uint8_t * tx_out,
    size_t * tx_len
) 
```





**Parameters:**


* `tcp` Pointer to TCP stack. 
* `frame` Inbound Ethernet II + IPv4 + TCP frame. 
* `tcp_off` Byte offset of TCP header inside frame. 
* `payload_len` Validated TCP payload byte length. 
* `tx_out` Output Ethernet frame buffer (&gt;= 54 bytes). 
* `tx_len` Output Ethernet frame length pointer. 




        

<hr>



### function tcp\_validate\_frame 

_Validate Ethernet/IPv4/TCP header lengths and compute segment offsets._ 
```C++
static SYN_Status tcp_validate_frame (
    const uint8_t * frame,
    size_t len,
    size_t * tcp_off_out,
    uint8_t * data_off_out,
    size_t * payload_len_out
) 
```





**Parameters:**


* `frame` Raw Ethernet frame bytes. 
* `len` Ethernet frame byte length (&gt;= 54). 
* `tcp_off_out` [out] Byte offset of TCP header inside frame. 
* `data_off_out` [out] TCP header length in bytes. 
* `payload_len_out` [out] TCP payload byte count. 



**Returns:**

SYN\_OK if valid TCP-over-IPv4 frame, SYN\_ERROR otherwise. 





        

<hr>



### function tcp\_wake\_blocked 

_Detach and resume a task blocked on a TCP connection, if any._ 
```C++
static void tcp_wake_blocked (
    SYN_TcpConn * conn
) 
```





**Parameters:**


* `conn` TCP connection pointer. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_tcp.c`

