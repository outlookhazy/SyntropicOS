

# File syn\_mqtt.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_mqtt.c**](syn__mqtt_8c.md)

[Go to the source code of this file](syn__mqtt_8c_source.md)

_Lightweight MQTT 3.1.1 client implementation._ 

* `#include "syn_mqtt.h"`
* `#include "../util/syn_assert.h"`
* `#include "../port/syn_port_system.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqtt\_init**](#function-syn_mqtt_init) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* client, const char \* host, uint16\_t port, const char \* client\_id, const char \* username, const char \* password, uint16\_t keep\_alive\_s, uint8\_t \* rx\_buf, size\_t rx\_buf\_size, uint8\_t \* tx\_buf, size\_t tx\_buf\_size) <br>_Initialize the MQTT client._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqtt\_publish**](#function-syn_mqtt_publish) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* client, const char \* topic, const void \* payload, size\_t len, uint8\_t qos, [**bool**](syn__defs_8h.md#enum-bool) retain) <br>_Publish a message to a topic._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqtt\_subscribe**](#function-syn_mqtt_subscribe) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* client, const char \* topic, uint8\_t qos) <br>_Subscribe to a topic._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_mqtt\_task**](#function-syn_mqtt_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative task for driving the MQTT client._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**encode\_remaining\_len**](#function-encode_remaining_len) (uint8\_t \* buf, uint32\_t len) <br>_Encode MQTT remaining length into variable-length bytes._  |
|  void | [**handle\_publish**](#function-handle_publish) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* c, const uint8\_t \* payload, uint32\_t len, uint8\_t qos\_bits) <br>_Handle an incoming PUBLISH packet._  |
|  int | [**read\_all**](#function-read_all) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock, uint8\_t \* buf, size\_t len, uint32\_t timeout\_ms) <br>_Read exactly_ `len` _bytes from socket._ |
|  int | [**read\_remaining\_len**](#function-read_remaining_len) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock, uint32\_t \* len\_out, uint32\_t timeout\_ms) <br>_Read MQTT variable-length "remaining length" field from socket._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**send\_mqtt\_connect**](#function-send_mqtt_connect) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* c) <br>_Build and send an MQTT CONNECT packet._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**send\_mqtt\_ping**](#function-send_mqtt_ping) (const [**SYN\_MqttClient**](structSYN__MqttClient.md) \* c) <br>_Send an MQTT PINGREQ packet._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**MQTT\_ACK\_TIMEOUT\_MS**](syn__mqtt_8c.md#define-mqtt_ack_timeout_ms)  `5000`<br> |
| define  | [**MQTT\_RECV\_TIMEOUT\_MS**](syn__mqtt_8c.md#define-mqtt_recv_timeout_ms)  `500`<br> |

## Public Functions Documentation




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

PT\_WAITING or PT\_EXITED status. 





        

<hr>
## Public Static Functions Documentation




### function encode\_remaining\_len 

_Encode MQTT remaining length into variable-length bytes._ 
```C++
static size_t encode_remaining_len (
    uint8_t * buf,
    uint32_t len
) 
```





**Parameters:**


* `buf` Output buffer (at least 4 bytes). 
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



### function read\_all 

_Read exactly_ `len` _bytes from socket._
```C++
static int read_all (
    SYN_Socket sock,
    uint8_t * buf,
    size_t len,
    uint32_t timeout_ms
) 
```





**Parameters:**


* `sock` Socket. 
* `buf` Output buffer. 
* `len` Bytes to read. 
* `timeout_ms` Timeout per recv call. 



**Returns:**

Total bytes read, or -1 on error. 





        

<hr>



### function read\_remaining\_len 

_Read MQTT variable-length "remaining length" field from socket._ 
```C++
static int read_remaining_len (
    SYN_Socket sock,
    uint32_t * len_out,
    uint32_t timeout_ms
) 
```





**Parameters:**


* `sock` Socket. 
* `len_out` [out] Decoded length. 
* `timeout_ms` Timeout. 



**Returns:**

0 on success, -1 on error. 





        

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
#define MQTT_RECV_TIMEOUT_MS `500`
```



Default receive timeout (ms). 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_mqtt.c`

