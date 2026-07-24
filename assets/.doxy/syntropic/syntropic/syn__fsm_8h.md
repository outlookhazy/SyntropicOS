

# File syn\_fsm.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_fsm.h**](syn__fsm_8h.md)

[Go to the source code of this file](syn__fsm_8h_source.md)

_Lightweight table-driven finite state machine._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_FSM**](structSYN__FSM.md) <br>_FSM instance — transition table, state descriptors, current state._  |
| struct | [**SYN\_FSM\_StateDesc**](structSYN__FSM__StateDesc.md) <br>_Per-state entry/exit actions._  |
| struct | [**SYN\_FSM\_Transition**](structSYN__FSM__Transition.md) <br>_Transition descriptor — (from, event) → to, with optional guard/action._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_FSM\_Action**](#typedef-syn_fsm_action)  <br>_Action function — called when a transition fires._  |
| typedef int16\_t | [**SYN\_FSM\_Event**](#typedef-syn_fsm_event)  <br> |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_FSM\_Guard**](#typedef-syn_fsm_guard)  <br>_Guard function — return true to allow the transition._  |
| typedef int16\_t | [**SYN\_FSM\_State**](#typedef-syn_fsm_state)  <br> |




















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
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_fsm\_in\_state**](#function-syn_fsm_in_state) (const [**SYN\_FSM**](structSYN__FSM.md) \* fsm, [**SYN\_FSM\_State**](syn__fsm_8h.md#typedef-syn_fsm_state) st) <br>_Check if the FSM is in a specific state._  |
|  [**SYN\_FSM\_State**](syn__fsm_8h.md#typedef-syn_fsm_state) | [**syn\_fsm\_state**](#function-syn_fsm_state) (const [**SYN\_FSM**](structSYN__FSM.md) \* fsm) <br>_Get the current state._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_FSM\_END**](syn__fsm_8h.md#define-syn_fsm_end)  `{ [**SYN\_FSM\_STATE\_NONE**](syn__fsm_8h.md#define-syn_fsm_state_none), 0, 0, NULL, NULL }`<br> |
| define  | [**SYN\_FSM\_STATE\_END**](syn__fsm_8h.md#define-syn_fsm_state_end)  `{ [**SYN\_FSM\_STATE\_NONE**](syn__fsm_8h.md#define-syn_fsm_state_none), NULL, NULL }`<br> |
| define  | [**SYN\_FSM\_STATE\_NONE**](syn__fsm_8h.md#define-syn_fsm_state_none)  `(([**SYN\_FSM\_State**](syn__fsm_8h.md#typedef-syn_fsm_state))-1)`<br> |

## Detailed Description


Define states and events as enums, then describe transitions in a const table. The FSM dispatches events, calls transition actions, and optionally logs every transition via the logging module.




**
**


```C++
enum { ST_IDLE, ST_RUNNING, ST_ERROR };
enum { EV_START, EV_STOP, EV_FAULT };

void on_start(void *ctx) { motor_enable(); }
void on_stop(void *ctx)  { motor_disable(); }

static const SYN_FSM_Transition table[] = {
    { ST_IDLE,    EV_START, ST_RUNNING, NULL, on_start },
    { ST_RUNNING, EV_STOP,  ST_IDLE,    NULL, on_stop  },
    { ST_RUNNING, EV_FAULT, ST_ERROR,   NULL, NULL     },
    SYN_FSM_END
};

static SYN_FSM fsm;
syn_fsm_init(&fsm, table, ST_IDLE, "motor");
syn_fsm_dispatch(&fsm, EV_START);
```
 





    
## Public Types Documentation




### typedef SYN\_FSM\_Action 

_Action function — called when a transition fires._ 
```C++
typedef void(* SYN_FSM_Action) (void *ctx);
```





**Parameters:**


* `ctx` User context. 




        

<hr>



### typedef SYN\_FSM\_Event 

```C++
typedef int16_t SYN_FSM_Event;
```



FSM event type. 
 


        

<hr>



### typedef SYN\_FSM\_Guard 

_Guard function — return true to allow the transition._ 
```C++
typedef bool(* SYN_FSM_Guard) (void *ctx);
```





**Parameters:**


* `ctx` User context. 



**Returns:**

true to allow, false to block. 





        

<hr>



### typedef SYN\_FSM\_State 

```C++
typedef int16_t SYN_FSM_State;
```



FSM state type. 
 


        

<hr>
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




### function syn\_fsm\_in\_state 

_Check if the FSM is in a specific state._ 
```C++
static inline bool syn_fsm_in_state (
    const SYN_FSM * fsm,
    SYN_FSM_State st
) 
```





**Parameters:**


* `fsm` FSM instance. 
* `st` State to check. 



**Returns:**

true if in that state. 





        

<hr>



### function syn\_fsm\_state 

_Get the current state._ 
```C++
static inline SYN_FSM_State syn_fsm_state (
    const SYN_FSM * fsm
) 
```





**Parameters:**


* `fsm` FSM instance. 



**Returns:**

Current state. 





        

<hr>
## Macro Definition Documentation





### define SYN\_FSM\_END 

```C++
#define SYN_FSM_END `{ SYN_FSM_STATE_NONE , 0, 0, NULL, NULL }`
```



Table terminator. Place at the end of your transition array. 


        

<hr>



### define SYN\_FSM\_STATE\_END 

```C++
#define SYN_FSM_STATE_END `{ SYN_FSM_STATE_NONE , NULL, NULL }`
```



State descriptor table terminator. 


        

<hr>



### define SYN\_FSM\_STATE\_NONE 

```C++
#define SYN_FSM_STATE_NONE `(( SYN_FSM_State )-1)`
```



Sentinel value marking the end of a transition table. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_fsm.h`

