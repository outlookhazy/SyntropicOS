

# Struct SYN\_Task



[**ClassList**](annotated.md) **>** [**SYN\_Task**](structSYN__Task.md)



_Task descriptor — binds a protothread to scheduler metadata._ [More...](#detailed-description)

* `#include <syn_task.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**delay\_until**](#variable-delay_until)  <br> |
|  [**SYN\_TaskFunc**](syn__task_8h.md#typedef-syn_taskfunc) | [**func**](#variable-func)  <br> |
|  const char \* | [**name**](#variable-name)  <br> |
|  uint8\_t | [**priority**](#variable-priority)  <br> |
|  [**SYN\_PT**](structSYN__PT.md) | [**pt**](#variable-pt)  <br> |
|  uint8\_t | [**state**](#variable-state)  <br> |
|  void \* | [**user\_data**](#variable-user_data)  <br> |
|  [**SYN\_EventFlags**](structSYN__EventFlags.md) \* | [**wait\_event**](#variable-wait_event)  <br> |
|  uint32\_t | [**wait\_mask**](#variable-wait_mask)  <br> |












































## Detailed Description


Typical size: ~28 bytes on a 32-bit target. 


    
## Public Attributes Documentation




### variable delay\_until 

```C++
uint32_t SYN_Task::delay_until;
```



Tick deadline for PT\_TASK\_DELAY\_MS 
 


        

<hr>



### variable func 

```C++
SYN_TaskFunc SYN_Task::func;
```



The task's protothread function 
 


        

<hr>



### variable name 

```C++
const char* SYN_Task::name;
```



Human-readable name (for debug/logging) 
 


        

<hr>



### variable priority 

```C++
uint8_t SYN_Task::priority;
```



0 = highest priority 
 


        

<hr>



### variable pt 

```C++
SYN_PT SYN_Task::pt;
```



Protothread continuation (2 bytes) 
 


        

<hr>



### variable state 

```C++
uint8_t SYN_Task::state;
```



SYN\_TaskState 
 


        

<hr>



### variable user\_data 

```C++
void* SYN_Task::user_data;
```



Optional pointer to task-private state 
 


        

<hr>



### variable wait\_event 

```C++
SYN_EventFlags* SYN_Task::wait_event;
```



Event flags task blocks on (NULL if not blocking) 


        

<hr>



### variable wait\_mask 

```C++
uint32_t SYN_Task::wait_mask;
```



Bitmask of event flags to wait for 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_task.h`

