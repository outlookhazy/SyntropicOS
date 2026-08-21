

# File syn\_mqttsn.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_mqttsn.h**](syn__mqttsn_8h.md)

[Go to the source code of this file](syn__mqttsn_8h_source.md)

_MQTT for Sensor Networks (MQTT-SN v1.2) Protocol Client Engine._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../net/syn_transport.h"`
* `#include "../pt/syn_pt.h"`
* `#include "../sched/syn_sched.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_MQTTSN\_Client**](structSYN__MQTTSN__Client.md) <br>_MQTT-SN Client instance structure._  |
| struct | [**SYN\_MQTTSN\_Config**](structSYN__MQTTSN__Config.md) <br>_MQTT-SN client configuration descriptor._  |
| struct | [**SYN\_MQTTSN\_TopicEntry**](structSYN__MQTTSN__TopicEntry.md) <br>_Registered topic entry mapping string name to 16-bit Topic ID._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_MQTTSN\_MessageCb**](#typedef-syn_mqttsn_messagecb)  <br>_Incoming publish message handler callback._  |
| enum  | [**SYN\_MQTTSN\_State**](#enum-syn_mqttsn_state)  <br>_MQTT-SN client connection state._  |




















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



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_MQTTSN\_DEFAULT\_DURATION\_S**](syn__mqttsn_8h.md#define-syn_mqttsn_default_duration_s)  `60U`<br> |
| define  | [**SYN\_MQTTSN\_FLAG\_CLEAN\_SESSION**](syn__mqttsn_8h.md#define-syn_mqttsn_flag_clean_session)  `0x04U`<br> |
| define  | [**SYN\_MQTTSN\_FLAG\_DUP**](syn__mqttsn_8h.md#define-syn_mqttsn_flag_dup)  `0x80U`<br> |
| define  | [**SYN\_MQTTSN\_FLAG\_QOS\_0**](syn__mqttsn_8h.md#define-syn_mqttsn_flag_qos_0)  `0x00U`<br> |
| define  | [**SYN\_MQTTSN\_FLAG\_QOS\_1**](syn__mqttsn_8h.md#define-syn_mqttsn_flag_qos_1)  `0x20U`<br> |
| define  | [**SYN\_MQTTSN\_FLAG\_QOS\_2**](syn__mqttsn_8h.md#define-syn_mqttsn_flag_qos_2)  `0x40U`<br> |
| define  | [**SYN\_MQTTSN\_FLAG\_QOS\_MASK**](syn__mqttsn_8h.md#define-syn_mqttsn_flag_qos_mask)  `0x60U`<br> |
| define  | [**SYN\_MQTTSN\_FLAG\_QOS\_NEG1**](syn__mqttsn_8h.md#define-syn_mqttsn_flag_qos_neg1)  `0x60U`<br> |
| define  | [**SYN\_MQTTSN\_FLAG\_RETAIN**](syn__mqttsn_8h.md#define-syn_mqttsn_flag_retain)  `0x10U`<br> |
| define  | [**SYN\_MQTTSN\_FLAG\_TOPIC\_MASK**](syn__mqttsn_8h.md#define-syn_mqttsn_flag_topic_mask)  `0x03U`<br> |
| define  | [**SYN\_MQTTSN\_FLAG\_TOPIC\_NORMAL**](syn__mqttsn_8h.md#define-syn_mqttsn_flag_topic_normal)  `0x00U`<br> |
| define  | [**SYN\_MQTTSN\_FLAG\_TOPIC\_PREDEF**](syn__mqttsn_8h.md#define-syn_mqttsn_flag_topic_predef)  `0x01U`<br> |
| define  | [**SYN\_MQTTSN\_FLAG\_TOPIC\_SHORT**](syn__mqttsn_8h.md#define-syn_mqttsn_flag_topic_short)  `0x02U`<br> |
| define  | [**SYN\_MQTTSN\_FLAG\_WILL**](syn__mqttsn_8h.md#define-syn_mqttsn_flag_will)  `0x08U`<br> |
| define  | [**SYN\_MQTTSN\_MAX\_CLIENT\_ID**](syn__mqttsn_8h.md#define-syn_mqttsn_max_client_id)  `24U`<br> |
| define  | [**SYN\_MQTTSN\_MAX\_REGISTRATIONS**](syn__mqttsn_8h.md#define-syn_mqttsn_max_registrations)  `16U`<br> |
| define  | [**SYN\_MQTTSN\_MAX\_TOPIC\_NAME**](syn__mqttsn_8h.md#define-syn_mqttsn_max_topic_name)  `64U`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_ADVERTISE**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_advertise)  `0x00U`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_CONNACK**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_connack)  `0x05U`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_CONNECT**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_connect)  `0x04U`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_DISCONNECT**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_disconnect)  `0x18U`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_GWINFO**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_gwinfo)  `0x02U`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_PINGREQ**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_pingreq)  `0x16U`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_PINGRESP**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_pingresp)  `0x17U`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_PUBACK**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_puback)  `0x0DU`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_PUBCOMP**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_pubcomp)  `0x0EU`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_PUBLISH**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_publish)  `0x0CU`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_PUBREC**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_pubrec)  `0x0FU`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_PUBREL**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_pubrel)  `0x10U`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_REGACK**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_regack)  `0x0BU`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_REGISTER**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_register)  `0x0AU`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_SEARCHGW**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_searchgw)  `0x01U`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_SUBACK**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_suback)  `0x13U`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_SUBSCRIBE**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_subscribe)  `0x12U`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_UNSUBACK**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_unsuback)  `0x15U`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_UNSUBSCRIBE**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_unsubscribe)  `0x14U`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_WILLMSG**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_willmsg)  `0x09U`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_WILLMSGREQ**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_willmsgreq)  `0x08U`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_WILLMSGRESP**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_willmsgresp)  `0x1DU`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_WILLMSGUPD**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_willmsgupd)  `0x1CU`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_WILLTOPIC**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_willtopic)  `0x07U`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_WILLTOPICREQ**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_willtopicreq)  `0x06U`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_WILLTOPICRESP**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_willtopicresp)  `0x1BU`<br> |
| define  | [**SYN\_MQTTSN\_MSG\_WILLTOPICUPD**](syn__mqttsn_8h.md#define-syn_mqttsn_msg_willtopicupd)  `0x1AU`<br> |
| define  | [**SYN\_MQTTSN\_PROTOCOL\_ID**](syn__mqttsn_8h.md#define-syn_mqttsn_protocol_id)  `0x01U`<br> |
| define  | [**SYN\_MQTTSN\_RC\_ACCEPTED**](syn__mqttsn_8h.md#define-syn_mqttsn_rc_accepted)  `0x00U`<br> |
| define  | [**SYN\_MQTTSN\_RC\_REJ\_CONGESTION**](syn__mqttsn_8h.md#define-syn_mqttsn_rc_rej_congestion)  `0x01U`<br> |
| define  | [**SYN\_MQTTSN\_RC\_REJ\_INVALID\_TOPIC\_ID**](syn__mqttsn_8h.md#define-syn_mqttsn_rc_rej_invalid_topic_id)  `0x02U`<br> |
| define  | [**SYN\_MQTTSN\_RC\_REJ\_NOT\_SUPPORTED**](syn__mqttsn_8h.md#define-syn_mqttsn_rc_rej_not_supported)  `0x03U`<br> |

## Detailed Description


Implements a zero-heap, deterministic MQTT-SN v1.2 client for low-power wireless networks:
* Operates over datagram transports (UDP, Zigbee, 802.15.4, Sub-GHz, BLE, RS485).
* Gateway Discovery (`SEARCHGW`, `GWINFO`, `ADVERTISE`).
* Short (2-char), Predefined, and Dynamically Registered Topic IDs (`REGISTER`, `REGACK`).
* Publish and Subscribe (QoS 0, QoS 1, QoS -1) with payload framing.
* Low-power Sleep/Awake mode transitions (`SLEEP`, `AWAKE` queue flush).
* Non-blocking poll step and cooperative protothread continuation for `syn_sched`. 




    
## Public Types Documentation




### typedef SYN\_MQTTSN\_MessageCb 

_Incoming publish message handler callback._ 
```C++
typedef void(* SYN_MQTTSN_MessageCb) (struct SYN_MQTTSN_Client *client, uint16_t topic_id, uint8_t topic_type, const uint8_t *payload, size_t len, void *user_data);
```





**Parameters:**


* `client` Client instance. 
* `topic_id` 16-bit Topic ID or 2-char short topic. 
* `topic_type` Type of topic (normal, predef, short). 
* `payload` Payload data pointer. 
* `len` Payload length in bytes. 
* `user_data` User context pointer. 




        

<hr>



### enum SYN\_MQTTSN\_State 

_MQTT-SN client connection state._ 
```C++
enum SYN_MQTTSN_State {
    SYN_MQTTSN_STATE_DISCONNECTED = 0,
    SYN_MQTTSN_STATE_SEARCHING_GW,
    SYN_MQTTSN_STATE_CONNECTING,
    SYN_MQTTSN_STATE_CONNECTED,
    SYN_MQTTSN_STATE_ASLEEP,
    SYN_MQTTSN_STATE_AWAKE
};
```




<hr>
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
## Macro Definition Documentation





### define SYN\_MQTTSN\_DEFAULT\_DURATION\_S 

```C++
#define SYN_MQTTSN_DEFAULT_DURATION_S `60U`
```



Default Keep-Alive duration in seconds 


        

<hr>



### define SYN\_MQTTSN\_FLAG\_CLEAN\_SESSION 

```C++
#define SYN_MQTTSN_FLAG_CLEAN_SESSION `0x04U`
```



Clean session flag 


        

<hr>



### define SYN\_MQTTSN\_FLAG\_DUP 

```C++
#define SYN_MQTTSN_FLAG_DUP `0x80U`
```



Duplicate delivery flag 


        

<hr>



### define SYN\_MQTTSN\_FLAG\_QOS\_0 

```C++
#define SYN_MQTTSN_FLAG_QOS_0 `0x00U`
```



QoS level 0 


        

<hr>



### define SYN\_MQTTSN\_FLAG\_QOS\_1 

```C++
#define SYN_MQTTSN_FLAG_QOS_1 `0x20U`
```



QoS level 1 


        

<hr>



### define SYN\_MQTTSN\_FLAG\_QOS\_2 

```C++
#define SYN_MQTTSN_FLAG_QOS_2 `0x40U`
```



QoS level 2 


        

<hr>



### define SYN\_MQTTSN\_FLAG\_QOS\_MASK 

```C++
#define SYN_MQTTSN_FLAG_QOS_MASK `0x60U`
```



QoS mask 


        

<hr>



### define SYN\_MQTTSN\_FLAG\_QOS\_NEG1 

```C++
#define SYN_MQTTSN_FLAG_QOS_NEG1 `0x60U`
```



QoS level -1 (transparent) 


        

<hr>



### define SYN\_MQTTSN\_FLAG\_RETAIN 

```C++
#define SYN_MQTTSN_FLAG_RETAIN `0x10U`
```



Retain flag 


        

<hr>



### define SYN\_MQTTSN\_FLAG\_TOPIC\_MASK 

```C++
#define SYN_MQTTSN_FLAG_TOPIC_MASK `0x03U`
```



Topic ID type mask 


        

<hr>



### define SYN\_MQTTSN\_FLAG\_TOPIC\_NORMAL 

```C++
#define SYN_MQTTSN_FLAG_TOPIC_NORMAL `0x00U`
```



Topic ID type: Normal registered ID 


        

<hr>



### define SYN\_MQTTSN\_FLAG\_TOPIC\_PREDEF 

```C++
#define SYN_MQTTSN_FLAG_TOPIC_PREDEF `0x01U`
```



Topic ID type: Predefined 16-bit ID 


        

<hr>



### define SYN\_MQTTSN\_FLAG\_TOPIC\_SHORT 

```C++
#define SYN_MQTTSN_FLAG_TOPIC_SHORT `0x02U`
```



Topic ID type: Short 2-char string 


        

<hr>



### define SYN\_MQTTSN\_FLAG\_WILL 

```C++
#define SYN_MQTTSN_FLAG_WILL `0x08U`
```



Will flag 


        

<hr>



### define SYN\_MQTTSN\_MAX\_CLIENT\_ID 

```C++
#define SYN_MQTTSN_MAX_CLIENT_ID `24U`
```



Max Client Identifier string length 


        

<hr>



### define SYN\_MQTTSN\_MAX\_REGISTRATIONS 

```C++
#define SYN_MQTTSN_MAX_REGISTRATIONS `16U`
```



Max tracked registered topics 


        

<hr>



### define SYN\_MQTTSN\_MAX\_TOPIC\_NAME 

```C++
#define SYN_MQTTSN_MAX_TOPIC_NAME `64U`
```



Max Topic name string length 


        

<hr>



### define SYN\_MQTTSN\_MSG\_ADVERTISE 

```C++
#define SYN_MQTTSN_MSG_ADVERTISE `0x00U`
```



Gateway advertise broadcast 


        

<hr>



### define SYN\_MQTTSN\_MSG\_CONNACK 

```C++
#define SYN_MQTTSN_MSG_CONNACK `0x05U`
```



Connect acknowledgment 


        

<hr>



### define SYN\_MQTTSN\_MSG\_CONNECT 

```C++
#define SYN_MQTTSN_MSG_CONNECT `0x04U`
```



Connect request 


        

<hr>



### define SYN\_MQTTSN\_MSG\_DISCONNECT 

```C++
#define SYN_MQTTSN_MSG_DISCONNECT `0x18U`
```



Disconnect / sleep command 


        

<hr>



### define SYN\_MQTTSN\_MSG\_GWINFO 

```C++
#define SYN_MQTTSN_MSG_GWINFO `0x02U`
```



Gateway information response 


        

<hr>



### define SYN\_MQTTSN\_MSG\_PINGREQ 

```C++
#define SYN_MQTTSN_MSG_PINGREQ `0x16U`
```



Ping request / wake ping 


        

<hr>



### define SYN\_MQTTSN\_MSG\_PINGRESP 

```C++
#define SYN_MQTTSN_MSG_PINGRESP `0x17U`
```



Ping response 


        

<hr>



### define SYN\_MQTTSN\_MSG\_PUBACK 

```C++
#define SYN_MQTTSN_MSG_PUBACK `0x0DU`
```



Publish acknowledgment 


        

<hr>



### define SYN\_MQTTSN\_MSG\_PUBCOMP 

```C++
#define SYN_MQTTSN_MSG_PUBCOMP `0x0EU`
```



Publish complete (QoS 2) 


        

<hr>



### define SYN\_MQTTSN\_MSG\_PUBLISH 

```C++
#define SYN_MQTTSN_MSG_PUBLISH `0x0CU`
```



Publish message 


        

<hr>



### define SYN\_MQTTSN\_MSG\_PUBREC 

```C++
#define SYN_MQTTSN_MSG_PUBREC `0x0FU`
```



Publish received (QoS 2) 


        

<hr>



### define SYN\_MQTTSN\_MSG\_PUBREL 

```C++
#define SYN_MQTTSN_MSG_PUBREL `0x10U`
```



Publish release (QoS 2) 


        

<hr>



### define SYN\_MQTTSN\_MSG\_REGACK 

```C++
#define SYN_MQTTSN_MSG_REGACK `0x0BU`
```



Topic registration acknowledgment 


        

<hr>



### define SYN\_MQTTSN\_MSG\_REGISTER 

```C++
#define SYN_MQTTSN_MSG_REGISTER `0x0AU`
```



Topic registration request 


        

<hr>



### define SYN\_MQTTSN\_MSG\_SEARCHGW 

```C++
#define SYN_MQTTSN_MSG_SEARCHGW `0x01U`
```



Client search gateway broadcast 


        

<hr>



### define SYN\_MQTTSN\_MSG\_SUBACK 

```C++
#define SYN_MQTTSN_MSG_SUBACK `0x13U`
```



Subscribe acknowledgment 


        

<hr>



### define SYN\_MQTTSN\_MSG\_SUBSCRIBE 

```C++
#define SYN_MQTTSN_MSG_SUBSCRIBE `0x12U`
```



Subscribe request 


        

<hr>



### define SYN\_MQTTSN\_MSG\_UNSUBACK 

```C++
#define SYN_MQTTSN_MSG_UNSUBACK `0x15U`
```



Unsubscribe acknowledgment 


        

<hr>



### define SYN\_MQTTSN\_MSG\_UNSUBSCRIBE 

```C++
#define SYN_MQTTSN_MSG_UNSUBSCRIBE `0x14U`
```



Unsubscribe request 


        

<hr>



### define SYN\_MQTTSN\_MSG\_WILLMSG 

```C++
#define SYN_MQTTSN_MSG_WILLMSG `0x09U`
```



Will message payload 


        

<hr>



### define SYN\_MQTTSN\_MSG\_WILLMSGREQ 

```C++
#define SYN_MQTTSN_MSG_WILLMSGREQ `0x08U`
```



Will message request 


        

<hr>



### define SYN\_MQTTSN\_MSG\_WILLMSGRESP 

```C++
#define SYN_MQTTSN_MSG_WILLMSGRESP `0x1DU`
```



Will message update response 


        

<hr>



### define SYN\_MQTTSN\_MSG\_WILLMSGUPD 

```C++
#define SYN_MQTTSN_MSG_WILLMSGUPD `0x1CU`
```



Will message update 


        

<hr>



### define SYN\_MQTTSN\_MSG\_WILLTOPIC 

```C++
#define SYN_MQTTSN_MSG_WILLTOPIC `0x07U`
```



Will topic specification 


        

<hr>



### define SYN\_MQTTSN\_MSG\_WILLTOPICREQ 

```C++
#define SYN_MQTTSN_MSG_WILLTOPICREQ `0x06U`
```



Will topic request 


        

<hr>



### define SYN\_MQTTSN\_MSG\_WILLTOPICRESP 

```C++
#define SYN_MQTTSN_MSG_WILLTOPICRESP `0x1BU`
```



Will topic update response 


        

<hr>



### define SYN\_MQTTSN\_MSG\_WILLTOPICUPD 

```C++
#define SYN_MQTTSN_MSG_WILLTOPICUPD `0x1AU`
```



Will topic update 


        

<hr>



### define SYN\_MQTTSN\_PROTOCOL\_ID 

```C++
#define SYN_MQTTSN_PROTOCOL_ID `0x01U`
```



MQTT-SN Protocol ID v1.2 


        

<hr>



### define SYN\_MQTTSN\_RC\_ACCEPTED 

```C++
#define SYN_MQTTSN_RC_ACCEPTED `0x00U`
```



Request accepted 


        

<hr>



### define SYN\_MQTTSN\_RC\_REJ\_CONGESTION 

```C++
#define SYN_MQTTSN_RC_REJ_CONGESTION `0x01U`
```



Rejected: Congestion 


        

<hr>



### define SYN\_MQTTSN\_RC\_REJ\_INVALID\_TOPIC\_ID 

```C++
#define SYN_MQTTSN_RC_REJ_INVALID_TOPIC_ID `0x02U`
```



Rejected: Invalid Topic ID 


        

<hr>



### define SYN\_MQTTSN\_RC\_REJ\_NOT\_SUPPORTED 

```C++
#define SYN_MQTTSN_RC_REJ_NOT_SUPPORTED `0x03U`
```



Rejected: Not supported 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_mqttsn.h`

