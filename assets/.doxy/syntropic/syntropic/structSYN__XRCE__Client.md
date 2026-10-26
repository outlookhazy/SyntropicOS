

# Struct SYN\_XRCE\_Client



[**ClassList**](annotated.md) **>** [**SYN\_XRCE\_Client**](structSYN__XRCE__Client.md)



_Micro XRCE-DDS Client instance context._ 

* `#include <syn_xrce_dds.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_XRCE\_Config**](structSYN__XRCE__Config.md) | [**cfg**](#variable-cfg)  <br> |
|  uint32\_t | [**last\_heartbeat\_ms**](#variable-last_heartbeat_ms)  <br> |
|  uint16\_t | [**next\_req\_id**](#variable-next_req_id)  <br> |
|  size\_t | [**object\_count**](#variable-object_count)  <br> |
|  [**SYN\_XRCE\_Object**](structSYN__XRCE__Object.md) | [**objects**](#variable-objects)  <br> |
|  [**SYN\_PT**](structSYN__PT.md) | [**pt**](#variable-pt)  <br> |
|  [**SYN\_XRCE\_ClientState**](syn__xrce__dds_8h.md#enum-syn_xrce_clientstate) | [**state**](#variable-state)  <br> |
|  uint16\_t | [**stream\_seq\_num**](#variable-stream_seq_num)  <br> |












































## Public Attributes Documentation




### variable cfg 

```C++
SYN_XRCE_Config SYN_XRCE_Client::cfg;
```



Task and connection configuration 


        

<hr>



### variable last\_heartbeat\_ms 

```C++
uint32_t SYN_XRCE_Client::last_heartbeat_ms;
```



Timestamp of last transmitted heartbeat 


        

<hr>



### variable next\_req\_id 

```C++
uint16_t SYN_XRCE_Client::next_req_id;
```



Request ID counter 


        

<hr>



### variable object\_count 

```C++
size_t SYN_XRCE_Client::object_count;
```



Count of configured entities 


        

<hr>



### variable objects 

```C++
SYN_XRCE_Object SYN_XRCE_Client::objects[SYN_XRCE_MAX_OBJECTS];
```



Tracked local entity table 


        

<hr>



### variable pt 

```C++
SYN_PT SYN_XRCE_Client::pt;
```



Protothread state 


        

<hr>



### variable state 

```C++
SYN_XRCE_ClientState SYN_XRCE_Client::state;
```



Active connection state 


        

<hr>



### variable stream\_seq\_num 

```C++
uint16_t SYN_XRCE_Client::stream_seq_num;
```



Outgoing sequence number 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_xrce_dds.h`

