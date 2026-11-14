

# File syn\_task\_profile.c



[**FileList**](files.md) **>** [**debug**](dir_6f573363694cd388330fe66cf20dcff2.md) **>** [**syn\_task\_profile.c**](syn__task__profile_8c.md)

[Go to the source code of this file](syn__task__profile_8c_source.md)

_Opt-In Task Execution, CPU Jitter & Cooperative Chunk-Budget Profiler implementation._ 

* `#include "syn_task_profile.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_task\_profile\_check\_budgets**](#function-syn_task_profile_check_budgets) (const [**SYN\_TaskProfileManager**](structSYN__TaskProfileManager.md) \* mgr, const [**SYN\_Sched**](structSYN__Sched.md) \* sched, size\_t \* out\_violations, [**SYN\_TaskBudgetViolation**](structSYN__TaskBudgetViolation.md) \* worst\_out) <br>_Verify profiled cooperative chunk durations against scheduler peer budgets._  |
|  void | [**syn\_task\_profile\_enable**](#function-syn_task_profile_enable) ([**SYN\_TaskProfileManager**](structSYN__TaskProfileManager.md) \* mgr, [**bool**](syn__defs_8h.md#enum-bool) enable) <br>_Enable or disable the task profiler._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_task\_profile\_get**](#function-syn_task_profile_get) (const [**SYN\_TaskProfileManager**](structSYN__TaskProfileManager.md) \* mgr, size\_t task\_idx, [**SYN\_TaskProfile**](structSYN__TaskProfile.md) \* profile) <br>_Get the profile statistics for a specific task._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_task\_profile\_init**](#function-syn_task_profile_init) ([**SYN\_TaskProfileManager**](structSYN__TaskProfileManager.md) \* mgr) <br>_Initialize the task execution profiler._  |
|  void | [**syn\_task\_profile\_step\_end**](#function-syn_task_profile_step_end) ([**SYN\_TaskProfileManager**](structSYN__TaskProfileManager.md) \* mgr, size\_t task\_idx, uint32\_t now\_us) <br>_Record the completion / yield of a task step execution._  |
|  void | [**syn\_task\_profile\_step\_start**](#function-syn_task_profile_step_start) ([**SYN\_TaskProfileManager**](structSYN__TaskProfileManager.md) \* mgr, size\_t task\_idx, const char \* task\_name, uint32\_t now\_us) <br>_Record the start of a task step execution._  |
|  uint32\_t | [**syn\_task\_profile\_task\_budget\_us**](#function-syn_task_profile_task_budget_us) (const [**SYN\_TaskProfileManager**](structSYN__TaskProfileManager.md) \* mgr, const [**SYN\_Sched**](structSYN__Sched.md) \* sched, size\_t task\_idx, uint8\_t \* out\_peer\_prio) <br>_Compute the cooperative chunk budget (us) for_ `task_idx` _in_`sched` _._ |
|  void | [**syn\_task\_profile\_update**](#function-syn_task_profile_update) ([**SYN\_TaskProfileManager**](structSYN__TaskProfileManager.md) \* mgr, uint32\_t now\_us) <br>_Recalculate CPU utilization percentages across active tasks._  |




























## Public Functions Documentation




### function syn\_task\_profile\_check\_budgets 

_Verify profiled cooperative chunk durations against scheduler peer budgets._ 
```C++
SYN_Status syn_task_profile_check_budgets (
    const SYN_TaskProfileManager * mgr,
    const SYN_Sched * sched,
    size_t * out_violations,
    SYN_TaskBudgetViolation * worst_out
) 
```



Scans all profiled tasks with `step_count > 0` up to `min(mgr->count, sched->task_count)`, counting every task whose `max_yield_us` exceeds `syn_task_profile_task_budget_us()`. Populates `worst_out` (when non-NULL) with the violation having the largest overrun (`max_yield_us - budget_us`).




**Parameters:**


* `mgr` Initialized task profile manager. 
* `sched` Scheduler instance whose tasks were profiled. 
* `out_violations` Output receiving total number of budget-violating tasks. 
* `worst_out` Optional output receiving the worst violation (zeroed if 0). 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM on NULL required arguments. 





        

<hr>



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

SYN\_OK on success, or SYN\_INVALID\_PARAM when `mgr` is NULL. 





        

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



### function syn\_task\_profile\_task\_budget\_us 

_Compute the cooperative chunk budget (us) for_ `task_idx` _in_`sched` _._
```C++
uint32_t syn_task_profile_task_budget_us (
    const SYN_TaskProfileManager * mgr,
    const SYN_Sched * sched,
    size_t task_idx,
    uint8_t * out_peer_prio
) 
```



Returns the smallest `sched->max_wait_ms[prio] * 1000` among all participating peer tasks `j != task_idx` (any non-SUSPENDED peer that is currently alive or executed at least one profiled step in `mgr`). Returns `UINT32_MAX` when uncontended or out of range.




**Parameters:**


* `mgr` Optional profiler manager (used to include completed peers). 
* `sched` Scheduler instance. 
* `task_idx` Task slot index in `sched->tasks`. 
* `out_peer_prio` Optional output receiving the constraining peer priority (0xFF if none). 



**Returns:**

Allowed cooperative chunk duration in microseconds, or UINT32\_MAX if unbounded. 





        

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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/debug/syn_task_profile.c`

