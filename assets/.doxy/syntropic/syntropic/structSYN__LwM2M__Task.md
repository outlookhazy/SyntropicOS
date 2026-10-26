

# Struct SYN\_LwM2M\_Task



[**ClassList**](annotated.md) **>** [**SYN\_LwM2M\_Task**](structSYN__LwM2M__Task.md)



_LwM2M Supervisor Task Context._ 

* `#include <syn_lwm2m_task.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_LwM2M\_TaskConfig**](structSYN__LwM2M__TaskConfig.md) | [**cfg**](#variable-cfg)  <br> |
|  uint16\_t | [**changed\_inst\_id**](#variable-changed_inst_id)  <br> |
|  uint16\_t | [**changed\_obj\_id**](#variable-changed_obj_id)  <br> |
|  int32\_t | [**changed\_res\_id**](#variable-changed_res_id)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**force\_deregister**](#variable-force_deregister)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**force\_update**](#variable-force_update)  <br> |
|  uint32\_t | [**last\_action\_ms**](#variable-last_action_ms)  <br> |
|  uint16\_t | [**next\_msg\_id**](#variable-next_msg_id)  <br> |
|  [**SYN\_PT**](structSYN__PT.md) | [**pt**](#variable-pt)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**resource\_changed**](#variable-resource_changed)  <br> |
|  uint32\_t | [**retry\_count**](#variable-retry_count)  <br> |












































## Public Attributes Documentation




### variable cfg 

```C++
SYN_LwM2M_TaskConfig SYN_LwM2M_Task::cfg;
```



Task configuration 


        

<hr>



### variable changed\_inst\_id 

```C++
uint16_t SYN_LwM2M_Task::changed_inst_id;
```



Changed Instance ID 


        

<hr>



### variable changed\_obj\_id 

```C++
uint16_t SYN_LwM2M_Task::changed_obj_id;
```



Changed Object ID 


        

<hr>



### variable changed\_res\_id 

```C++
int32_t SYN_LwM2M_Task::changed_res_id;
```



Changed Resource ID 


        

<hr>



### variable force\_deregister 

```C++
bool SYN_LwM2M_Task::force_deregister;
```



Flag to request clean Deregistration 


        

<hr>



### variable force\_update 

```C++
bool SYN_LwM2M_Task::force_update;
```



Flag to trigger immediate Registration Update 


        

<hr>



### variable last\_action\_ms 

```C++
uint32_t SYN_LwM2M_Task::last_action_ms;
```



Timestamp of last transmitted request (ms) 


        

<hr>



### variable next\_msg\_id 

```C++
uint16_t SYN_LwM2M_Task::next_msg_id;
```



Message ID sequence counter 


        

<hr>



### variable pt 

```C++
SYN_PT SYN_LwM2M_Task::pt;
```



Protothread state 


        

<hr>



### variable resource\_changed 

```C++
bool SYN_LwM2M_Task::resource_changed;
```



Flag indicating a resource value changed 


        

<hr>



### variable retry\_count 

```C++
uint32_t SYN_LwM2M_Task::retry_count;
```



Number of failed retry attempts 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lwm2m_task.h`

