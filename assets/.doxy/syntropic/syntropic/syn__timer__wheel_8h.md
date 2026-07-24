

# File syn\_timer\_wheel.h



[**FileList**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_timer\_wheel.h**](syn__timer__wheel_8h.md)

[Go to the source code of this file](syn__timer__wheel_8h_source.md)

_Hashed Timing Wheel Scheduler._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_TimerWheel**](structSYN__TimerWheel.md) <br>_Hashed Timing Wheel Descriptor._  |
| struct | [**SYN\_TimerWheelNode**](structSYN__TimerWheelNode.md) <br>_Timer Wheel Node Structure._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_TimerWheelCb**](#typedef-syn_timerwheelcb)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_timer\_wheel\_add**](#function-syn_timer_wheel_add) ([**SYN\_TimerWheel**](structSYN__TimerWheel.md) \* wheel, [**SYN\_TimerWheelNode**](structSYN__TimerWheelNode.md) \* node, uint32\_t delay\_ticks, SYN\_TimerWheelCb cb, void \* arg) <br>_Schedule a timer in the timing wheel._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_timer\_wheel\_cancel**](#function-syn_timer_wheel_cancel) ([**SYN\_TimerWheel**](structSYN__TimerWheel.md) \* wheel, [**SYN\_TimerWheelNode**](structSYN__TimerWheelNode.md) \* node) <br>_Cancel a scheduled timer node._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_timer\_wheel\_init**](#function-syn_timer_wheel_init) ([**SYN\_TimerWheel**](structSYN__TimerWheel.md) \* wheel) <br>_Initialize a timing wheel scheduler._  |
|  size\_t | [**syn\_timer\_wheel\_step**](#function-syn_timer_wheel_step) ([**SYN\_TimerWheel**](structSYN__TimerWheel.md) \* wheel) <br>_Advance the timing wheel by one tick and fire expired timers._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_TIMER\_WHEEL\_BUCKETS**](syn__timer__wheel_8h.md#define-syn_timer_wheel_buckets)  `64U`<br> |

## Detailed Description


Implements an O(1) constant-time hashed timing wheel scheduler optimized for low-power tickless Operation with hundreds of concurrent software timers. 


    
## Public Types Documentation




### typedef SYN\_TimerWheelCb 

```C++
typedef void(* SYN_TimerWheelCb) (void *arg);
```




<hr>
## Public Functions Documentation




### function syn\_timer\_wheel\_add 

_Schedule a timer in the timing wheel._ 
```C++
SYN_Status syn_timer_wheel_add (
    SYN_TimerWheel * wheel,
    SYN_TimerWheelNode * node,
    uint32_t delay_ticks,
    SYN_TimerWheelCb cb,
    void * arg
) 
```





**Parameters:**


* `wheel` Pointer to wheel descriptor. 
* `node` Pointer to timer node memory. 
* `delay_ticks` Delay in ticks before timer fires. 
* `cb` Expiration callback. 
* `arg` Callback user argument. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_timer\_wheel\_cancel 

_Cancel a scheduled timer node._ 
```C++
SYN_Status syn_timer_wheel_cancel (
    SYN_TimerWheel * wheel,
    SYN_TimerWheelNode * node
) 
```





**Parameters:**


* `wheel` Pointer to wheel descriptor. 
* `node` Pointer to timer node. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_timer\_wheel\_init 

_Initialize a timing wheel scheduler._ 
```C++
SYN_Status syn_timer_wheel_init (
    SYN_TimerWheel * wheel
) 
```





**Parameters:**


* `wheel` Pointer to wheel descriptor. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_timer\_wheel\_step 

_Advance the timing wheel by one tick and fire expired timers._ 
```C++
size_t syn_timer_wheel_step (
    SYN_TimerWheel * wheel
) 
```





**Parameters:**


* `wheel` Pointer to wheel descriptor. 



**Returns:**

Number of timers fired during this tick step. 





        

<hr>
## Macro Definition Documentation





### define SYN\_TIMER\_WHEEL\_BUCKETS 

```C++
#define SYN_TIMER_WHEEL_BUCKETS `64U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_timer_wheel.h`

