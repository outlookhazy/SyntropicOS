

# Struct SYN\_FSM\_StateDesc



[**ClassList**](annotated.md) **>** [**SYN\_FSM\_StateDesc**](structSYN__FSM__StateDesc.md)



_Per-state entry/exit actions._ [More...](#detailed-description)

* `#include <syn_fsm.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_FSM\_Action**](syn__fsm_8h.md#typedef-syn_fsm_action) | [**on\_enter**](#variable-on_enter)  <br> |
|  [**SYN\_FSM\_Action**](syn__fsm_8h.md#typedef-syn_fsm_action) | [**on\_exit**](#variable-on_exit)  <br> |
|  [**SYN\_FSM\_State**](syn__fsm_8h.md#typedef-syn_fsm_state) | [**state**](#variable-state)  <br> |












































## Detailed Description


Optional — pass NULL to [**syn\_fsm\_init()**](syn__fsm_8c.md#function-syn_fsm_init) if you don't need them.


Per-state entry/exit action descriptor. 


    
## Public Attributes Documentation




### variable on\_enter 

```C++
SYN_FSM_Action SYN_FSM_StateDesc::on_enter;
```



Called on state entry (or NULL) 
 


        

<hr>



### variable on\_exit 

```C++
SYN_FSM_Action SYN_FSM_StateDesc::on_exit;
```



Called on state exit (or NULL) 
 


        

<hr>



### variable state 

```C++
SYN_FSM_State SYN_FSM_StateDesc::state;
```



State this applies to 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_fsm.h`

