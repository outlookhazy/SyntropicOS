

# Struct SYN\_FSM\_Transition



[**ClassList**](annotated.md) **>** [**SYN\_FSM\_Transition**](structSYN__FSM__Transition.md)



_Transition descriptor — (from, event) → to, with optional guard/action._ 

* `#include <syn_fsm.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_FSM\_Action**](syn__fsm_8h.md#typedef-syn_fsm_action) | [**action**](#variable-action)  <br> |
|  [**SYN\_FSM\_Event**](syn__fsm_8h.md#typedef-syn_fsm_event) | [**event**](#variable-event)  <br> |
|  [**SYN\_FSM\_State**](syn__fsm_8h.md#typedef-syn_fsm_state) | [**from**](#variable-from)  <br> |
|  [**SYN\_FSM\_Guard**](syn__fsm_8h.md#typedef-syn_fsm_guard) | [**guard**](#variable-guard)  <br> |
|  [**SYN\_FSM\_State**](syn__fsm_8h.md#typedef-syn_fsm_state) | [**to**](#variable-to)  <br> |












































## Public Attributes Documentation




### variable action 

```C++
SYN_FSM_Action SYN_FSM_Transition::action;
```



Transition action (NULL = none) 
 


        

<hr>



### variable event 

```C++
SYN_FSM_Event SYN_FSM_Transition::event;
```



Triggering event 
 


        

<hr>



### variable from 

```C++
SYN_FSM_State SYN_FSM_Transition::from;
```



Source state 
 


        

<hr>



### variable guard 

```C++
SYN_FSM_Guard SYN_FSM_Transition::guard;
```



Guard function (NULL = always allow) 
 


        

<hr>



### variable to 

```C++
SYN_FSM_State SYN_FSM_Transition::to;
```



Destination state 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_fsm.h`

