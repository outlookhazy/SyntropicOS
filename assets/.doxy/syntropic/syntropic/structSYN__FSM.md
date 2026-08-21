

# Struct SYN\_FSM



[**ClassList**](annotated.md) **>** [**SYN\_FSM**](structSYN__FSM.md)



_FSM instance — transition table, state descriptors, current state._ 

* `#include <syn_fsm.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  void \* | [**ctx**](#variable-ctx)  <br> |
|  [**SYN\_FSM\_State**](syn__fsm_8h.md#typedef-syn_fsm_state) | [**current**](#variable-current)  <br> |
|  const [**SYN\_FSM\_StateDesc**](structSYN__FSM__StateDesc.md) \* | [**state\_descs**](#variable-state_descs)  <br> |
|  const char \*const  \* | [**state\_names**](#variable-state_names)  <br> |
|  const char \* | [**tag**](#variable-tag)  <br> |
|  const [**SYN\_FSM\_Transition**](structSYN__FSM__Transition.md) \* | [**transitions**](#variable-transitions)  <br> |












































## Public Attributes Documentation




### variable ctx 

```C++
void* SYN_FSM::ctx;
```



User context for actions 


        

<hr>



### variable current 

```C++
SYN_FSM_State SYN_FSM::current;
```



Current state 
 


        

<hr>



### variable state\_descs 

```C++
const SYN_FSM_StateDesc* SYN_FSM::state_descs;
```



Optional (may be NULL) 
 


        

<hr>



### variable state\_names 

```C++
const char* const* SYN_FSM::state_names;
```



Optional (may be NULL) 
 


        

<hr>



### variable tag 

```C++
const char* SYN_FSM::tag;
```



Log tag 
 


        

<hr>



### variable transitions 

```C++
const SYN_FSM_Transition* SYN_FSM::transitions;
```



Transition table 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_fsm.h`

