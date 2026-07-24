

# File syn\_ao.c



[**FileList**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_ao.c**](syn__ao_8c.md)

[Go to the source code of this file](syn__ao_8c_source.md)

_Active Object framework implementation._ 

* `#include "syn_ao.h"`
* `#include "../sched/syn_sched.h"`
* `#include "../port/syn_port_system.h"`
* `#include "../util/syn_assert.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_ao\_init**](#function-syn_ao_init) ([**SYN\_ActiveObject**](structSYN__ActiveObject.md) \* ao, const char \* name, const [**SYN\_FSM\_Transition**](structSYN__FSM__Transition.md) \* transitions, [**SYN\_FSM\_State**](syn__fsm_8h.md#typedef-syn_fsm_state) initial\_state, void \* mailbox\_buf, size\_t mailbox\_cap) <br>_Initialize an Active Object._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ao\_post**](#function-syn_ao_post) ([**SYN\_ActiveObject**](structSYN__ActiveObject.md) \* ao, uint16\_t sig, void \* data) <br>_Post an event to the Active Object's mailbox._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_ao\_pt\_run**](#function-syn_ao_pt_run) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Protothread entry for active objects_  _dispatches queued events._ |


























## Public Functions Documentation




### function syn\_ao\_init 

_Initialize an Active Object._ 
```C++
void syn_ao_init (
    SYN_ActiveObject * ao,
    const char * name,
    const SYN_FSM_Transition * transitions,
    SYN_FSM_State initial_state,
    void * mailbox_buf,
    size_t mailbox_cap
) 
```





**Parameters:**


* `ao` Active Object instance. 
* `name` Name of the active object (used for task/logging). 
* `transitions` Transition table for the internal state machine. 
* `initial_state` Initial state of the state machine. 
* `mailbox_buf` Buffer backing the mailbox event queue. 
* `mailbox_cap` Capacity (number of elements) of the buffer. 




        

<hr>



### function syn\_ao\_post 

_Post an event to the Active Object's mailbox._ 
```C++
bool syn_ao_post (
    SYN_ActiveObject * ao,
    uint16_t sig,
    void * data
) 
```



Thread-safe and ISR-safe. Protects the mailbox write using critical sections.




**Parameters:**


* `ao` Active Object instance. 
* `sig` Signal identifier. 
* `data` Optional payload. 



**Returns:**

true if the event was successfully queued, false if queue is full. 





        

<hr>
## Public Static Functions Documentation




### function syn\_ao\_pt\_run 

_Protothread entry for active objects_  _dispatches queued events._
```C++
static SYN_PT_Status syn_ao_pt_run (
    SYN_PT * pt,
    SYN_Task * task
) 
```





**Parameters:**


* `pt` Protothread state. 
* `task` Owning scheduler task. 



**Returns:**

PT status. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_ao.c`

