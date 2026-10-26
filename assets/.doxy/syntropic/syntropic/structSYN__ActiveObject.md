

# Struct SYN\_ActiveObject



[**ClassList**](annotated.md) **>** [**SYN\_ActiveObject**](structSYN__ActiveObject.md)



_Active Object structure._ 

* `#include <syn_ao.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_FSM**](structSYN__FSM.md) | [**fsm**](#variable-fsm)  <br> |
|  [**SYN\_AO\_Event**](structSYN__AO__Event.md) | [**last\_event**](#variable-last_event)  <br> |
|  [**SYN\_Mailbox**](structSYN__Mailbox.md) | [**mailbox**](#variable-mailbox)  <br> |
|  [**SYN\_PT**](structSYN__PT.md) | [**pt**](#variable-pt)  <br> |
|  [**SYN\_Task**](structSYN__Task.md) | [**task**](#variable-task)  <br> |












































## Public Attributes Documentation




### variable fsm 

```C++
SYN_FSM SYN_ActiveObject::fsm;
```



State machine 
 


        

<hr>



### variable last\_event 

```C++
SYN_AO_Event SYN_ActiveObject::last_event;
```



Most recent dispatched event (access in actions via ao-&gt;last\_event) 


        

<hr>



### variable mailbox 

```C++
SYN_Mailbox SYN_ActiveObject::mailbox;
```



Event mailbox 
 


        

<hr>



### variable pt 

```C++
SYN_PT SYN_ActiveObject::pt;
```



Task protothread state 
 


        

<hr>



### variable task 

```C++
SYN_Task SYN_ActiveObject::task;
```



Scheduler task handle 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_ao.h`

