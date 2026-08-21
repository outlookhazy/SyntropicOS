

# File syn\_mqtt.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_mqtt.h**](syn__mqtt_8h.md)

[Go to the source code of this file](syn__mqtt_8h_source.md)

_Lightweight MQTT 3.1.1 client._ 

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_socket.h"`
* `#include "../pt/syn_pt.h"`
* `#include "../sched/syn_task.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Mqtt5\_UserProp**](structSYN__Mqtt5__UserProp.md) <br>_MQTT v5.0 Key-Value User Property._  |
| struct | [**SYN\_MqttClient**](structSYN__MqttClient.md) <br>_MQTT client context structure._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_MqttRxPhase**](#enum-syn_mqttrxphase)  <br>_Non-blocking packet reception states._  |
| enum  | [**SYN\_MqttState**](#enum-syn_mqttstate)  <br>_MQTT client connection states._  |
| enum  | [**SYN\_MqttVersion**](#enum-syn_mqttversion)  <br>_MQTT Protocol Version._  |




















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



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_MQTT5\_PROP\_ASSIGNED\_CLIENT\_ID**](syn__mqtt_8h.md#define-syn_mqtt5_prop_assigned_client_id)  `0x12U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_AUTH\_DATA**](syn__mqtt_8h.md#define-syn_mqtt5_prop_auth_data)  `0x16U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_AUTH\_METHOD**](syn__mqtt_8h.md#define-syn_mqtt5_prop_auth_method)  `0x15U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_CONTENT\_TYPE**](syn__mqtt_8h.md#define-syn_mqtt5_prop_content_type)  `0x03U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_CORRELATION\_DATA**](syn__mqtt_8h.md#define-syn_mqtt5_prop_correlation_data)  `0x09U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_MAXIMUM\_QOS**](syn__mqtt_8h.md#define-syn_mqtt5_prop_maximum_qos)  `0x24U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_MAX\_PACKET\_SIZE**](syn__mqtt_8h.md#define-syn_mqtt5_prop_max_packet_size)  `0x27U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_MESSAGE\_EXPIRY\_INTERVAL**](syn__mqtt_8h.md#define-syn_mqtt5_prop_message_expiry_interval)  `0x02U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_PAYLOAD\_FORMAT\_INDICATOR**](syn__mqtt_8h.md#define-syn_mqtt5_prop_payload_format_indicator)  `0x01U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_REASON\_STRING**](syn__mqtt_8h.md#define-syn_mqtt5_prop_reason_string)  `0x1FU`<br> |
| define  | [**SYN\_MQTT5\_PROP\_RECEIVE\_MAXIMUM**](syn__mqtt_8h.md#define-syn_mqtt5_prop_receive_maximum)  `0x21U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_REQ\_PROBLEM\_INFO**](syn__mqtt_8h.md#define-syn_mqtt5_prop_req_problem_info)  `0x17U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_REQ\_RESPONSE\_INFO**](syn__mqtt_8h.md#define-syn_mqtt5_prop_req_response_info)  `0x19U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_RESPONSE\_INFO**](syn__mqtt_8h.md#define-syn_mqtt5_prop_response_info)  `0x1AU`<br> |
| define  | [**SYN\_MQTT5\_PROP\_RESPONSE\_TOPIC**](syn__mqtt_8h.md#define-syn_mqtt5_prop_response_topic)  `0x08U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_RETAIN\_AVAILABLE**](syn__mqtt_8h.md#define-syn_mqtt5_prop_retain_available)  `0x25U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_SERVER\_KEEP\_ALIVE**](syn__mqtt_8h.md#define-syn_mqtt5_prop_server_keep_alive)  `0x13U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_SERVER\_REFERENCE**](syn__mqtt_8h.md#define-syn_mqtt5_prop_server_reference)  `0x1CU`<br> |
| define  | [**SYN\_MQTT5\_PROP\_SESSION\_EXPIRY\_INTERVAL**](syn__mqtt_8h.md#define-syn_mqtt5_prop_session_expiry_interval)  `0x11U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_SHARED\_SUB\_AVAIL**](syn__mqtt_8h.md#define-syn_mqtt5_prop_shared_sub_avail)  `0x2AU`<br> |
| define  | [**SYN\_MQTT5\_PROP\_SUBSCRIPTION\_IDENTIFIER**](syn__mqtt_8h.md#define-syn_mqtt5_prop_subscription_identifier)  `0x0BU`<br> |
| define  | [**SYN\_MQTT5\_PROP\_SUB\_ID\_AVAIL**](syn__mqtt_8h.md#define-syn_mqtt5_prop_sub_id_avail)  `0x29U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_TOPIC\_ALIAS**](syn__mqtt_8h.md#define-syn_mqtt5_prop_topic_alias)  `0x23U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_TOPIC\_ALIAS\_MAXIMUM**](syn__mqtt_8h.md#define-syn_mqtt5_prop_topic_alias_maximum)  `0x22U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_USER\_PROPERTY**](syn__mqtt_8h.md#define-syn_mqtt5_prop_user_property)  `0x26U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_WILDCARD\_SUB\_AVAIL**](syn__mqtt_8h.md#define-syn_mqtt5_prop_wildcard_sub_avail)  `0x28U`<br> |
| define  | [**SYN\_MQTT5\_PROP\_WILL\_DELAY\_INTERVAL**](syn__mqtt_8h.md#define-syn_mqtt5_prop_will_delay_interval)  `0x18U`<br> |

## Public Types Documentation




### enum SYN\_MqttRxPhase 

_Non-blocking packet reception states._ 
```C++
enum SYN_MqttRxPhase {
    SYN_MQTT_RX_IDLE,
    SYN_MQTT_RX_REMAINING_LEN,
    SYN_MQTT_RX_PAYLOAD,
    SYN_MQTT_RX_DISCARD
};
```




<hr>



### enum SYN\_MqttState 

_MQTT client connection states._ 
```C++
enum SYN_MqttState {
    SYN_MQTT_DISCONNECTED,
    SYN_MQTT_CONNECTING,
    SYN_MQTT_CONNECTED
};
```




<hr>



### enum SYN\_MqttVersion 

_MQTT Protocol Version._ 
```C++
enum SYN_MqttVersion {
    SYN_MQTT_VERSION_3_1_1 = 4,
    SYN_MQTT_VERSION_5_0 = 5
};
```




<hr>
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

PT\_WAITING or PT\_EXITED status. 





        

<hr>
## Macro Definition Documentation





### define SYN\_MQTT5\_PROP\_ASSIGNED\_CLIENT\_ID 

```C++
#define SYN_MQTT5_PROP_ASSIGNED_CLIENT_ID `0x12U`
```



Assigned Client Identifier 


        

<hr>



### define SYN\_MQTT5\_PROP\_AUTH\_DATA 

```C++
#define SYN_MQTT5_PROP_AUTH_DATA `0x16U`
```



Authentication Data 


        

<hr>



### define SYN\_MQTT5\_PROP\_AUTH\_METHOD 

```C++
#define SYN_MQTT5_PROP_AUTH_METHOD `0x15U`
```



Authentication Method 


        

<hr>



### define SYN\_MQTT5\_PROP\_CONTENT\_TYPE 

```C++
#define SYN_MQTT5_PROP_CONTENT_TYPE `0x03U`
```



Content Type 


        

<hr>



### define SYN\_MQTT5\_PROP\_CORRELATION\_DATA 

```C++
#define SYN_MQTT5_PROP_CORRELATION_DATA `0x09U`
```



Correlation Data 


        

<hr>



### define SYN\_MQTT5\_PROP\_MAXIMUM\_QOS 

```C++
#define SYN_MQTT5_PROP_MAXIMUM_QOS `0x24U`
```



Maximum QoS 


        

<hr>



### define SYN\_MQTT5\_PROP\_MAX\_PACKET\_SIZE 

```C++
#define SYN_MQTT5_PROP_MAX_PACKET_SIZE `0x27U`
```



Maximum Packet Size 


        

<hr>



### define SYN\_MQTT5\_PROP\_MESSAGE\_EXPIRY\_INTERVAL 

```C++
#define SYN_MQTT5_PROP_MESSAGE_EXPIRY_INTERVAL `0x02U`
```



Message Expiry Interval 


        

<hr>



### define SYN\_MQTT5\_PROP\_PAYLOAD\_FORMAT\_INDICATOR 

```C++
#define SYN_MQTT5_PROP_PAYLOAD_FORMAT_INDICATOR `0x01U`
```



Payload Format Indicator 


        

<hr>



### define SYN\_MQTT5\_PROP\_REASON\_STRING 

```C++
#define SYN_MQTT5_PROP_REASON_STRING `0x1FU`
```



Reason String 


        

<hr>



### define SYN\_MQTT5\_PROP\_RECEIVE\_MAXIMUM 

```C++
#define SYN_MQTT5_PROP_RECEIVE_MAXIMUM `0x21U`
```



Receive Maximum 


        

<hr>



### define SYN\_MQTT5\_PROP\_REQ\_PROBLEM\_INFO 

```C++
#define SYN_MQTT5_PROP_REQ_PROBLEM_INFO `0x17U`
```



Request Problem Information 


        

<hr>



### define SYN\_MQTT5\_PROP\_REQ\_RESPONSE\_INFO 

```C++
#define SYN_MQTT5_PROP_REQ_RESPONSE_INFO `0x19U`
```



Request Response Information 


        

<hr>



### define SYN\_MQTT5\_PROP\_RESPONSE\_INFO 

```C++
#define SYN_MQTT5_PROP_RESPONSE_INFO `0x1AU`
```



Response Information 


        

<hr>



### define SYN\_MQTT5\_PROP\_RESPONSE\_TOPIC 

```C++
#define SYN_MQTT5_PROP_RESPONSE_TOPIC `0x08U`
```



Response Topic 


        

<hr>



### define SYN\_MQTT5\_PROP\_RETAIN\_AVAILABLE 

```C++
#define SYN_MQTT5_PROP_RETAIN_AVAILABLE `0x25U`
```



Retain Available 


        

<hr>



### define SYN\_MQTT5\_PROP\_SERVER\_KEEP\_ALIVE 

```C++
#define SYN_MQTT5_PROP_SERVER_KEEP_ALIVE `0x13U`
```



Server Keep Alive 


        

<hr>



### define SYN\_MQTT5\_PROP\_SERVER\_REFERENCE 

```C++
#define SYN_MQTT5_PROP_SERVER_REFERENCE `0x1CU`
```



Server Reference 


        

<hr>



### define SYN\_MQTT5\_PROP\_SESSION\_EXPIRY\_INTERVAL 

```C++
#define SYN_MQTT5_PROP_SESSION_EXPIRY_INTERVAL `0x11U`
```



Session Expiry Interval 


        

<hr>



### define SYN\_MQTT5\_PROP\_SHARED\_SUB\_AVAIL 

```C++
#define SYN_MQTT5_PROP_SHARED_SUB_AVAIL `0x2AU`
```



Shared Subscription Available 


        

<hr>



### define SYN\_MQTT5\_PROP\_SUBSCRIPTION\_IDENTIFIER 

```C++
#define SYN_MQTT5_PROP_SUBSCRIPTION_IDENTIFIER `0x0BU`
```



Subscription Identifier 


        

<hr>



### define SYN\_MQTT5\_PROP\_SUB\_ID\_AVAIL 

```C++
#define SYN_MQTT5_PROP_SUB_ID_AVAIL `0x29U`
```



Subscription Identifiers Available 


        

<hr>



### define SYN\_MQTT5\_PROP\_TOPIC\_ALIAS 

```C++
#define SYN_MQTT5_PROP_TOPIC_ALIAS `0x23U`
```



Topic Alias 


        

<hr>



### define SYN\_MQTT5\_PROP\_TOPIC\_ALIAS\_MAXIMUM 

```C++
#define SYN_MQTT5_PROP_TOPIC_ALIAS_MAXIMUM `0x22U`
```



Topic Alias Maximum 


        

<hr>



### define SYN\_MQTT5\_PROP\_USER\_PROPERTY 

```C++
#define SYN_MQTT5_PROP_USER_PROPERTY `0x26U`
```



User Property 


        

<hr>



### define SYN\_MQTT5\_PROP\_WILDCARD\_SUB\_AVAIL 

```C++
#define SYN_MQTT5_PROP_WILDCARD_SUB_AVAIL `0x28U`
```



Wildcard Subscription Available 


        

<hr>



### define SYN\_MQTT5\_PROP\_WILL\_DELAY\_INTERVAL 

```C++
#define SYN_MQTT5_PROP_WILL_DELAY_INTERVAL `0x18U`
```



Will Delay Interval 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_mqtt.h`

