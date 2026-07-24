

# File syn\_task\_profile.h



[**FileList**](files.md) **>** [**debug**](dir_6f573363694cd388330fe66cf20dcff2.md) **>** [**syn\_task\_profile.h**](syn__task__profile_8h.md)

[Go to the source code of this file](syn__task__profile_8h_source.md)

_Opt-In Task Execution & CPU Jitter Profiler._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../sched/syn_sched.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_TaskProfile**](structSYN__TaskProfile.md) <br>_Task Execution Profile Metrics Structure._  |
| struct | [**SYN\_TaskProfileManager**](structSYN__TaskProfileManager.md) <br>_Task Profiler Registry / Manager Instance._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_task\_profile\_enable**](#function-syn_task_profile_enable) ([**SYN\_TaskProfileManager**](structSYN__TaskProfileManager.md) \* mgr, [**bool**](syn__defs_8h.md#enum-bool) enable) <br>_Enable or disable the task profiler._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_task\_profile\_get**](#function-syn_task_profile_get) (const [**SYN\_TaskProfileManager**](structSYN__TaskProfileManager.md) \* mgr, size\_t task\_idx, [**SYN\_TaskProfile**](structSYN__TaskProfile.md) \* profile) <br>_Get the profile statistics for a specific task._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_task\_profile\_init**](#function-syn_task_profile_init) ([**SYN\_TaskProfileManager**](structSYN__TaskProfileManager.md) \* mgr) <br>_Initialize the task execution profiler._  |
|  void | [**syn\_task\_profile\_step\_end**](#function-syn_task_profile_step_end) ([**SYN\_TaskProfileManager**](structSYN__TaskProfileManager.md) \* mgr, size\_t task\_idx, uint32\_t now\_us) <br>_Record the completion / yield of a task step execution._  |
|  void | [**syn\_task\_profile\_step\_start**](#function-syn_task_profile_step_start) ([**SYN\_TaskProfileManager**](structSYN__TaskProfileManager.md) \* mgr, size\_t task\_idx, const char \* task\_name, uint32\_t now\_us) <br>_Record the start of a task step execution._  |
|  void | [**syn\_task\_profile\_update**](#function-syn_task_profile_update) ([**SYN\_TaskProfileManager**](structSYN__TaskProfileManager.md) \* mgr, uint32\_t now\_us) <br>_Recalculate CPU utilization percentages across active tasks._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_TASK\_PROFILE\_MAX\_TASKS**](syn__task__profile_8h.md#define-syn_task_profile_max_tasks)  `16`<br> |

## Detailed Description


Provides opt-in execution duration, maximum yield latency, and CPU % utilization tracking for cooperative protothread tasks. 


    
## Public Functions Documentation




### function syn\_task\_profile\_enable 

_Enable or disable the task profiler._ 
```C++
void syn_task_profile_enable (
    SYN_TaskProfileManager * mgr,
    bool enable
) 
```





**Parameters:**


* `mgr` Pointer to profile manager. 
* `enable` true to enable, false to disable. 




        

<hr>



### function syn\_task\_profile\_get 

_Get the profile statistics for a specific task._ 
```C++
SYN_Status syn_task_profile_get (
    const SYN_TaskProfileManager * mgr,
    size_t task_idx,
    SYN_TaskProfile * profile
) 
```





**Parameters:**


* `mgr` Pointer to profile manager. 
* `task_idx` Task index. 
* `profile` Output profile metrics pointer. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM. 





        

<hr>



### function syn\_task\_profile\_init 

_Initialize the task execution profiler._ 
```C++
SYN_Status syn_task_profile_init (
    SYN_TaskProfileManager * mgr
) 
```





**Parameters:**


* `mgr` Pointer to task profile manager. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_task\_profile\_step\_end 

_Record the completion / yield of a task step execution._ 
```C++
void syn_task_profile_step_end (
    SYN_TaskProfileManager * mgr,
    size_t task_idx,
    uint32_t now_us
) 
```





**Parameters:**


* `mgr` Pointer to profile manager. 
* `task_idx` Index of executing task. 
* `now_us` Current microsecond timestamp. 




        

<hr>



### function syn\_task\_profile\_step\_start 

_Record the start of a task step execution._ 
```C++
void syn_task_profile_step_start (
    SYN_TaskProfileManager * mgr,
    size_t task_idx,
    const char * task_name,
    uint32_t now_us
) 
```





**Parameters:**


* `mgr` Pointer to profile manager. 
* `task_idx` Index of executing task. 
* `task_name` Task identifier name. 
* `now_us` Current microsecond timestamp. 




        

<hr>



### function syn\_task\_profile\_update 

_Recalculate CPU utilization percentages across active tasks._ 
```C++
void syn_task_profile_update (
    SYN_TaskProfileManager * mgr,
    uint32_t now_us
) 
```





**Parameters:**


* `mgr` Pointer to profile manager. 
* `now_us` Current microsecond timestamp. 




        

<hr>
## Macro Definition Documentation





### define SYN\_TASK\_PROFILE\_MAX\_TASKS 

```C++
#define SYN_TASK_PROFILE_MAX_TASKS `16`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/debug/syn_task_profile.h`

