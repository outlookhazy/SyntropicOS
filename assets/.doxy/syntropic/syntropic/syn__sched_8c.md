

# File syn\_sched.c



[**FileList**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_sched.c**](syn__sched_8c.md)

[Go to the source code of this file](syn__sched_8c_source.md)

_Cooperative scheduler implementation._ 

* `#include "syn_sched.h"`
* `#include "../port/syn_port_system.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_event.h"`
* `#include "../util/syn_metrics.h"`
* `#include <limits.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_sched\_alive\_count**](#function-syn_sched_alive_count) (const [**SYN\_Sched**](structSYN__Sched.md) \* sched) <br>_Get the number of tasks that are still alive in the scheduler._  |
|  void | [**syn\_sched\_init**](#function-syn_sched_init) ([**SYN\_Sched**](structSYN__Sched.md) \* sched, [**SYN\_Task**](structSYN__Task.md) \* tasks, size\_t count) <br>_Initialize the scheduler with a task array._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_sched\_run**](#function-syn_sched_run) ([**SYN\_Sched**](structSYN__Sched.md) \* sched) <br>_Run one scheduler tick._  |
|  [**SYN\_NORETURN**](syn__compiler_8h.md#define-syn_noreturn) void | [**syn\_sched\_run\_forever**](#function-syn_sched_run_forever) ([**SYN\_Sched**](structSYN__Sched.md) \* sched) <br>_Run the scheduler forever._  |
|  void | [**syn\_task\_create**](#function-syn_task_create) ([**SYN\_Task**](structSYN__Task.md) \* task, const char \* name, [**SYN\_TaskFunc**](syn__task_8h.md#typedef-syn_taskfunc) func, uint8\_t priority, void \* user\_data) <br>_Initialize a single task descriptor._  |
|  void | [**syn\_task\_restart**](#function-syn_task_restart) ([**SYN\_Task**](structSYN__Task.md) \* task) <br>_Restart a task from the beginning of its protothread._  |
|  void | [**syn\_task\_resume**](#function-syn_task_resume) ([**SYN\_Task**](structSYN__Task.md) \* task) <br>_Resume a suspended task, making it eligible to run again._  |
|  void | [**syn\_task\_suspend**](#function-syn_task_suspend) ([**SYN\_Task**](structSYN__Task.md) \* task) <br>_Suspend a task. It will be skipped by the scheduler until resumed._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**sched\_run\_task**](#function-sched_run_task) ([**SYN\_Task**](structSYN__Task.md) \* task) <br>_Execute a single task's protothread function._  |


























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

_Run one scheduler tick._ 
```C++
bool syn_sched_run (
    SYN_Sched * sched
) 
```



Scans all tasks and runs the single highest-priority ready task (lowest priority value first, round-robin among equal priorities).




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



### function syn\_task\_create 

_Initialize a single task descriptor._ 
```C++
void syn_task_create (
    SYN_Task * task,
    const char * name,
    SYN_TaskFunc func,
    uint8_t priority,
    void * user_data
) 
```



Sets the task to READY state with the protothread reset to the top.




**Parameters:**


* `task` Task to initialize. 
* `name` Human-readable name (stored by pointer, not copied). 
* `func` Protothread function. 
* `priority` Priority level (0 = highest). 
* `user_data` Optional pointer to task-private data (or NULL). 




        

<hr>



### function syn\_task\_restart 

_Restart a task from the beginning of its protothread._ 
```C++
void syn_task_restart (
    SYN_Task * task
) 
```



Resets the protothread continuation and sets the task to READY.




**Parameters:**


* `task` Task to restart. 




        

<hr>



### function syn\_task\_resume 

_Resume a suspended task, making it eligible to run again._ 
```C++
void syn_task_resume (
    SYN_Task * task
) 
```





**Parameters:**


* `task` Task to resume. 




        

<hr>



### function syn\_task\_suspend 

_Suspend a task. It will be skipped by the scheduler until resumed._ 
```C++
void syn_task_suspend (
    SYN_Task * task
) 
```





**Parameters:**


* `task` Task to suspend. 




        

<hr>
## Public Static Functions Documentation




### function sched\_run\_task 

_Execute a single task's protothread function._ 
```C++
static SYN_PT_Status sched_run_task (
    SYN_Task * task
) 
```





**Parameters:**


* `task` Task to run. 



**Returns:**

The protothread status returned by the task. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_sched.c`

