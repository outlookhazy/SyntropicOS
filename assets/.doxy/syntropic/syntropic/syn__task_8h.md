

# File syn\_task.h



[**FileList**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_task.h**](syn__task_8h.md)

[Go to the source code of this file](syn__task_8h_source.md)

_Task descriptor for the cooperative scheduler._ [More...](#detailed-description)

* `#include "../pt/syn_pt.h"`
* `#include <stdint.h>`
* `#include "syn_event_flags.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Task**](structSYN__Task.md) <br>_Task descriptor — binds a protothread to scheduler metadata._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status)(\* | [**SYN\_TaskFunc**](#typedef-syn_taskfunc)  <br>_Protothread task function._  |
| enum  | [**SYN\_TaskState**](#enum-syn_taskstate)  <br>_Cooperative task lifecycle state._  |
















































## Detailed Description


Defines the task control block ([**SYN\_Task**](structSYN__Task.md)) that pairs a protothread with scheduling metadata: priority, state, name, delay target, and optional event-wait fields for true blocking.


Tasks are caller-owned — you allocate them however you like (static array, global, on the stack). The scheduler just takes a pointer to your array. 


    
## Public Types Documentation




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

PT status indicating whether the thread yielded, is waiting, or exited. 





        

<hr>



### enum SYN\_TaskState 

_Cooperative task lifecycle state._ 
```C++
enum SYN_TaskState {
    SYN_TASK_READY = 0,
    SYN_TASK_SUSPENDED = 1,
    SYN_TASK_DEAD = 2,
    SYN_TASK_DEFERRED = 3,
    SYN_TASK_BLOCKED = 4,
    SYN_TASK_WAITING = 5
};
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_task.h`

