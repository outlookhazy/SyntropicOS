

# Struct SYN\_LwM2M\_Client



[**ClassList**](annotated.md) **>** [**SYN\_LwM2M\_Client**](structSYN__LwM2M__Client.md)



_LwM2M Client Context._ 

* `#include <syn_lwm2m.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const char \* | [**binding**](#variable-binding)  <br> |
|  const char \* | [**endpoint\_name**](#variable-endpoint_name)  <br> |
|  uint32\_t | [**last\_update\_ms**](#variable-last_update_ms)  <br> |
|  uint32\_t | [**lifetime\_s**](#variable-lifetime_s)  <br> |
|  char | [**location\_path**](#variable-location_path)  <br> |
|  size\_t | [**object\_count**](#variable-object_count)  <br> |
|  const [**SYN\_LwM2M\_Object**](structSYN__LwM2M__Object.md) \* | [**objects**](#variable-objects)  <br> |
|  [**SYN\_LwM2M\_Observation**](structSYN__LwM2M__Observation.md) | [**observations**](#variable-observations)  <br> |
|  [**SYN\_LwM2M\_ClientState**](syn__lwm2m_8h.md#enum-syn_lwm2m_clientstate) | [**state**](#variable-state)  <br> |
|  [**SYN\_Transport**](structSYN__Transport.md) \* | [**transport**](#variable-transport)  <br> |












































## Public Attributes Documentation




### variable binding 

```C++
const char* SYN_LwM2M_Client::binding;
```



Transport binding ("U" for UDP) 


        

<hr>



### variable endpoint\_name 

```C++
const char* SYN_LwM2M_Client::endpoint_name;
```



Client endpoint name (e.g. "urn:imei:12345") 


        

<hr>



### variable last\_update\_ms 

```C++
uint32_t SYN_LwM2M_Client::last_update_ms;
```



Timestamp of last registration/update in ms 


        

<hr>



### variable lifetime\_s 

```C++
uint32_t SYN_LwM2M_Client::lifetime_s;
```



Registration lifetime in seconds 


        

<hr>



### variable location\_path 

```C++
char SYN_LwM2M_Client::location_path[SYN_LWM2M_MAX_URI_PATH_LEN];
```



Server-assigned location path (e.g. "rd/42") 


        

<hr>



### variable object\_count 

```C++
size_t SYN_LwM2M_Client::object_count;
```



Count of registered objects 


        

<hr>



### variable objects 

```C++
const SYN_LwM2M_Object* SYN_LwM2M_Client::objects[SYN_LWM2M_MAX_OBJECTS];
```



Registered objects 


        

<hr>



### variable observations 

```C++
SYN_LwM2M_Observation SYN_LwM2M_Client::observations[SYN_LWM2M_MAX_OBSERVERS];
```



Observation slots 


        

<hr>



### variable state 

```C++
SYN_LwM2M_ClientState SYN_LwM2M_Client::state;
```



Registration state 


        

<hr>



### variable transport 

```C++
SYN_Transport* SYN_LwM2M_Client::transport;
```



Underlying generic or DTLS transport 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lwm2m.h`

