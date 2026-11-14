

# File syn\_mqtt.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_mqtt.c**](syn__mqtt_8c.md)

[Go to the source code of this file](syn__mqtt_8c_source.md)

_Lightweight MQTT 3.1.1 client implementation — fully non-blocking._ 

* `#include "../port/syn_port_system.h"`
* `#include "../sched/syn_sched.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_metrics.h"`
* `#include "../util/syn_pack.h"`
* `#include "syn_mqtt.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_mqtt5\_encode\_user\_prop**](#function-syn_mqtt5_encode_user_prop) (const char \* key, const char \* val, uint8\_t \* buf, size\_t max\_buf\_len) <br>_Encode an MQTT 5.0 User Property (Key-Value string pair)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_mqtt\_decode\_varint**](#function-syn_mqtt_decode_varint) (const uint8\_t \* buf, size\_t buf\_len, uint32\_t \* val, size\_t \* bytes\_read) <br>_Decode a Variable Byte Integer (MQTT 3.1.1 & MQTT 5.0)._  |
|  void | [**syn\_mqtt\_disconnect**](#function-syn_mqtt_disconnect) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* client) <br>_Disconnect the MQTT client and close underlying TCP socket._  |
|  size\_t | [**syn\_mqtt\_encode\_varint**](#function-syn_mqtt_encode_varint) (uint32\_t val, uint8\_t buf) <br>_Encode a Variable Byte Integer (MQTT 3.1.1 & MQTT 5.0)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqtt\_init**](#function-syn_mqtt_init) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* client, const char \* host, uint16\_t port, const char \* client\_id, const char \* username, const char \* password, uint16\_t keep\_alive\_s, uint8\_t \* rx\_buf, size\_t rx\_buf\_size, uint8\_t \* tx\_buf, size\_t tx\_buf\_size) <br>_Initialize the MQTT client._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqtt\_ping**](#function-syn_mqtt_ping) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* client) <br>_Transmit an explicit MQTT PINGREQ packet._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqtt\_publish**](#function-syn_mqtt_publish) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* client, const char \* topic, const void \* payload, size\_t len, uint8\_t qos, [**bool**](syn__defs_8h.md#enum-bool) retain) <br>_Publish a message to a topic._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqtt\_subscribe**](#function-syn_mqtt_subscribe) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* client, const char \* topic, uint8\_t qos) <br>_Subscribe to a topic._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_mqtt\_task**](#function-syn_mqtt_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative task for driving the MQTT client._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**encode\_remaining\_len**](#function-encode_remaining_len) (uint8\_t \* buf, uint32\_t len) <br>_Encode remaining length field._  |
|  void | [**handle\_publish**](#function-handle_publish) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* c, const uint8\_t \* payload, uint32\_t len, uint8\_t qos\_bits) <br>_Handle an incoming PUBLISH packet._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**mqtt\_connack\_valid**](#function-mqtt_connack_valid) (const [**SYN\_MqttClient**](structSYN__MqttClient.md) \* c) <br>_Validate an inbound MQTT 3.1.1 CONNACK packet._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**mqtt\_connect\_creds\_len**](#function-mqtt_connect_creds_len) (const [**SYN\_MqttClient**](structSYN__MqttClient.md) \* c, [**bool**](syn__defs_8h.md#enum-bool) \* has\_user, [**bool**](syn__defs_8h.md#enum-bool) \* has\_pass, size\_t \* u\_len, size\_t \* p\_len) <br>_Compute validated MQTT 3.1.1 CONNECT credential flags and lengths._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**mqtt\_connected\_timers\_due**](#function-mqtt_connected_timers_due) (const [**SYN\_MqttClient**](structSYN__MqttClient.md) \* c, uint32\_t now) <br>_Check whether QoS 1 retransmit or keep-alive timers have elapsed while connected._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**mqtt\_has\_work**](#function-mqtt_has_work) (const [**SYN\_MqttClient**](structSYN__MqttClient.md) \* c) <br>_Check if MQTT client task has actionable work._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**mqtt\_more\_rx\_ready**](#function-mqtt_more_rx_ready) (const [**SYN\_MqttClient**](structSYN__MqttClient.md) \* c, [**bool**](syn__defs_8h.md#enum-bool) progressed) <br>_Check whether sustained RX progress left additional readable bytes on an active socket._  |
|  uint16\_t | [**mqtt\_next\_packet\_id**](#function-mqtt_next_packet_id) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* c) <br>_Allocate the next non-zero 16-bit MQTT packet identifier._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**mqtt\_publish\_args\_ok**](#function-mqtt_publish_args_ok) (const [**SYN\_MqttClient**](structSYN__MqttClient.md) \* client, const char \* topic, const void \* payload, size\_t len, uint8\_t qos) <br>_Validate arguments and client state for syn\_mqtt\_publish._  |
|  void | [**mqtt\_put\_str**](#function-mqtt_put_str) (uint8\_t \* tx, size\_t \* pos, const char \* str, uint16\_t len) <br>_Append a 16-bit length-prefixed UTF-8 string into an MQTT packet buffer._  |
|  int | [**mqtt\_recv\_checked**](#function-mqtt_recv_checked) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* c, uint8\_t \* buf, size\_t len) <br>_Non-blocking socket read that resets session on EOF or readable-error._  |
|  void | [**mqtt\_reset\_session**](#function-mqtt_reset_session) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* c) <br>_Close any open socket and reset transient MQTT session and QoS 1 retransmit state._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**mqtt\_rx\_discard**](#function-mqtt_rx_discard) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* c) <br>_Drain oversized packet payload bytes without buffering._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**mqtt\_rx\_idle**](#function-mqtt_rx_idle) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* c) <br>_Read and process the 1-byte fixed header opcode in SYN\_MQTT\_RX\_IDLE._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**mqtt\_rx\_payload**](#function-mqtt_rx_payload) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* c) <br>_Accumulate payload bytes into c-&gt;rx\_buf and dispatch when complete._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**mqtt\_rx\_rem\_len**](#function-mqtt_rx_rem_len) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* c) <br>_Read and decode one remaining-length varint byte._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**mqtt\_rx\_step**](#function-mqtt_rx_step) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* c) <br>_Advance the active MQTT RX parser phase by one socket read step._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**mqtt\_send\_tracked**](#function-mqtt_send_tracked) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* c, const uint8\_t \* buf, size\_t len) <br>_Transmit an MQTT packet on an active session, updating activity or resetting on error._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**mqtt\_step\_active**](#function-mqtt_step_active) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* c) <br>_Advance an active (CONNECTING or CONNECTED) MQTT session by one non-blocking slice._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**mqtt\_step\_keepalive**](#function-mqtt_step_keepalive) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* c, uint32\_t now) <br>_Enforce PINGRESP timeout or transmit due keep-alive PINGREQ while connected._  |
|  void | [**mqtt\_store\_retransmit**](#function-mqtt_store_retransmit) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* c, const uint8\_t \* tx, size\_t len) <br>_Cache an encoded QoS 1 PUBLISH packet in the bounded retransmit buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**mqtt\_try\_connect**](#function-mqtt_try_connect) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* c) <br>_Open a fresh TCP socket and send MQTT CONNECT._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**poll\_rx**](#function-poll_rx) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* c) <br>_Non-blocking receive state machine poll (at most 1 payload packet or 32 steps)._  |
|  void | [**process\_packet**](#function-process_packet) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* c) <br>_Dispatch a fully received MQTT packet._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**send\_mqtt\_connect**](#function-send_mqtt_connect) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* c) <br>_Build and send an MQTT CONNECT packet._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**send\_mqtt\_ping**](#function-send_mqtt_ping) (const [**SYN\_MqttClient**](structSYN__MqttClient.md) \* c) <br>_Send an MQTT PINGREQ packet._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**MQTT\_ACK\_TIMEOUT\_MS**](syn__mqtt_8c.md#define-mqtt_ack_timeout_ms)  `5000`<br> |
| define  | [**MQTT\_RECV\_TIMEOUT\_MS**](syn__mqtt_8c.md#define-mqtt_recv_timeout_ms)  `5000`<br> |

## Public Functions Documentation




### function syn\_mqtt5\_encode\_user\_prop 

_Encode an MQTT 5.0 User Property (Key-Value string pair)._ 
```C++
size_t syn_mqtt5_encode_user_prop (
    const char * key,
    const char * val,
    uint8_t * buf,
    size_t max_buf_len
) 
```





**Parameters:**


* `key` Property key string. 
* `val` Property value string. 
* `buf` [out] Output buffer. 
* `max_buf_len` Capacity of output buffer. 



**Returns:**

Number of bytes written, or 0 on error/overflow. 





        

<hr>



### function syn\_mqtt\_decode\_varint 

_Decode a Variable Byte Integer (MQTT 3.1.1 & MQTT 5.0)._ 
```C++
bool syn_mqtt_decode_varint (
    const uint8_t * buf,
    size_t buf_len,
    uint32_t * val,
    size_t * bytes_read
) 
```





**Parameters:**


* `buf` Buffer containing varint bytes. 
* `buf_len` Available bytes in buffer. 
* `val` [out] Parsed integer value. 
* `bytes_read` [out] Number of bytes consumed (1..4). 



**Returns:**

true on success, false if incomplete or malformed (&gt; 4 bytes). 





        

<hr>



### function syn\_mqtt\_disconnect 

_Disconnect the MQTT client and close underlying TCP socket._ 
```C++
void syn_mqtt_disconnect (
    SYN_MqttClient * client
) 
```



Sends an MQTT DISCONNECT packet if currently connected, then closes the socket and transitions client state to DISCONNECTED.




**Parameters:**


* `client` Pointer to client context. 




        

<hr>



### function syn\_mqtt\_encode\_varint 

_Encode a Variable Byte Integer (MQTT 3.1.1 & MQTT 5.0)._ 
```C++
size_t syn_mqtt_encode_varint (
    uint32_t val,
    uint8_t buf
) 
```





**Parameters:**


* `val` Value to encode (0..268435455). 
* `buf` [out] Output buffer (must have at least 4 bytes capacity). 



**Returns:**

Number of bytes written (1..4). 





        

<hr>



### function syn\_mqtt\_init 

_Initialize the MQTT client._ 
```C++
SYN_Status syn_mqtt_init (
    SYN_MqttClient * client,
    const char * host,
    uint16_t port,
    const char * client_id,
    const char * username,
    const char * password,
    uint16_t keep_alive_s,
    uint8_t * rx_buf,
    size_t rx_buf_size,
    uint8_t * tx_buf,
    size_t tx_buf_size
) 
```



Configures broker destination, client ID, authentication credentials, keep-alive timing parameters, and network packet buffers.




**Parameters:**


* `client` Pointer to client context. 
* `host` Broker network address string. 
* `port` Broker port number. 
* `client_id` MQTT client identity string. 
* `username` Authentication username (or NULL). 
* `password` Authentication password (or NULL). 
* `keep_alive_s` Keep-alive timeout parameter in seconds. 
* `rx_buf` Receive buffer storage. 
* `rx_buf_size` Receive buffer capacity. 
* `tx_buf` Transmit buffer storage. 
* `tx_buf_size` Transmit buffer capacity. 



**Returns:**

SYN\_OK on successful configuration, or error parameter code. 





        

<hr>



### function syn\_mqtt\_ping 

_Transmit an explicit MQTT PINGREQ packet._ 
```C++
SYN_Status syn_mqtt_ping (
    SYN_MqttClient * client
) 
```



Note: PINGREQ packets are sent automatically by syn\_mqtt\_task based on the configured keep\_alive\_s interval. This function allows manual pinging on demand.




**Parameters:**


* `client` Pointer to client context. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if not connected or transmit failed. 





        

<hr>



### function syn\_mqtt\_publish 

_Publish a message to a topic._ 
```C++
SYN_Status syn_mqtt_publish (
    SYN_MqttClient * client,
    const char * topic,
    const void * payload,
    size_t len,
    uint8_t qos,
    bool retain
) 
```



Non-blocking publish command. For QoS 0, queued directly. For QoS 1, tracks acknowledgement state.




**Parameters:**


* `client` Pointer to client context. 
* `topic` Topic name to target. 
* `payload` Data payload to send. 
* `len` Payload size in bytes. 
* `qos` Quality of service level (0 or 1). 
* `retain` Retain flag on broker. 



**Returns:**

SYN\_OK on queued, or error status if payload bounds exceeded. 





        

<hr>



### function syn\_mqtt\_subscribe 

_Subscribe to a topic._ 
```C++
SYN_Status syn_mqtt_subscribe (
    SYN_MqttClient * client,
    const char * topic,
    uint8_t qos
) 
```



Formats and queues a subscription request for transmission.




**Parameters:**


* `client` Pointer to client context. 
* `topic` Topic filter string. 
* `qos` Requested quality of service. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_mqtt\_task 

_Cooperative task for driving the MQTT client._ 
```C++
SYN_PT_Status syn_mqtt_task (
    SYN_PT * pt,
    SYN_Task * task
) 
```



Yields during connection, socket polling, keep-alive pinging, and packet parsing loops. Runs within the cooperative scheduler context.




**Parameters:**


* `pt` Cooperative protothread handle. 
* `task` Corresponding task control block. 



**Returns:**

PT\_IDLE while waiting, PT\_ENDED when complete, or PT\_EXITED on abort. 





        

<hr>
## Public Static Functions Documentation




### function encode\_remaining\_len 

_Encode remaining length field._ 
```C++
static size_t encode_remaining_len (
    uint8_t * buf,
    uint32_t len
) 
```





**Parameters:**


* `buf` Destination buffer. 
* `len` Length value to encode. 



**Returns:**

Number of bytes written. 





        

<hr>



### function handle\_publish 

_Handle an incoming PUBLISH packet._ 
```C++
static void handle_publish (
    SYN_MqttClient * c,
    const uint8_t * payload,
    uint32_t len,
    uint8_t qos_bits
) 
```





**Parameters:**


* `c` MQTT client. 
* `payload` Raw payload (after fixed header). 
* `len` Payload length. 
* `qos_bits` QoS flags from the fixed header. 




        

<hr>



### function mqtt\_connack\_valid 

_Validate an inbound MQTT 3.1.1 CONNACK packet._ 
```C++
static bool mqtt_connack_valid (
    const SYN_MqttClient * c
) 
```





**Parameters:**


* `c` MQTT client context with complete packet in rx\_buf. 



**Returns:**

true if CONNACK is well-formed and accepted while CONNECTING, false otherwise. 





        

<hr>



### function mqtt\_connect\_creds\_len 

_Compute validated MQTT 3.1.1 CONNECT credential flags and lengths._ 
```C++
static bool mqtt_connect_creds_len (
    const SYN_MqttClient * c,
    bool * has_user,
    bool * has_pass,
    size_t * u_len,
    size_t * p_len
) 
```





**Parameters:**


* `c` MQTT client context. 
* `has_user` [out] Set true when username is present. 
* `has_pass` [out] Set true when both username and password are present. 
* `u_len` [out] Username byte length. 
* `p_len` [out] Password byte length. 



**Returns:**

true if credential lengths fit in 16-bit MQTT strings, false otherwise. 





        

<hr>



### function mqtt\_connected\_timers\_due 

_Check whether QoS 1 retransmit or keep-alive timers have elapsed while connected._ 
```C++
static bool mqtt_connected_timers_due (
    const SYN_MqttClient * c,
    uint32_t now
) 
```





**Parameters:**


* `c` MQTT client context. 
* `now` Current millisecond tick. 



**Returns:**

true if a connected-state timer is due. 





        

<hr>



### function mqtt\_has\_work 

_Check if MQTT client task has actionable work._ 
```C++
static bool mqtt_has_work (
    const SYN_MqttClient * c
) 
```





**Parameters:**


* `c` Pointer to MQTT client instance. 



**Returns:**

true if work pending, false otherwise. 





        

<hr>



### function mqtt\_more\_rx\_ready 

_Check whether sustained RX progress left additional readable bytes on an active socket._ 
```C++
static bool mqtt_more_rx_ready (
    const SYN_MqttClient * c,
    bool progressed
) 
```





**Parameters:**


* `c` MQTT client context. 
* `progressed` Whether the preceding poll slice consumed bytes. 



**Returns:**

true if another cooperative RX slice should run immediately. 





        

<hr>



### function mqtt\_next\_packet\_id 

_Allocate the next non-zero 16-bit MQTT packet identifier._ 
```C++
static uint16_t mqtt_next_packet_id (
    SYN_MqttClient * c
) 
```





**Parameters:**


* `c` MQTT client context. 



**Returns:**

Non-zero packet identifier (1..65535). 





        

<hr>



### function mqtt\_publish\_args\_ok 

_Validate arguments and client state for syn\_mqtt\_publish._ 
```C++
static bool mqtt_publish_args_ok (
    const SYN_MqttClient * client,
    const char * topic,
    const void * payload,
    size_t len,
    uint8_t qos
) 
```





**Parameters:**


* `client` MQTT client context. 
* `topic` Topic string. 
* `payload` Payload pointer. 
* `len` Payload length. 
* `qos` Requested QoS level (0 or 1). 



**Returns:**

true if valid and client is connected, false otherwise. 





        

<hr>



### function mqtt\_put\_str 

_Append a 16-bit length-prefixed UTF-8 string into an MQTT packet buffer._ 
```C++
static void mqtt_put_str (
    uint8_t * tx,
    size_t * pos,
    const char * str,
    uint16_t len
) 
```





**Parameters:**


* `tx` Destination buffer. 
* `pos` [in,out] Write cursor. 
* `str` Source string bytes. 
* `len` String length (&lt;= UINT16\_MAX). 




        

<hr>



### function mqtt\_recv\_checked 

_Non-blocking socket read that resets session on EOF or readable-error._ 
```C++
static int mqtt_recv_checked (
    SYN_MqttClient * c,
    uint8_t * buf,
    size_t len
) 
```





**Parameters:**


* `c` MQTT client context. 
* `buf` Destination byte buffer. 
* `len` Maximum bytes to read. 



**Returns:**

Bytes read (&gt; 0), or -1 on no data / session closed. 





        

<hr>



### function mqtt\_reset\_session 

_Close any open socket and reset transient MQTT session and QoS 1 retransmit state._ 
```C++
static void mqtt_reset_session (
    SYN_MqttClient * c
) 
```





**Parameters:**


* `c` MQTT client context. 




        

<hr>



### function mqtt\_rx\_discard 

_Drain oversized packet payload bytes without buffering._ 
```C++
static bool mqtt_rx_discard (
    SYN_MqttClient * c
) 
```





**Parameters:**


* `c` MQTT client context. 



**Returns:**

true if discard bytes were consumed. 





        

<hr>



### function mqtt\_rx\_idle 

_Read and process the 1-byte fixed header opcode in SYN\_MQTT\_RX\_IDLE._ 
```C++
static bool mqtt_rx_idle (
    SYN_MqttClient * c
) 
```





**Parameters:**


* `c` MQTT client context. 



**Returns:**

true if a byte was consumed. 





        

<hr>



### function mqtt\_rx\_payload 

_Accumulate payload bytes into c-&gt;rx\_buf and dispatch when complete._ 
```C++
static bool mqtt_rx_payload (
    SYN_MqttClient * c
) 
```





**Parameters:**


* `c` MQTT client context. 



**Returns:**

true if payload bytes were consumed. 





        

<hr>



### function mqtt\_rx\_rem\_len 

_Read and decode one remaining-length varint byte._ 
```C++
static bool mqtt_rx_rem_len (
    SYN_MqttClient * c
) 
```





**Parameters:**


* `c` MQTT client context. 



**Returns:**

true if a valid byte was consumed without resetting the session. 





        

<hr>



### function mqtt\_rx\_step 

_Advance the active MQTT RX parser phase by one socket read step._ 
```C++
static bool mqtt_rx_step (
    SYN_MqttClient * c
) 
```





**Parameters:**


* `c` MQTT client context. 



**Returns:**

true if at least one byte was consumed in the active phase. 





        

<hr>



### function mqtt\_send\_tracked 

_Transmit an MQTT packet on an active session, updating activity or resetting on error._ 
```C++
static bool mqtt_send_tracked (
    SYN_MqttClient * c,
    const uint8_t * buf,
    size_t len
) 
```





**Parameters:**


* `c` MQTT client context. 
* `buf` Packet bytes to transmit. 
* `len` Number of bytes to transmit. 



**Returns:**

true if all bytes sent, false if transport failed and session was reset. 





        

<hr>



### function mqtt\_step\_active 

_Advance an active (CONNECTING or CONNECTED) MQTT session by one non-blocking slice._ 
```C++
static bool mqtt_step_active (
    SYN_MqttClient * c
) 
```





**Parameters:**


* `c` MQTT client context. 



**Returns:**

true if RX polling consumed at least one byte from the socket. 





        

<hr>



### function mqtt\_step\_keepalive 

_Enforce PINGRESP timeout or transmit due keep-alive PINGREQ while connected._ 
```C++
static bool mqtt_step_keepalive (
    SYN_MqttClient * c,
    uint32_t now
) 
```





**Parameters:**


* `c` MQTT client context. 
* `now` Current millisecond tick. 



**Returns:**

true if session remains connected, false if session was reset. 





        

<hr>



### function mqtt\_store\_retransmit 

_Cache an encoded QoS 1 PUBLISH packet in the bounded retransmit buffer._ 
```C++
static void mqtt_store_retransmit (
    SYN_MqttClient * c,
    const uint8_t * tx,
    size_t len
) 
```





**Parameters:**


* `c` MQTT client context. 
* `tx` Encoded PUBLISH frame. 
* `len` Total frame length. 




        

<hr>



### function mqtt\_try\_connect 

_Open a fresh TCP socket and send MQTT CONNECT._ 
```C++
static bool mqtt_try_connect (
    SYN_MqttClient * c
) 
```





**Parameters:**


* `c` MQTT client context. 



**Returns:**

true if socket connected and CONNECT frame was sent. 





        

<hr>



### function poll\_rx 

_Non-blocking receive state machine poll (at most 1 payload packet or 32 steps)._ 
```C++
static bool poll_rx (
    SYN_MqttClient * c
) 
```





**Parameters:**


* `c` MQTT client context. 



**Returns:**

true if at least one socket read consumed &gt;= 1 byte. 





        

<hr>



### function process\_packet 

_Dispatch a fully received MQTT packet._ 
```C++
static void process_packet (
    SYN_MqttClient * c
) 
```





**Parameters:**


* `c` MQTT client context. 




        

<hr>



### function send\_mqtt\_connect 

_Build and send an MQTT CONNECT packet._ 
```C++
static bool send_mqtt_connect (
    SYN_MqttClient * c
) 
```





**Parameters:**


* `c` MQTT client. 



**Returns:**

true on success. 





        

<hr>



### function send\_mqtt\_ping 

_Send an MQTT PINGREQ packet._ 
```C++
static bool send_mqtt_ping (
    const SYN_MqttClient * c
) 
```





**Parameters:**


* `c` MQTT client. 



**Returns:**

true on success. 





        

<hr>
## Macro Definition Documentation





### define MQTT\_ACK\_TIMEOUT\_MS 

```C++
#define MQTT_ACK_TIMEOUT_MS `5000`
```



Timeout for ACK responses (ms). 


        

<hr>



### define MQTT\_RECV\_TIMEOUT\_MS 

```C++
#define MQTT_RECV_TIMEOUT_MS `5000`
```



Timeout for incomplete packet reception (ms). 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_mqtt.c`

