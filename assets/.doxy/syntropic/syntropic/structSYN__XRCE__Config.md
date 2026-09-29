

# Struct SYN\_XRCE\_Config



[**ClassList**](annotated.md) **>** [**SYN\_XRCE\_Config**](structSYN__XRCE__Config.md)



_Micro XRCE-DDS Client configuration._ 

* `#include <syn_xrce_dds.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**client\_key**](#variable-client_key)  <br> |
|  uint32\_t | [**heartbeat\_period\_ms**](#variable-heartbeat_period_ms)  <br> |
|  [**SYN\_XRCE\_DataCb**](syn__xrce__dds_8h.md#typedef-syn_xrce_datacb) | [**on\_data**](#variable-on_data)  <br> |
|  [**SYN\_XRCE\_StatusCb**](syn__xrce__dds_8h.md#typedef-syn_xrce_statuscb) | [**on\_status**](#variable-on_status)  <br> |
|  uint8\_t \* | [**rx\_buf**](#variable-rx_buf)  <br> |
|  size\_t | [**rx\_buf\_size**](#variable-rx_buf_size)  <br> |
|  uint8\_t | [**session\_id**](#variable-session_id)  <br> |
|  [**SYN\_Transport**](structSYN__Transport.md) \* | [**transport**](#variable-transport)  <br> |
|  uint8\_t \* | [**tx\_buf**](#variable-tx_buf)  <br> |
|  size\_t | [**tx\_buf\_size**](#variable-tx_buf_size)  <br> |
|  void \* | [**user\_data**](#variable-user_data)  <br> |












































## Public Attributes Documentation




### variable client\_key 

```C++
uint32_t SYN_XRCE_Config::client_key;
```



32-bit unique client identifier 


        

<hr>



### variable heartbeat\_period\_ms 

```C++
uint32_t SYN_XRCE_Config::heartbeat_period_ms;
```



Keep-alive ping interval in ms 


        

<hr>



### variable on\_data 

```C++
SYN_XRCE_DataCb SYN_XRCE_Config::on_data;
```



Incoming message callback 


        

<hr>



### variable on\_status 

```C++
SYN_XRCE_StatusCb SYN_XRCE_Config::on_status;
```



Command status callback 


        

<hr>



### variable rx\_buf 

```C++
uint8_t* SYN_XRCE_Config::rx_buf;
```



RX scratch buffer 


        

<hr>



### variable rx\_buf\_size 

```C++
size_t SYN_XRCE_Config::rx_buf_size;
```



RX buffer size (&gt;= 256 bytes) 


        

<hr>



### variable session\_id 

```C++
uint8_t SYN_XRCE_Config::session_id;
```



Session ID (default 0x81) 


        

<hr>



### variable transport 

```C++
SYN_Transport* SYN_XRCE_Config::transport;
```



Transport adapter 


        

<hr>



### variable tx\_buf 

```C++
uint8_t* SYN_XRCE_Config::tx_buf;
```



TX scratch buffer 


        

<hr>



### variable tx\_buf\_size 

```C++
size_t SYN_XRCE_Config::tx_buf_size;
```



TX buffer size (&gt;= 256 bytes) 


        

<hr>



### variable user\_data 

```C++
void* SYN_XRCE_Config::user_data;
```



Context pointer for callbacks 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_xrce_dds.h`

