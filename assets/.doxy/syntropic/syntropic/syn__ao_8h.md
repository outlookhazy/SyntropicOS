

# File syn\_ao.h



[**FileList**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_ao.h**](syn__ao_8h.md)

[Go to the source code of this file](syn__ao_8h_source.md)

_Active Object (Actor model) framework wrapper._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../sched/syn_task.h"`
* `#include "../pt/syn_pt.h"`
* `#include "../sched/syn_mailbox.h"`
* `#include "../util/syn_fsm.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_AO\_Event**](structSYN__AO__Event.md) <br>_Active Object Event._  |
| struct | [**SYN\_ActiveObject**](structSYN__ActiveObject.md) <br>_Active Object structure._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_ao\_init**](#function-syn_ao_init) ([**SYN\_ActiveObject**](structSYN__ActiveObject.md) \* ao, const char \* name, const [**SYN\_FSM\_Transition**](structSYN__FSM__Transition.md) \* transitions, [**SYN\_FSM\_State**](syn__fsm_8h.md#typedef-syn_fsm_state) initial\_state, void \* mailbox\_buf, size\_t mailbox\_cap) <br>_Initialize an Active Object._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ao\_post**](#function-syn_ao_post) ([**SYN\_ActiveObject**](structSYN__ActiveObject.md) \* ao, uint16\_t sig, void \* data) <br>_Post an event to the Active Object's mailbox._  |




























## Detailed Description


Combines cooperative tasks, protothreads, mailboxes, and state machines to enforce strict event-driven encapsulation. 


    
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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_ao.h`

