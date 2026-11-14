

# File syn\_sched.c



[**FileList**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_sched.c**](syn__sched_8c.md)

[Go to the source code of this file](syn__sched_8c_source.md)

_Cooperative scheduler implementation._ 

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_system.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_event.h"`
* `#include "../util/syn_metrics.h"`
* `#include "syn_sched.h"`
* `#include "../debug/syn_task_profile.h"`
* `#include "syn_mutex.h"`
* `#include <limits.h>`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_sched\_alive\_count**](#function-syn_sched_alive_count) (const [**SYN\_Sched**](structSYN__Sched.md) \* sched) <br>_Get the number of tasks that are still alive in the scheduler._  |
|  void | [**syn\_sched\_init**](#function-syn_sched_init) ([**SYN\_Sched**](structSYN__Sched.md) \* sched, [**SYN\_Task**](structSYN__Task.md) \* tasks, size\_t count) <br>_Initialize the scheduler with a task array._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_sched\_run**](#function-syn_sched_run) ([**SYN\_Sched**](structSYN__Sched.md) \* sched) <br>_Run one scheduler dispatch._  |
|  [**SYN\_NORETURN**](syn__compiler_8h.md#define-syn_noreturn) void | [**syn\_sched\_run\_forever**](#function-syn_sched_run_forever) ([**SYN\_Sched**](structSYN__Sched.md) \* sched) <br>_Run the scheduler forever._  |
|  void | [**syn\_sched\_set\_max\_wait**](#function-syn_sched_set_max_wait) ([**SYN\_Sched**](structSYN__Sched.md) \* sched, [**SYN\_Prio**](syn__task_8h.md#enum-syn_prio) prio, uint16\_t ms) <br>_Configure the maximum starvation wait window for a priority level._  |
|  void | [**syn\_sched\_set\_profiler**](#function-syn_sched_set_profiler) ([**SYN\_Sched**](structSYN__Sched.md) \* sched, struct [**SYN\_TaskProfileManager**](structSYN__TaskProfileManager.md) \* profiler) <br>_Attach or detach a task execution profiler on_ `sched` _._ |
|  void | [**syn\_task\_boost\_priority**](#function-syn_task_boost_priority) ([**SYN\_Task**](structSYN__Task.md) \* task, [**SYN\_Prio**](syn__task_8h.md#enum-syn_prio) temp\_prio) <br>_Temporarily boost a task's execution priority._  |
|  void | [**syn\_task\_create**](#function-syn_task_create) ([**SYN\_Task**](structSYN__Task.md) \* task, const char \* name, [**SYN\_TaskFunc**](syn__task_8h.md#typedef-syn_taskfunc) func, [**SYN\_Prio**](syn__task_8h.md#enum-syn_prio) priority, void \* user\_data) <br>_Initialize a single task descriptor._  |
|  void | [**syn\_task\_restart**](#function-syn_task_restart) ([**SYN\_Task**](structSYN__Task.md) \* task) <br>_Restart a task from the beginning of its protothread._  |
|  void | [**syn\_task\_restore\_priority**](#function-syn_task_restore_priority) ([**SYN\_Task**](structSYN__Task.md) \* task) <br>_Restore a task's priority back to its configured base priority (preserving any active mutex priority-inheritance boost)._  |
|  void | [**syn\_task\_resume**](#function-syn_task_resume) ([**SYN\_Task**](structSYN__Task.md) \* task) <br>_Resume a suspended or condition-blocked task, making it eligible to run._  |
|  void | [**syn\_task\_set\_base\_priority**](#function-syn_task_set_base_priority) ([**SYN\_Task**](structSYN__Task.md) \* task, [**SYN\_Prio**](syn__task_8h.md#enum-syn_prio) new\_prio) <br>_Dynamically set a task's base priority and recompute active priority._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_task\_should\_share**](#function-syn_task_should_share) (const [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Query whether the running task should yield to share CPU time._  |
|  void | [**syn\_task\_suspend**](#function-syn_task_suspend) ([**SYN\_Task**](structSYN__Task.md) \* task) <br>_Suspend a task. It will be skipped by the scheduler until resumed._  |




























## Public Functions Documentation




### function syn\_sched\_alive\_count 

_Get the number of tasks that are still alive in the scheduler._ 
```C++
size_t syn_sched_alive_count (
    const SYN_Sched * sched
) 
```





**Parameters:**


* `sched` Scheduler. 



**Returns:**

Alive task count. 





        

<hr>



### function syn\_sched\_init 

_Initialize the scheduler with a task array._ 
```C++
void syn_sched_init (
    SYN_Sched * sched,
    SYN_Task * tasks,
    size_t count
) 
```





**Parameters:**


* `sched` Scheduler to initialize. 
* `tasks` Pointer to an array of [**SYN\_Task**](structSYN__Task.md) structs. Each task should already be initialized via [**syn\_task\_create()**](syn__sched_8h.md#function-syn_task_create). 
* `count` Number of tasks in the array. 




        

<hr>



### function syn\_sched\_run 

_Run one scheduler dispatch._ 
```C++
bool syn_sched_run (
    SYN_Sched * sched
) 
```



Selects and executes a single eligible task using ranked order (Overdue -&gt; Work -&gt; Poll -&gt; Round reset), ordered by priority and per-priority round-robin within every rank.




**Parameters:**


* `sched` Scheduler to run. 



**Returns:**

true if at least one task is still alive (not DEAD). 





        

<hr>



### function syn\_sched\_run\_forever 

_Run the scheduler forever._ 
```C++
SYN_NORETURN void syn_sched_run_forever (
    SYN_Sched * sched
) 
```



Equivalent to `while (1) { syn_sched_run(sched); }`. This function never returns.




**Parameters:**


* `sched` Scheduler to run. 




        

<hr>



### function syn\_sched\_set\_max\_wait 

_Configure the maximum starvation wait window for a priority level._ 
```C++
void syn_sched_set_max_wait (
    SYN_Sched * sched,
    SYN_Prio prio,
    uint16_t ms
) 
```



Values below 1 are clamped to 1 ms because tick resolution is 1 ms.




**Parameters:**


* `sched` Scheduler instance. 
* `prio` Priority level to configure. 
* `ms` Maximum wait window in milliseconds (clamped to &gt;= 1). 




        

<hr>



### function syn\_sched\_set\_profiler 

_Attach or detach a task execution profiler on_ `sched` _._
```C++
void syn_sched_set_profiler (
    SYN_Sched * sched,
    struct SYN_TaskProfileManager * profiler
) 
```



When a non-NULL enabled profiler is attached, [**syn\_sched\_run()**](syn__sched_8h.md#function-syn_sched_run) wraps every task dispatch with [**syn\_task\_profile\_step\_start()**](syn__task__profile_8c.md#function-syn_task_profile_step_start) and [**syn\_task\_profile\_step\_end()**](syn__task__profile_8c.md#function-syn_task_profile_step_end) using [**syn\_port\_get\_tick\_us()**](syn__port__system_8h.md#function-syn_port_get_tick_us), recording peak cooperative chunk duration (max\_yield\_us) per task slot. Pass NULL to detach.




**Parameters:**


* `sched` Scheduler instance. 
* `profiler` Profiler manager to attach, or NULL to detach. 




        

<hr>



### function syn\_task\_boost\_priority 

_Temporarily boost a task's execution priority._ 
```C++
void syn_task_boost_priority (
    SYN_Task * task,
    SYN_Prio temp_prio
) 
```



Monotonically ratchets `task->priority` upward when `temp_prio < task->priority` (`SYN_PRIO_CRITICAL = 0` is highest); calls with `temp_prio >= task->priority` are ignored so active higher boosts are never lowered. Promotes the task to Work rank and tightens `due_ms` to the boosted priority's starvation window when bound to a scheduler.




**Note:**

Main-loop / task context only (not ISR-safe). 




**Parameters:**


* `task` Task to boost. 
* `temp_prio` Temporary elevated priority. 




        

<hr>



### function syn\_task\_create 

_Initialize a single task descriptor._ 
```C++
void syn_task_create (
    SYN_Task * task,
    const char * name,
    SYN_TaskFunc func,
    SYN_Prio priority,
    void * user_data
) 
```



Sets the task to READY state with the protothread reset to the top.




**Parameters:**


* `task` Task to initialize. 
* `name` Human-readable name (stored by pointer, not copied). 
* `func` Protothread function. 
* `priority` Priority level (SYN\_PRIO\_CRITICAL = 0 is highest). 
* `user_data` Optional pointer to task-private data (or NULL). 




        

<hr>



### function syn\_task\_restart 

_Restart a task from the beginning of its protothread._ 
```C++
void syn_task_restart (
    SYN_Task * task
) 
```



Resets the protothread continuation, releases any held mutexes, restores base priority, and sets the task to READY.




**Note:**

Main-loop / task context only (not ISR-safe). 




**Parameters:**


* `task` Task to restart. 




        

<hr>



### function syn\_task\_restore\_priority 

_Restore a task's priority back to its configured base priority (preserving any active mutex priority-inheritance boost)._ 
```C++
void syn_task_restore_priority (
    SYN_Task * task
) 
```





**Note:**

Main-loop / task context only (not ISR-safe). 




**Parameters:**


* `task` Task to restore. 




        

<hr>



### function syn\_task\_resume 

_Resume a suspended or condition-blocked task, making it eligible to run._ 
```C++
void syn_task_resume (
    SYN_Task * task
) 
```





**Note:**

ISR-safe. Calling this from an ISR or driver wakes tasks parked in `PT_BLOCK_CONDITION` (and wrapper macros) or `syn_task_suspend()`. Tasks actively waiting on `PT_BLOCK_EVENT*` remain blocked until their event flag or timeout fires. 




**Parameters:**


* `task` Task to resume. 




        

<hr>



### function syn\_task\_set\_base\_priority 

_Dynamically set a task's base priority and recompute active priority._ 
```C++
void syn_task_set_base_priority (
    SYN_Task * task,
    SYN_Prio new_prio
) 
```



Preserves any higher active mutex boost, and promotes Work rank / tightens `due_ms` when raising effective priority.




**Note:**

Main-loop / task context only (not ISR-safe). 




**Parameters:**


* `task` Task to modify. 
* `new_prio` New base priority. 




        

<hr>



### function syn\_task\_should\_share 

_Query whether the running task should yield to share CPU time._ 
```C++
bool syn_task_should_share (
    const SYN_Task * task
) 
```



Returns true when another eligible task has active work (PT\_MORE) at equal or higher priority, or when any other eligible task has reached its starvation deadline. Returns false when called outside `task's` dispatch.




**Parameters:**


* `task` Task descriptor of the currently running task. 



**Returns:**

true if `task` should yield now, false otherwise. 





        

<hr>



### function syn\_task\_suspend 

_Suspend a task. It will be skipped by the scheduler until resumed._ 
```C++
void syn_task_suspend (
    SYN_Task * task
) 
```





**Note:**

Main-loop / task context only (not intended for arbitrary ISR use). 




**Parameters:**


* `task` Task to suspend. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_sched.c`

