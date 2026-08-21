

# Struct SYN\_MQTTSN\_Config



[**ClassList**](annotated.md) **>** [**SYN\_MQTTSN\_Config**](structSYN__MQTTSN__Config.md)



_MQTT-SN client configuration descriptor._ 

* `#include <syn_mqttsn.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**clean\_session**](#variable-clean_session)  <br> |
|  const char \* | [**client\_id**](#variable-client_id)  <br> |
|  uint16\_t | [**duration\_s**](#variable-duration_s)  <br> |
|  [**SYN\_MQTTSN\_MessageCb**](syn__mqttsn_8h.md#typedef-syn_mqttsn_messagecb) | [**on\_message**](#variable-on_message)  <br> |
|  uint8\_t \* | [**rx\_buf**](#variable-rx_buf)  <br> |
|  size\_t | [**rx\_buf\_size**](#variable-rx_buf_size)  <br> |
|  [**SYN\_Transport**](structSYN__Transport.md) \* | [**transport**](#variable-transport)  <br> |
|  uint8\_t \* | [**tx\_buf**](#variable-tx_buf)  <br> |
|  size\_t | [**tx\_buf\_size**](#variable-tx_buf_size)  <br> |
|  void \* | [**user\_data**](#variable-user_data)  <br> |












































## Public Attributes Documentation




### variable clean\_session 

```C++
bool SYN_MQTTSN_Config::clean_session;
```



Clean session flag 


        

<hr>



### variable client\_id 

```C++
const char* SYN_MQTTSN_Config::client_id;
```



Client identifier string (&lt;= 23 chars) 


        

<hr>



### variable duration\_s 

```C++
uint16_t SYN_MQTTSN_Config::duration_s;
```



Keep-alive duration in seconds 


        

<hr>



### variable on\_message 

```C++
SYN_MQTTSN_MessageCb SYN_MQTTSN_Config::on_message;
```



Incoming message callback 


        

<hr>



### variable rx\_buf 

```C++
uint8_t* SYN_MQTTSN_Config::rx_buf;
```



Scratch RX buffer 


        

<hr>



### variable rx\_buf\_size 

```C++
size_t SYN_MQTTSN_Config::rx_buf_size;
```



Scratch RX buffer size (&gt;= 128 bytes) 


        

<hr>



### variable transport 

```C++
SYN_Transport* SYN_MQTTSN_Config::transport;
```



Underlying datagram transport 


        

<hr>



### variable tx\_buf 

```C++
uint8_t* SYN_MQTTSN_Config::tx_buf;
```



Scratch TX buffer 


        

<hr>



### variable tx\_buf\_size 

```C++
size_t SYN_MQTTSN_Config::tx_buf_size;
```



Scratch TX buffer size (&gt;= 128 bytes) 


        

<hr>



### variable user\_data 

```C++
void* SYN_MQTTSN_Config::user_data;
```



User context pointer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_mqttsn.h`

