

# File syn\_task.h



[**FileList**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_task.h**](syn__task_8h.md)

[Go to the source code of this file](syn__task_8h_source.md)

_Task descriptor for the cooperative scheduler._ [More...](#detailed-description)

* `#include "../pt/syn_pt.h"`
* `#include "syn_event_flags.h"`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Task**](structSYN__Task.md) <br>_Task descriptor — binds a protothread to scheduler metadata._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_Prio**](#enum-syn_prio)  <br>_Cooperative task priority levels (0 = highest)._  |
| typedef [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status)(\* | [**SYN\_TaskFunc**](#typedef-syn_taskfunc)  <br>_Protothread task function._  |
| enum  | [**SYN\_TaskState**](#enum-syn_taskstate)  <br>_Cooperative task lifecycle state._  |















































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_SCHED\_FLAG\_DUE\_VALID**](syn__task_8h.md#define-syn_sched_flag_due_valid)  `0x08U`<br>_Task due\_ms has been stamped by the scheduler._  |
| define  | [**SYN\_SCHED\_FLAG\_ISR\_WAKE**](syn__task_8h.md#define-syn_sched_flag_isr_wake)  `0x10U`<br>_Task was resumed asynchronously during its current dispatch._  |
| define  | [**SYN\_SCHED\_FLAG\_MORE**](syn__task_8h.md#define-syn_sched_flag_more)  `0x01U`<br>_Task reported PT\_MORE on last run, woke from sleep, or was boosted._  |
| define  | [**SYN\_SCHED\_FLAG\_MUTEX\_WAIT**](syn__task_8h.md#define-syn_sched_flag_mutex_wait)  `0x20U`<br>_Task is actively contended in a_ [_**SYN\_Mutex**_](structSYN__Mutex.md) _waiter list._ |
| define  | [**SYN\_SCHED\_FLAG\_PASSED**](syn__task_8h.md#define-syn_sched_flag_passed)  `0x02U`<br>_Task reported PT\_IDLE during the current polling round._  |
| define  | [**SYN\_SCHED\_FLAG\_RESTARTED**](syn__task_8h.md#define-syn_sched_flag_restarted)  `0x40U`<br>_Task called_ [_**syn\_task\_restart()**_](syn__sched_8c.md#function-syn_task_restart) _on itself during its current dispatch._ |
| define  | [**SYN\_SCHED\_FLAG\_SLEEPING**](syn__task_8h.md#define-syn_sched_flag_sleeping)  `0x04U`<br>_Task was delayed, blocked, or suspended after its last dispatch._  |
| define  | [**SYN\_TASK\_NO\_MANUAL\_PRIO**](syn__task_8h.md#define-syn_task_no_manual_prio)  `0xFFU`<br>_Sentinel value for_ [_**SYN\_Task.manual\_priority**_](structSYN__Task.md#variable-manual_priority) _when no manual boost is active._ |

## Detailed Description


Defines the task control block ([**SYN\_Task**](structSYN__Task.md)) that pairs a protothread with scheduling metadata: priority, state, name, delay target, and optional event-wait fields for true blocking.


Tasks are caller-owned — you allocate them however you like (static array, global, on the stack). The scheduler just takes a pointer to your array.




**
**

[**syn\_task\_restart()**](syn__sched_8c.md#function-syn_task_restart) resets the protothread continuation (lc), delay deadlines, and event wait masks. It does NOT reset static local variables inside the task function. Place all state initialization after [**PT\_BEGIN()**](syn__pt_8h.md#define-pt_begin) or reset static state inside task-private user\_data structs to ensure clean restarts. 





    
## Public Types Documentation




### enum SYN\_Prio 

_Cooperative task priority levels (0 = highest)._ 
```C++
enum SYN_Prio {
    SYN_PRIO_CRITICAL = 0,
    SYN_PRIO_HIGH = 1,
    SYN_PRIO_NORMAL = 2,
    SYN_PRIO_LOW = 3,
    SYN_PRIO_LEVEL_MAX = 255
};
```




<hr>



### typedef SYN\_TaskFunc 

_Protothread task function._ 
```C++
typedef SYN_PT_Status(* SYN_TaskFunc) (SYN_PT *pt, struct SYN_Task *task);
```



A task function receives its own protothread and task descriptor. It must follow the PT\_BEGIN / PT\_END pattern.




**Parameters:**


* `pt` Pointer to the task's protothread (same as &task-&gt;pt). 
* `task` Pointer to the task descriptor (for user\_data, delay, etc.). 



**Returns:**

Protothread status (PT\_IDLE, PT\_MORE, PT\_EXITED, or PT\_ENDED). 





        

<hr>



### enum SYN\_TaskState 

_Cooperative task lifecycle state._ 
```C++
enum SYN_TaskState {
    SYN_TASK_READY = 0,
    SYN_TASK_SUSPENDED = 1,
    SYN_TASK_DEAD = 2,
    SYN_TASK_BLOCKED = 3
};
```




<hr>
## Macro Definition Documentation





### define SYN\_SCHED\_FLAG\_DUE\_VALID 

_Task due\_ms has been stamped by the scheduler._ 
```C++
#define SYN_SCHED_FLAG_DUE_VALID `0x08U`
```




<hr>



### define SYN\_SCHED\_FLAG\_ISR\_WAKE 

_Task was resumed asynchronously during its current dispatch._ 
```C++
#define SYN_SCHED_FLAG_ISR_WAKE `0x10U`
```




<hr>



### define SYN\_SCHED\_FLAG\_MORE 

_Task reported PT\_MORE on last run, woke from sleep, or was boosted._ 
```C++
#define SYN_SCHED_FLAG_MORE `0x01U`
```




<hr>



### define SYN\_SCHED\_FLAG\_MUTEX\_WAIT 

_Task is actively contended in a_ [_**SYN\_Mutex**_](structSYN__Mutex.md) _waiter list._
```C++
#define SYN_SCHED_FLAG_MUTEX_WAIT `0x20U`
```




<hr>



### define SYN\_SCHED\_FLAG\_PASSED 

_Task reported PT\_IDLE during the current polling round._ 
```C++
#define SYN_SCHED_FLAG_PASSED `0x02U`
```




<hr>



### define SYN\_SCHED\_FLAG\_RESTARTED 

_Task called_ [_**syn\_task\_restart()**_](syn__sched_8c.md#function-syn_task_restart) _on itself during its current dispatch._
```C++
#define SYN_SCHED_FLAG_RESTARTED `0x40U`
```




<hr>



### define SYN\_SCHED\_FLAG\_SLEEPING 

_Task was delayed, blocked, or suspended after its last dispatch._ 
```C++
#define SYN_SCHED_FLAG_SLEEPING `0x04U`
```




<hr>



### define SYN\_TASK\_NO\_MANUAL\_PRIO 

_Sentinel value for_ [_**SYN\_Task.manual\_priority**_](structSYN__Task.md#variable-manual_priority) _when no manual boost is active._
```C++
#define SYN_TASK_NO_MANUAL_PRIO `0xFFU`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_task.h`

