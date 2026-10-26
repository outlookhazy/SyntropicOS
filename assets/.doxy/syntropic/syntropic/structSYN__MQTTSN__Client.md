

# Struct SYN\_MQTTSN\_Client



[**ClassList**](annotated.md) **>** [**SYN\_MQTTSN\_Client**](structSYN__MQTTSN__Client.md)



_MQTT-SN Client instance structure._ 

* `#include <syn_mqttsn.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_MQTTSN\_Config**](structSYN__MQTTSN__Config.md) | [**cfg**](#variable-cfg)  <br> |
|  uint8\_t | [**gateway\_id**](#variable-gateway_id)  <br> |
|  uint32\_t | [**last\_activity\_ms**](#variable-last_activity_ms)  <br> |
|  uint16\_t | [**next\_msg\_id**](#variable-next_msg_id)  <br> |
|  [**SYN\_PT**](structSYN__PT.md) | [**pt**](#variable-pt)  <br> |
|  [**SYN\_MQTTSN\_State**](syn__mqttsn_8h.md#enum-syn_mqttsn_state) | [**state**](#variable-state)  <br> |
|  size\_t | [**topic\_count**](#variable-topic_count)  <br> |
|  [**SYN\_MQTTSN\_TopicEntry**](structSYN__MQTTSN__TopicEntry.md) | [**topics**](#variable-topics)  <br> |












































## Public Attributes Documentation




### variable cfg 

```C++
SYN_MQTTSN_Config SYN_MQTTSN_Client::cfg;
```



Client configuration 


        

<hr>



### variable gateway\_id 

```C++
uint8_t SYN_MQTTSN_Client::gateway_id;
```



Discovered gateway ID 


        

<hr>



### variable last\_activity\_ms 

```C++
uint32_t SYN_MQTTSN_Client::last_activity_ms;
```



Timestamp of last sent/received frame 


        

<hr>



### variable next\_msg\_id 

```C++
uint16_t SYN_MQTTSN_Client::next_msg_id;
```



Next message sequence ID 


        

<hr>



### variable pt 

```C++
SYN_PT SYN_MQTTSN_Client::pt;
```



Protothread state 


        

<hr>



### variable state 

```C++
SYN_MQTTSN_State SYN_MQTTSN_Client::state;
```



Connection state 


        

<hr>



### variable topic\_count 

```C++
size_t SYN_MQTTSN_Client::topic_count;
```



Number of topics 


        

<hr>



### variable topics 

```C++
SYN_MQTTSN_TopicEntry SYN_MQTTSN_Client::topics[SYN_MQTTSN_MAX_REGISTRATIONS];
```



Topic table 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_mqttsn.h`

