

# Struct SYN\_Task



[**ClassList**](annotated.md) **>** [**SYN\_Task**](structSYN__Task.md)



_Task descriptor — binds a protothread to scheduler metadata._ [More...](#detailed-description)

* `#include <syn_task.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**base\_priority**](#variable-base_priority)  <br> |
|  uint32\_t | [**delay\_until**](#variable-delay_until)  <br> |
|  uint32\_t | [**due\_ms**](#variable-due_ms)  <br> |
|  [**SYN\_TaskFunc**](syn__task_8h.md#typedef-syn_taskfunc) | [**func**](#variable-func)  <br> |
|  struct [**SYN\_Mutex**](structSYN__Mutex.md) \* | [**held\_mutexes**](#variable-held_mutexes)  <br> |
|  uint8\_t | [**manual\_priority**](#variable-manual_priority)  <br> |
|  const char \* | [**name**](#variable-name)  <br> |
|  uint8\_t | [**priority**](#variable-priority)  <br> |
|  [**SYN\_PT**](structSYN__PT.md) | [**pt**](#variable-pt)  <br> |
|  struct [**SYN\_Sched**](structSYN__Sched.md) \* | [**sched**](#variable-sched)  <br> |
|  volatile uint8\_t | [**sched\_flags**](#variable-sched_flags)  <br> |
|  volatile uint8\_t | [**state**](#variable-state)  <br> |
|  void \* | [**user\_data**](#variable-user_data)  <br> |
|  [**SYN\_EventFlags**](structSYN__EventFlags.md) \* | [**wait\_event**](#variable-wait_event)  <br> |
|  uint32\_t | [**wait\_mask**](#variable-wait_mask)  <br> |












































## Detailed Description


Size: 44 bytes on a 32-bit target (80 bytes on 64-bit). 


    
## Public Attributes Documentation




### variable base\_priority 

```C++
uint8_t SYN_Task::base_priority;
```



Configured base priority 


        

<hr>



### variable delay\_until 

```C++
uint32_t SYN_Task::delay_until;
```



Tick deadline for PT\_TASK\_DELAY\_MS 


        

<hr>



### variable due\_ms 

```C++
uint32_t SYN_Task::due_ms;
```



Starvation deadline tick for scheduler 


        

<hr>



### variable func 

```C++
SYN_TaskFunc SYN_Task::func;
```



The task's protothread function 


        

<hr>



### variable held\_mutexes 

```C++
struct SYN_Mutex* SYN_Task::held_mutexes;
```



Intrusive list of owned [**SYN\_Mutex**](structSYN__Mutex.md) locks 


        

<hr>



### variable manual\_priority 

```C++
uint8_t SYN_Task::manual_priority;
```



Manual boost priority (0xFF = none) 


        

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



Active priority (0 = highest) 


        

<hr>



### variable pt 

```C++
SYN_PT SYN_Task::pt;
```



Protothread continuation (2 bytes) 


        

<hr>



### variable sched 

```C++
struct SYN_Sched* SYN_Task::sched;
```



Owning scheduler instance (or NULL) 


        

<hr>



### variable sched\_flags 

```C++
volatile uint8_t SYN_Task::sched_flags;
```



Internal scheduler rank/round flags 


        

<hr>



### variable state 

```C++
volatile uint8_t SYN_Task::state;
```



[**SYN\_TaskState**](syn__task_8h.md#enum-syn_taskstate) 


        

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

