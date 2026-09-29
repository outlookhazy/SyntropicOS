

# Struct SYN\_LwM2M\_Observation



[**ClassList**](annotated.md) **>** [**SYN\_LwM2M\_Observation**](structSYN__LwM2M__Observation.md)



_Active resource or instance observation (RFC 7641)._ 

* `#include <syn_lwm2m.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  uint16\_t | [**inst\_id**](#variable-inst_id)  <br> |
|  uint32\_t | [**last\_notify\_ms**](#variable-last_notify_ms)  <br> |
|  uint16\_t | [**obj\_id**](#variable-obj_id)  <br> |
|  uint32\_t | [**pmax**](#variable-pmax)  <br> |
|  uint32\_t | [**pmin**](#variable-pmin)  <br> |
|  int32\_t | [**res\_id**](#variable-res_id)  <br> |
|  uint32\_t | [**seq**](#variable-seq)  <br> |
|  uint8\_t | [**token**](#variable-token)  <br> |
|  uint8\_t | [**token\_len**](#variable-token_len)  <br> |












































## Public Attributes Documentation




### variable active 

```C++
bool SYN_LwM2M_Observation::active;
```



Slot is currently active 


        

<hr>



### variable inst\_id 

```C++
uint16_t SYN_LwM2M_Observation::inst_id;
```



Target Instance ID 


        

<hr>



### variable last\_notify\_ms 

```C++
uint32_t SYN_LwM2M_Observation::last_notify_ms;
```



Timestamp of last notification in ms 


        

<hr>



### variable obj\_id 

```C++
uint16_t SYN_LwM2M_Observation::obj_id;
```



Target Object ID 


        

<hr>



### variable pmax 

```C++
uint32_t SYN_LwM2M_Observation::pmax;
```



Maximum observation period in ms 


        

<hr>



### variable pmin 

```C++
uint32_t SYN_LwM2M_Observation::pmin;
```



Minimum observation period in ms 


        

<hr>



### variable res\_id 

```C++
int32_t SYN_LwM2M_Observation::res_id;
```



Target Resource ID (-1 if observing whole instance) 


        

<hr>



### variable seq 

```C++
uint32_t SYN_LwM2M_Observation::seq;
```



Notification sequence counter 


        

<hr>



### variable token 

```C++
uint8_t SYN_LwM2M_Observation::token[8];
```



CoAP observation token 


        

<hr>



### variable token\_len 

```C++
uint8_t SYN_LwM2M_Observation::token_len;
```



CoAP observation token length 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lwm2m.h`

