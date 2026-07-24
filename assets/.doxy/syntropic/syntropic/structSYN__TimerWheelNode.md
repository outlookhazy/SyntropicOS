

# Struct SYN\_TimerWheelNode



[**ClassList**](annotated.md) **>** [**SYN\_TimerWheelNode**](structSYN__TimerWheelNode.md)



_Timer Wheel Node Structure._ 

* `#include <syn_timer_wheel.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  void \* | [**arg**](#variable-arg)  <br> |
|  SYN\_TimerWheelCb | [**cb**](#variable-cb)  <br> |
|  uint32\_t | [**expires\_tick**](#variable-expires_tick)  <br> |
|  struct [**SYN\_TimerWheelNode**](structSYN__TimerWheelNode.md) \* | [**next**](#variable-next)  <br> |
|  struct [**SYN\_TimerWheelNode**](structSYN__TimerWheelNode.md) \* | [**prev**](#variable-prev)  <br> |
|  uint32\_t | [**rotation\_count**](#variable-rotation_count)  <br> |












































## Public Attributes Documentation




### variable active 

```C++
bool SYN_TimerWheelNode::active;
```



Timer active flag 
 


        

<hr>



### variable arg 

```C++
void* SYN_TimerWheelNode::arg;
```



Callback user argument 
 


        

<hr>



### variable cb 

```C++
SYN_TimerWheelCb SYN_TimerWheelNode::cb;
```



Callback function pointer 
 


        

<hr>



### variable expires\_tick 

```C++
uint32_t SYN_TimerWheelNode::expires_tick;
```



Target expiration tick 
 


        

<hr>



### variable next 

```C++
struct SYN_TimerWheelNode* SYN_TimerWheelNode::next;
```



Pointer to next node in bucket 
 


        

<hr>



### variable prev 

```C++
struct SYN_TimerWheelNode* SYN_TimerWheelNode::prev;
```



Pointer to previous node in bucket 
 


        

<hr>



### variable rotation\_count 

```C++
uint32_t SYN_TimerWheelNode::rotation_count;
```



Rounds remaining before firing 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_timer_wheel.h`

