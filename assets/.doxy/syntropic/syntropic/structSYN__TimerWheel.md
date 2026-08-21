

# Struct SYN\_TimerWheel



[**ClassList**](annotated.md) **>** [**SYN\_TimerWheel**](structSYN__TimerWheel.md)



_Hashed Timing Wheel Descriptor._ 

* `#include <syn_timer_wheel.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_TimerWheelNode**](structSYN__TimerWheelNode.md) \* | [**buckets**](#variable-buckets)  <br> |
|  uint32\_t | [**current\_tick**](#variable-current_tick)  <br> |












































## Public Attributes Documentation




### variable buckets 

```C++
SYN_TimerWheelNode* SYN_TimerWheel::buckets[SYN_TIMER_WHEEL_BUCKETS];
```



Bucket list array 


        

<hr>



### variable current\_tick 

```C++
uint32_t SYN_TimerWheel::current_tick;
```



Current wheel tick 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_timer_wheel.h`

