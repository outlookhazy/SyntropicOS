

# File syn\_fsm.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_fsm.c**](syn__fsm_8c.md)

[Go to the source code of this file](syn__fsm_8c_source.md)

_Finite state machine implementation._ 

* `#include "syn_fsm.h"`
* `#include "syn_assert.h"`
* `#include "../log/syn_log.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_fsm\_dispatch**](#function-syn_fsm_dispatch) ([**SYN\_FSM**](structSYN__FSM.md) \* fsm, [**SYN\_FSM\_Event**](syn__fsm_8h.md#typedef-syn_fsm_event) event) <br>_Dispatch an event._  |
|  void | [**syn\_fsm\_init**](#function-syn_fsm_init) ([**SYN\_FSM**](structSYN__FSM.md) \* fsm, const [**SYN\_FSM\_Transition**](structSYN__FSM__Transition.md) \* transitions, [**SYN\_FSM\_State**](syn__fsm_8h.md#typedef-syn_fsm_state) initial, const char \* tag) <br>_Initialize the FSM._  |
|  void | [**syn\_fsm\_set\_context**](#function-syn_fsm_set_context) ([**SYN\_FSM**](structSYN__FSM.md) \* fsm, void \* ctx) <br>_Set the user context pointer passed to guards and actions._  |
|  void | [**syn\_fsm\_set\_state**](#function-syn_fsm_set_state) ([**SYN\_FSM**](structSYN__FSM.md) \* fsm, [**SYN\_FSM\_State**](syn__fsm_8h.md#typedef-syn_fsm_state) state) <br>_Force-set the FSM to a specific state._  |
|  void | [**syn\_fsm\_set\_state\_descs**](#function-syn_fsm_set_state_descs) ([**SYN\_FSM**](structSYN__FSM.md) \* fsm, const [**SYN\_FSM\_StateDesc**](structSYN__FSM__StateDesc.md) \* descs) <br>_Set optional state descriptors (entry/exit actions)._  |
|  void | [**syn\_fsm\_set\_state\_names**](#function-syn_fsm_set_state_names) ([**SYN\_FSM**](structSYN__FSM.md) \* fsm, const char \*const \* names) <br>_Set optional state name strings for debug/logging._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  const [**SYN\_FSM\_StateDesc**](structSYN__FSM__StateDesc.md) \* | [**find\_state\_desc**](#function-find_state_desc) (const [**SYN\_FSM**](structSYN__FSM.md) \* fsm, [**SYN\_FSM\_State**](syn__fsm_8h.md#typedef-syn_fsm_state) state) <br>_Find the state descriptor for a given state._  |
|  void | [**fire\_enter**](#function-fire_enter) (const [**SYN\_FSM**](structSYN__FSM.md) \* fsm, [**SYN\_FSM\_State**](syn__fsm_8h.md#typedef-syn_fsm_state) state) <br>_Invoke the on\_enter callback for a state._  |
|  void | [**fire\_exit**](#function-fire_exit) (const [**SYN\_FSM**](structSYN__FSM.md) \* fsm, [**SYN\_FSM\_State**](syn__fsm_8h.md#typedef-syn_fsm_state) state) <br>_Invoke the on\_exit callback for a state._  |


























## Public Functions Documentation




### function syn\_fsm\_dispatch 

_Dispatch an event._ 
```C++
bool syn_fsm_dispatch (
    SYN_FSM * fsm,
    SYN_FSM_Event event
) 
```



Scans the transition table for a matching (current\_state, event) pair. If a guard exists and returns false, the transition is skipped and the next matching row is tried. On a successful match:
* Exit action for the current state (if state\_descs set)
* Transition action
* State change
* Entry action for the new state (if state\_descs set)






**Parameters:**


* `fsm` FSM instance. 
* `event` Event to dispatch. 



**Returns:**

true if a transition was taken, false if no match found. 





        

<hr>



### function syn\_fsm\_init 

_Initialize the FSM._ 
```C++
void syn_fsm_init (
    SYN_FSM * fsm,
    const SYN_FSM_Transition * transitions,
    SYN_FSM_State initial,
    const char * tag
) 
```





**Parameters:**


* `fsm` FSM instance. 
* `transitions` Transition table (terminated by SYN\_FSM\_END). 
* `initial` Initial state. 
* `tag` Log tag for transition logging (e.g., "motor"). 




        

<hr>



### function syn\_fsm\_set\_context 

_Set the user context pointer passed to guards and actions._ 
```C++
void syn_fsm_set_context (
    SYN_FSM * fsm,
    void * ctx
) 
```





**Parameters:**


* `fsm` FSM instance. 
* `ctx` User context. 




        

<hr>



### function syn\_fsm\_set\_state 

_Force-set the FSM to a specific state._ 
```C++
void syn_fsm_set_state (
    SYN_FSM * fsm,
    SYN_FSM_State state
) 
```



Fires exit action for the old state and entry action for the new state (if state\_descs are set). No transition action is called.




**Parameters:**


* `fsm` FSM instance. 
* `state` New state. 




        

<hr>



### function syn\_fsm\_set\_state\_descs 

_Set optional state descriptors (entry/exit actions)._ 
```C++
void syn_fsm_set_state_descs (
    SYN_FSM * fsm,
    const SYN_FSM_StateDesc * descs
) 
```





**Parameters:**


* `fsm` FSM instance. 
* `descs` State descriptor table (terminated by SYN\_FSM\_STATE\_END). 




        

<hr>



### function syn\_fsm\_set\_state\_names 

_Set optional state name strings for debug/logging._ 
```C++
void syn_fsm_set_state_names (
    SYN_FSM * fsm,
    const char *const * names
) 
```





**Parameters:**


* `fsm` FSM instance. 
* `names` Array of state name strings, indexed by state value. Must cover all state values used in the transition table. 




        

<hr>
## Public Static Functions Documentation




### function find\_state\_desc 

_Find the state descriptor for a given state._ 
```C++
static const SYN_FSM_StateDesc * find_state_desc (
    const SYN_FSM * fsm,
    SYN_FSM_State state
) 
```





**Parameters:**


* `fsm` FSM instance. 
* `state` State to look up. 



**Returns:**

Descriptor, or NULL if not found. 





        

<hr>



### function fire\_enter 

_Invoke the on\_enter callback for a state._ 
```C++
static void fire_enter (
    const SYN_FSM * fsm,
    SYN_FSM_State state
) 
```





**Parameters:**


* `fsm` FSM instance. 
* `state` State being entered. 




        

<hr>



### function fire\_exit 

_Invoke the on\_exit callback for a state._ 
```C++
static void fire_exit (
    const SYN_FSM * fsm,
    SYN_FSM_State state
) 
```





**Parameters:**


* `fsm` FSM instance. 
* `state` State being exited. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_fsm.c`

