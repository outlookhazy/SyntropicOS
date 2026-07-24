

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
| struct | [**SYN\_MqttClient**](structSYN__MqttClient.md) <br>_MQTT client context structure._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_MqttRxPhase**](#enum-syn_mqttrxphase)  <br>_Non-blocking packet reception states._  |
| enum  | [**SYN\_MqttState**](#enum-syn_mqttstate)  <br>_MQTT client connection states._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqtt\_init**](#function-syn_mqtt_init) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* client, const char \* host, uint16\_t port, const char \* client\_id, const char \* username, const char \* password, uint16\_t keep\_alive\_s, uint8\_t \* rx\_buf, size\_t rx\_buf\_size, uint8\_t \* tx\_buf, size\_t tx\_buf\_size) <br>_Initialize the MQTT client._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqtt\_publish**](#function-syn_mqtt_publish) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* client, const char \* topic, const void \* payload, size\_t len, uint8\_t qos, [**bool**](syn__defs_8h.md#enum-bool) retain) <br>_Publish a message to a topic._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mqtt\_subscribe**](#function-syn_mqtt_subscribe) ([**SYN\_MqttClient**](structSYN__MqttClient.md) \* client, const char \* topic, uint8\_t qos) <br>_Subscribe to a topic._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_mqtt\_task**](#function-syn_mqtt_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative task for driving the MQTT client._  |




























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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_mqtt.h`

