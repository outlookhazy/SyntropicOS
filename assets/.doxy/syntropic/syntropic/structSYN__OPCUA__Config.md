

# Struct SYN\_OPCUA\_Config



[**ClassList**](annotated.md) **>** [**SYN\_OPCUA\_Config**](structSYN__OPCUA__Config.md)



_Configuration parameters for Micro OPC UA Server._ 

* `#include <syn_opcua.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const char \* | [**endpoint\_url**](#variable-endpoint_url)  <br> |
|  uint8\_t \* | [**rx\_buf**](#variable-rx_buf)  <br> |
|  size\_t | [**rx\_buf\_size**](#variable-rx_buf_size)  <br> |
|  const char \* | [**server\_name**](#variable-server_name)  <br> |
|  [**SYN\_Transport**](structSYN__Transport.md) \* | [**transport**](#variable-transport)  <br> |
|  uint8\_t \* | [**tx\_buf**](#variable-tx_buf)  <br> |
|  size\_t | [**tx\_buf\_size**](#variable-tx_buf_size)  <br> |
|  void \* | [**user\_data**](#variable-user_data)  <br> |












































## Public Attributes Documentation




### variable endpoint\_url 

```C++
const char* SYN_OPCUA_Config::endpoint_url;
```



Server endpoint URL (e.g. "opc.tcp://0.0.0.0:4840") 


        

<hr>



### variable rx\_buf 

```C++
uint8_t* SYN_OPCUA_Config::rx_buf;
```



Caller-allocated RX scratch buffer 


        

<hr>



### variable rx\_buf\_size 

```C++
size_t SYN_OPCUA_Config::rx_buf_size;
```



RX buffer size (&gt;= 512 bytes) 


        

<hr>



### variable server\_name 

```C++
const char* SYN_OPCUA_Config::server_name;
```



Human-readable server name 


        

<hr>



### variable transport 

```C++
SYN_Transport* SYN_OPCUA_Config::transport;
```



Underlying transport (TCP stream) 


        

<hr>



### variable tx\_buf 

```C++
uint8_t* SYN_OPCUA_Config::tx_buf;
```



Caller-allocated TX scratch buffer 


        

<hr>



### variable tx\_buf\_size 

```C++
size_t SYN_OPCUA_Config::tx_buf_size;
```



TX buffer size (&gt;= 512 bytes) 


        

<hr>



### variable user\_data 

```C++
void* SYN_OPCUA_Config::user_data;
```



User context pointer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_opcua.h`

