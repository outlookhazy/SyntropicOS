

# Struct SYN\_LwM2M\_TaskConfig



[**ClassList**](annotated.md) **>** [**SYN\_LwM2M\_TaskConfig**](structSYN__LwM2M__TaskConfig.md)



_Configuration parameters for LwM2M Client Supervisor Task._ 

* `#include <syn_lwm2m_task.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_LwM2M\_Client**](structSYN__LwM2M__Client.md) \* | [**client**](#variable-client)  <br> |
|  [**SYN\_CoapsClient**](structSYN__CoapsClient.md) \* | [**coaps\_client**](#variable-coaps_client)  <br> |
|  [**SYN\_LwM2M\_FwUpdateCb**](syn__lwm2m__task_8h.md#typedef-syn_lwm2m_fwupdatecb) | [**on\_fw\_update**](#variable-on_fw_update)  <br> |
|  [**SYN\_LwM2M\_RebootCb**](syn__lwm2m__task_8h.md#typedef-syn_lwm2m_rebootcb) | [**on\_reboot**](#variable-on_reboot)  <br> |
|  [**SYN\_LwM2M\_FactoryResetCb**](syn__lwm2m__task_8h.md#typedef-syn_lwm2m_factoryresetcb) | [**on\_reset**](#variable-on_reset)  <br> |
|  [**SYN\_LwM2M\_StateChangeCb**](syn__lwm2m__task_8h.md#typedef-syn_lwm2m_statechangecb) | [**on\_state**](#variable-on_state)  <br> |
|  uint32\_t | [**retry\_backoff\_ms**](#variable-retry_backoff_ms)  <br> |
|  uint8\_t \* | [**rx\_buf**](#variable-rx_buf)  <br> |
|  size\_t | [**rx\_buf\_size**](#variable-rx_buf_size)  <br> |
|  [**SYN\_Transport**](structSYN__Transport.md) \* | [**transport**](#variable-transport)  <br> |
|  uint8\_t \* | [**tx\_buf**](#variable-tx_buf)  <br> |
|  size\_t | [**tx\_buf\_size**](#variable-tx_buf_size)  <br> |
|  void \* | [**user\_data**](#variable-user_data)  <br> |












































## Public Attributes Documentation




### variable client 

```C++
SYN_LwM2M_Client* SYN_LwM2M_TaskConfig::client;
```



Initialized LwM2M client instance 


        

<hr>



### variable coaps\_client 

```C++
SYN_CoapsClient* SYN_LwM2M_TaskConfig::coaps_client;
```



Optional DTLS 1.3 client context (or NULL) 


        

<hr>



### variable on\_fw\_update 

```C++
SYN_LwM2M_FwUpdateCb SYN_LwM2M_TaskConfig::on_fw_update;
```



Firmware update handler 


        

<hr>



### variable on\_reboot 

```C++
SYN_LwM2M_RebootCb SYN_LwM2M_TaskConfig::on_reboot;
```



Device reboot handler 


        

<hr>



### variable on\_reset 

```C++
SYN_LwM2M_FactoryResetCb SYN_LwM2M_TaskConfig::on_reset;
```



Factory reset handler 


        

<hr>



### variable on\_state 

```C++
SYN_LwM2M_StateChangeCb SYN_LwM2M_TaskConfig::on_state;
```



State change notification callback 


        

<hr>



### variable retry\_backoff\_ms 

```C++
uint32_t SYN_LwM2M_TaskConfig::retry_backoff_ms;
```



Retry backoff period on network failure (ms) 


        

<hr>



### variable rx\_buf 

```C++
uint8_t* SYN_LwM2M_TaskConfig::rx_buf;
```



Caller-allocated RX frame scratch buffer 


        

<hr>



### variable rx\_buf\_size 

```C++
size_t SYN_LwM2M_TaskConfig::rx_buf_size;
```



Size of RX frame buffer (recommended &gt;= 512) 


        

<hr>



### variable transport 

```C++
SYN_Transport* SYN_LwM2M_TaskConfig::transport;
```



Underlying datagram transport (UDP/custom) 


        

<hr>



### variable tx\_buf 

```C++
uint8_t* SYN_LwM2M_TaskConfig::tx_buf;
```



Caller-allocated TX frame scratch buffer 


        

<hr>



### variable tx\_buf\_size 

```C++
size_t SYN_LwM2M_TaskConfig::tx_buf_size;
```



Size of TX frame buffer (recommended &gt;= 512) 


        

<hr>



### variable user\_data 

```C++
void* SYN_LwM2M_TaskConfig::user_data;
```



User data passed to callbacks 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lwm2m_task.h`

