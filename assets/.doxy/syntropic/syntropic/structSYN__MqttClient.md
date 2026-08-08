

# Struct SYN\_MqttClient



[**ClassList**](annotated.md) **>** [**SYN\_MqttClient**](structSYN__MqttClient.md)



_MQTT client context structure._ 

* `#include <syn_mqtt.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const char \* | [**client\_id**](#variable-client_id)  <br> |
|  void \* | [**ctx**](#variable-ctx)  <br> |
|  const char \* | [**host**](#variable-host)  <br> |
|  uint16\_t | [**keep\_alive\_s**](#variable-keep_alive_s)  <br> |
|  uint32\_t | [**last\_activity\_ms**](#variable-last_activity_ms)  <br> |
|  uint16\_t | [**next\_packet\_id**](#variable-next_packet_id)  <br> |
|  void(\* | [**on\_message**](#variable-on_message)  <br>_User callback for incoming publications._  |
|  const char \* | [**password**](#variable-password)  <br> |
|  uint16\_t | [**pending\_puback\_id**](#variable-pending_puback_id)  <br> |
|  uint32\_t | [**pending\_puback\_ms**](#variable-pending_puback_ms)  <br> |
|  uint16\_t | [**port**](#variable-port)  <br> |
|  uint8\_t | [**retransmit\_buf**](#variable-retransmit_buf)  <br> |
|  size\_t | [**retransmit\_len**](#variable-retransmit_len)  <br> |
|  uint8\_t \* | [**rx\_buf**](#variable-rx_buf)  <br> |
|  size\_t | [**rx\_buf\_size**](#variable-rx_buf_size)  <br> |
|  uint32\_t | [**rx\_deadline**](#variable-rx_deadline)  <br> |
|  uint8\_t | [**rx\_header**](#variable-rx_header)  <br> |
|  uint32\_t | [**rx\_mult**](#variable-rx_mult)  <br> |
|  [**SYN\_MqttRxPhase**](syn__mqtt_8h.md#enum-syn_mqttrxphase) | [**rx\_phase**](#variable-rx_phase)  <br> |
|  size\_t | [**rx\_pos**](#variable-rx_pos)  <br> |
|  uint32\_t | [**rx\_rem\_len**](#variable-rx_rem_len)  <br> |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**sock**](#variable-sock)  <br> |
|  [**SYN\_MqttState**](syn__mqtt_8h.md#enum-syn_mqttstate) | [**state**](#variable-state)  <br> |
|  uint8\_t \* | [**tx\_buf**](#variable-tx_buf)  <br> |
|  size\_t | [**tx\_buf\_size**](#variable-tx_buf_size)  <br> |
|  const char \* | [**username**](#variable-username)  <br> |












































## Public Attributes Documentation




### variable client\_id 

```C++
const char* SYN_MqttClient::client_id;
```



Client identifier string (must be unique) 


        

<hr>



### variable ctx 

```C++
void* SYN_MqttClient::ctx;
```



Context pointer for on\_message callback 


        

<hr>



### variable host 

```C++
const char* SYN_MqttClient::host;
```



Hostname or IP of the broker 


        

<hr>



### variable keep\_alive\_s 

```C++
uint16_t SYN_MqttClient::keep_alive_s;
```



Keep alive ping interval in seconds 


        

<hr>



### variable last\_activity\_ms 

```C++
uint32_t SYN_MqttClient::last_activity_ms;
```



Timestamp of last transmitted or received packet 


        

<hr>



### variable next\_packet\_id 

```C++
uint16_t SYN_MqttClient::next_packet_id;
```



Sequence counter for packet identifiers 


        

<hr>



### variable on\_message 

_User callback for incoming publications._ 
```C++
void(* SYN_MqttClient::on_message) (const char *topic, const uint8_t *payload, size_t len, void *ctx);
```





**Parameters:**


* `topic` Topic name string. 
* `payload` Received data payload buffer. 
* `len` Size of payload in bytes. 
* `ctx` User-defined context pointer. 




        

<hr>



### variable password 

```C++
const char* SYN_MqttClient::password;
```



Optional login password, or NULL 


        

<hr>



### variable pending\_puback\_id 

```C++
uint16_t SYN_MqttClient::pending_puback_id;
```



Awaiting QoS 1 puback confirmation packet ID 


        

<hr>



### variable pending\_puback\_ms 

```C++
uint32_t SYN_MqttClient::pending_puback_ms;
```



Timeout timer for pending puback confirmation 


        

<hr>



### variable port 

```C++
uint16_t SYN_MqttClient::port;
```



Broker Port (typically 1883) 


        

<hr>



### variable retransmit\_buf 

```C++
uint8_t SYN_MqttClient::retransmit_buf[128];
```



Buffer for storing unacknowledged QoS 1 packet 


        

<hr>



### variable retransmit\_len 

```C++
size_t SYN_MqttClient::retransmit_len;
```



Length of packet in retransmit\_buf 


        

<hr>



### variable rx\_buf 

```C++
uint8_t* SYN_MqttClient::rx_buf;
```



Receive packet formatting buffer 


        

<hr>



### variable rx\_buf\_size 

```C++
size_t SYN_MqttClient::rx_buf_size;
```



Capacity of rx\_buf 


        

<hr>



### variable rx\_deadline 

```C++
uint32_t SYN_MqttClient::rx_deadline;
```



Tick deadline for incomplete packet RX 


        

<hr>



### variable rx\_header 

```C++
uint8_t SYN_MqttClient::rx_header;
```



Opcode byte of current packet 


        

<hr>



### variable rx\_mult 

```C++
uint32_t SYN_MqttClient::rx_mult;
```



Multiplier for decoding varint remaining len 


        

<hr>



### variable rx\_phase 

```C++
SYN_MqttRxPhase SYN_MqttClient::rx_phase;
```



Current RX state machine phase 


        

<hr>



### variable rx\_pos 

```C++
size_t SYN_MqttClient::rx_pos;
```



Bytes read into rx\_buf or discarded so far 


        

<hr>



### variable rx\_rem\_len 

```C++
uint32_t SYN_MqttClient::rx_rem_len;
```



Total remaining length of current packet 


        

<hr>



### variable sock 

```C++
SYN_Socket SYN_MqttClient::sock;
```



TCP socket connection to broker 


        

<hr>



### variable state 

```C++
SYN_MqttState SYN_MqttClient::state;
```



Connection state machine status 


        

<hr>



### variable tx\_buf 

```C++
uint8_t* SYN_MqttClient::tx_buf;
```



Transmit packet formatting buffer 


        

<hr>



### variable tx\_buf\_size 

```C++
size_t SYN_MqttClient::tx_buf_size;
```



Capacity of tx\_buf 


        

<hr>



### variable username 

```C++
const char* SYN_MqttClient::username;
```



Optional login username, or NULL 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_mqtt.h`

