

# File syn\_mqttsn.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_mqttsn.c**](syn__mqttsn_8c.md)

[Go to the source code of this file](syn__mqttsn_8c_source.md)

_MQTT for Sensor Networks (MQTT-SN v1.2) Protocol Client Engine Implementation._ 

* `#include "../port/syn_port_system.h"`
* `#include "syn_mqttsn.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqttsn\_client\_connect**](#function-syn_mqttsn_client_connect) ([**SYN\_MQTTSN\_Client**](structSYN__MQTTSN__Client.md) \* client) <br>_Send CONNECT packet to MQTT-SN gateway._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqttsn\_client\_disconnect**](#function-syn_mqttsn_client_disconnect) ([**SYN\_MQTTSN\_Client**](structSYN__MQTTSN__Client.md) \* client) <br>_Disconnect client from gateway._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqttsn\_client\_init**](#function-syn_mqttsn_client_init) ([**SYN\_MQTTSN\_Client**](structSYN__MQTTSN__Client.md) \* client, const [**SYN\_MQTTSN\_Config**](structSYN__MQTTSN__Config.md) \* cfg) <br>_Initialize MQTT-SN client._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_mqttsn\_client\_pt**](#function-syn_mqttsn_client_pt) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Protothread continuation function for_ `syn_sched` _._ |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqttsn\_client\_publish**](#function-syn_mqttsn_client_publish) ([**SYN\_MQTTSN\_Client**](structSYN__MQTTSN__Client.md) \* client, uint16\_t topic\_id, int8\_t qos, [**bool**](syn__defs_8h.md#enum-bool) retain, const uint8\_t \* payload, size\_t len) <br>_Publish data payload using a registered 16-bit Topic ID._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqttsn\_client\_publish\_predefined**](#function-syn_mqttsn_client_publish_predefined) ([**SYN\_MQTTSN\_Client**](structSYN__MQTTSN__Client.md) \* client, uint16\_t predef\_id, int8\_t qos, [**bool**](syn__defs_8h.md#enum-bool) retain, const uint8\_t \* payload, size\_t len) <br>_Publish data payload using a predefined 16-bit Topic ID._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqttsn\_client\_publish\_short**](#function-syn_mqttsn_client_publish_short) ([**SYN\_MQTTSN\_Client**](structSYN__MQTTSN__Client.md) \* client, const char short\_topic, int8\_t qos, [**bool**](syn__defs_8h.md#enum-bool) retain, const uint8\_t \* payload, size\_t len) <br>_Publish data payload using a short 2-character topic name._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqttsn\_client\_register\_topic**](#function-syn_mqttsn_client_register_topic) ([**SYN\_MQTTSN\_Client**](structSYN__MQTTSN__Client.md) \* client, const char \* topic\_name, uint16\_t \* out\_msg\_id) <br>_Register a topic name with gateway to obtain a 16-bit Topic ID._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqttsn\_client\_searchgw**](#function-syn_mqttsn_client_searchgw) ([**SYN\_MQTTSN\_Client**](structSYN__MQTTSN__Client.md) \* client, uint8\_t radius) <br>_Broadcast SEARCHGW to discover active MQTT-SN gateways._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqttsn\_client\_sleep**](#function-syn_mqttsn_client_sleep) ([**SYN\_MQTTSN\_Client**](structSYN__MQTTSN__Client.md) \* client, uint16\_t sleep\_duration\_s) <br>_Put client into low-power SLEEP state with specified sleep duration._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqttsn\_client\_step**](#function-syn_mqttsn_client_step) ([**SYN\_MQTTSN\_Client**](structSYN__MQTTSN__Client.md) \* client, uint32\_t now\_ms) <br>_Non-blocking poll step processing received frames and keep-alive ping timers._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqttsn\_client\_subscribe**](#function-syn_mqttsn_client_subscribe) ([**SYN\_MQTTSN\_Client**](structSYN__MQTTSN__Client.md) \* client, const char \* topic\_name, uint16\_t predef\_id, int8\_t qos) <br>_Subscribe to a topic name or predefined ID._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqttsn\_client\_unsubscribe**](#function-syn_mqttsn_client_unsubscribe) ([**SYN\_MQTTSN\_Client**](structSYN__MQTTSN__Client.md) \* client, const char \* topic\_name, uint16\_t predef\_id) <br>_Unsubscribe from a topic._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqttsn\_client\_wake**](#function-syn_mqttsn_client_wake) ([**SYN\_MQTTSN\_Client**](structSYN__MQTTSN__Client.md) \* client) <br>_Wake up from sleep to flush messages or return to connected state._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**mqttsn\_decode\_header**](#function-mqttsn_decode_header) (const uint8\_t \* buf, size\_t len, uint8\_t \* out\_msg\_type, size\_t \* out\_payload\_offset, size\_t \* out\_payload\_len) <br>_Decode MQTT-SN packet header from buffer._  |
|  size\_t | [**mqttsn\_encode\_header**](#function-mqttsn_encode_header) (uint8\_t \* buf, size\_t max\_buf, uint8\_t msg\_type, size\_t payload\_len) <br>_Encode MQTT-SN packet header into buffer._  |
|  [**SYN\_MQTTSN\_TopicEntry**](structSYN__MQTTSN__TopicEntry.md) \* | [**mqttsn\_find\_or\_add\_topic**](#function-mqttsn_find_or_add_topic) ([**SYN\_MQTTSN\_Client**](structSYN__MQTTSN__Client.md) \* client, const char \* topic\_name) <br>_Find or allocate topic entry in client table._  |
|  void | [**mqttsn\_process\_rx\_frame**](#function-mqttsn_process_rx_frame) ([**SYN\_MQTTSN\_Client**](structSYN__MQTTSN__Client.md) \* client, uint8\_t msg\_type, const uint8\_t \* payload, size\_t payload\_len) <br>_Handle incoming decoded MQTT-SN message frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**mqttsn\_publish\_internal**](#function-mqttsn_publish_internal) ([**SYN\_MQTTSN\_Client**](structSYN__MQTTSN__Client.md) \* client, uint16\_t topic\_id, uint8\_t topic\_type\_flag, int8\_t qos, [**bool**](syn__defs_8h.md#enum-bool) retain, const uint8\_t \* payload, size\_t len) <br>_Internal helper to publish with configurable topic ID type flag._  |


























## Public Functions Documentation




### function syn\_mqttsn\_client\_connect 

_Send CONNECT packet to MQTT-SN gateway._ 
```C++
SYN_Status syn_mqttsn_client_connect (
    SYN_MQTTSN_Client * client
) 
```





**Parameters:**


* `client` Client instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_mqttsn\_client\_disconnect 

_Disconnect client from gateway._ 
```C++
SYN_Status syn_mqttsn_client_disconnect (
    SYN_MQTTSN_Client * client
) 
```





**Parameters:**


* `client` Client instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_mqttsn\_client\_init 

_Initialize MQTT-SN client._ 
```C++
SYN_Status syn_mqttsn_client_init (
    SYN_MQTTSN_Client * client,
    const SYN_MQTTSN_Config * cfg
) 
```





**Parameters:**


* `client` Client instance pointer. 
* `cfg` Configuration descriptor. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid argument. 





        

<hr>



### function syn\_mqttsn\_client\_pt 

_Protothread continuation function for_ `syn_sched` _._
```C++
SYN_PT_Status syn_mqttsn_client_pt (
    SYN_PT * pt,
    SYN_Task * task
) 
```





**Parameters:**


* `pt` Protothread continuation structure. 
* `task` Task handle containing [**SYN\_MQTTSN\_Client**](structSYN__MQTTSN__Client.md) in user\_data. 



**Returns:**

SYN\_PT\_Status continuation status. 





        

<hr>



### function syn\_mqttsn\_client\_publish 

_Publish data payload using a registered 16-bit Topic ID._ 
```C++
SYN_Status syn_mqttsn_client_publish (
    SYN_MQTTSN_Client * client,
    uint16_t topic_id,
    int8_t qos,
    bool retain,
    const uint8_t * payload,
    size_t len
) 
```





**Parameters:**


* `client` Client instance. 
* `topic_id` 16-bit Topic ID. 
* `qos` Quality of Service (0, 1, or -1). 
* `retain` Retain flag. 
* `payload` Payload buffer. 
* `len` Payload length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_mqttsn\_client\_publish\_predefined 

_Publish data payload using a predefined 16-bit Topic ID._ 
```C++
SYN_Status syn_mqttsn_client_publish_predefined (
    SYN_MQTTSN_Client * client,
    uint16_t predef_id,
    int8_t qos,
    bool retain,
    const uint8_t * payload,
    size_t len
) 
```





**Parameters:**


* `client` Client instance. 
* `predef_id` Predefined 16-bit Topic ID. 
* `qos` Quality of Service (0, 1, or -1). 
* `retain` Retain flag. 
* `payload` Payload buffer. 
* `len` Payload length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_mqttsn\_client\_publish\_short 

_Publish data payload using a short 2-character topic name._ 
```C++
SYN_Status syn_mqttsn_client_publish_short (
    SYN_MQTTSN_Client * client,
    const char short_topic,
    int8_t qos,
    bool retain,
    const uint8_t * payload,
    size_t len
) 
```





**Parameters:**


* `client` Client instance. 
* `short_topic` 2-character topic string. 
* `qos` Quality of Service (0, 1, or -1). 
* `retain` Retain flag. 
* `payload` Payload buffer. 
* `len` Payload length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_mqttsn\_client\_register\_topic 

_Register a topic name with gateway to obtain a 16-bit Topic ID._ 
```C++
SYN_Status syn_mqttsn_client_register_topic (
    SYN_MQTTSN_Client * client,
    const char * topic_name,
    uint16_t * out_msg_id
) 
```





**Parameters:**


* `client` Client instance. 
* `topic_name` String topic name. 
* `out_msg_id` Optional pointer to receive allocated MsgId. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_mqttsn\_client\_searchgw 

_Broadcast SEARCHGW to discover active MQTT-SN gateways._ 
```C++
SYN_Status syn_mqttsn_client_searchgw (
    SYN_MQTTSN_Client * client,
    uint8_t radius
) 
```





**Parameters:**


* `client` Client instance. 
* `radius` Search radius hop count. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_mqttsn\_client\_sleep 

_Put client into low-power SLEEP state with specified sleep duration._ 
```C++
SYN_Status syn_mqttsn_client_sleep (
    SYN_MQTTSN_Client * client,
    uint16_t sleep_duration_s
) 
```





**Parameters:**


* `client` Client instance. 
* `sleep_duration_s` Sleep duration in seconds. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_mqttsn\_client\_step 

_Non-blocking poll step processing received frames and keep-alive ping timers._ 
```C++
SYN_Status syn_mqttsn_client_step (
    SYN_MQTTSN_Client * client,
    uint32_t now_ms
) 
```





**Parameters:**


* `client` Client instance. 
* `now_ms` Current timestamp in milliseconds. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_mqttsn\_client\_subscribe 

_Subscribe to a topic name or predefined ID._ 
```C++
SYN_Status syn_mqttsn_client_subscribe (
    SYN_MQTTSN_Client * client,
    const char * topic_name,
    uint16_t predef_id,
    int8_t qos
) 
```





**Parameters:**


* `client` Client instance. 
* `topic_name` Topic name string (or NULL if subscribing by predef\_id). 
* `predef_id` Predefined Topic ID (if topic\_name is NULL). 
* `qos` Requested QoS level. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_mqttsn\_client\_unsubscribe 

_Unsubscribe from a topic._ 
```C++
SYN_Status syn_mqttsn_client_unsubscribe (
    SYN_MQTTSN_Client * client,
    const char * topic_name,
    uint16_t predef_id
) 
```





**Parameters:**


* `client` Client instance. 
* `topic_name` Topic name string. 
* `predef_id` Predefined Topic ID. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_mqttsn\_client\_wake 

_Wake up from sleep to flush messages or return to connected state._ 
```C++
SYN_Status syn_mqttsn_client_wake (
    SYN_MQTTSN_Client * client
) 
```





**Parameters:**


* `client` Client instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Public Static Functions Documentation




### function mqttsn\_decode\_header 

_Decode MQTT-SN packet header from buffer._ 
```C++
static bool mqttsn_decode_header (
    const uint8_t * buf,
    size_t len,
    uint8_t * out_msg_type,
    size_t * out_payload_offset,
    size_t * out_payload_len
) 
```





**Parameters:**


* `buf` Input buffer. 
* `len` Buffer length. 
* `out_msg_type` Output message type. 
* `out_payload_offset` Output byte offset to payload. 
* `out_payload_len` Output payload length. 



**Returns:**

true if valid header, false if incomplete or malformed. 





        

<hr>



### function mqttsn\_encode\_header 

_Encode MQTT-SN packet header into buffer._ 
```C++
static size_t mqttsn_encode_header (
    uint8_t * buf,
    size_t max_buf,
    uint8_t msg_type,
    size_t payload_len
) 
```





**Parameters:**


* `buf` Output buffer. 
* `max_buf` Maximum buffer capacity. 
* `msg_type` MQTT-SN message type. 
* `payload_len` Length of payload that follows the header. 



**Returns:**

Header length in bytes (2 or 4), or 0 on buffer overflow. 





        

<hr>



### function mqttsn\_find\_or\_add\_topic 

_Find or allocate topic entry in client table._ 
```C++
static SYN_MQTTSN_TopicEntry * mqttsn_find_or_add_topic (
    SYN_MQTTSN_Client * client,
    const char * topic_name
) 
```





**Parameters:**


* `client` Client instance. 
* `topic_name` Topic string. 



**Returns:**

Pointer to topic entry, or NULL if table is full. 





        

<hr>



### function mqttsn\_process\_rx\_frame 

_Handle incoming decoded MQTT-SN message frame._ 
```C++
static void mqttsn_process_rx_frame (
    SYN_MQTTSN_Client * client,
    uint8_t msg_type,
    const uint8_t * payload,
    size_t payload_len
) 
```





**Parameters:**


* `client` Client instance pointer. 
* `msg_type` Decoded MQTT-SN message type. 
* `payload` Pointer to payload buffer. 
* `payload_len` Length of payload in bytes. 




        

<hr>



### function mqttsn\_publish\_internal 

_Internal helper to publish with configurable topic ID type flag._ 
```C++
static SYN_Status mqttsn_publish_internal (
    SYN_MQTTSN_Client * client,
    uint16_t topic_id,
    uint8_t topic_type_flag,
    int8_t qos,
    bool retain,
    const uint8_t * payload,
    size_t len
) 
```





**Parameters:**


* `client` Client instance pointer. 
* `topic_id` Topic ID or 2-byte short topic name. 
* `topic_type_flag` Topic ID type flag (Normal, Predefined, Short). 
* `qos` QoS level (0, 1, -1). 
* `retain` Retain flag. 
* `payload` Payload data buffer. 
* `len` Payload length in bytes. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_mqttsn.c`

