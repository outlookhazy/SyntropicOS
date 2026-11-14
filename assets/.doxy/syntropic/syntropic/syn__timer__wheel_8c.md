

# File syn\_timer\_wheel.c



[**FileList**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_timer\_wheel.c**](syn__timer__wheel_8c.md)

[Go to the source code of this file](syn__timer__wheel_8c_source.md)

_Hashed Timing Wheel Scheduler implementation._ 

* `#include "syn_timer_wheel.h"`
* `#include "../port/syn_port_spinlock.h"`
* `#include "../port/syn_port_system.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_timer\_wheel\_add**](#function-syn_timer_wheel_add) ([**SYN\_TimerWheel**](structSYN__TimerWheel.md) \* wheel, [**SYN\_TimerWheelNode**](structSYN__TimerWheelNode.md) \* node, uint32\_t delay\_ticks, [**SYN\_TimerWheelCb**](syn__timer__wheel_8h.md#typedef-syn_timerwheelcb) cb, void \* arg) <br>_Schedule a timer in the timing wheel._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_timer\_wheel\_cancel**](#function-syn_timer_wheel_cancel) ([**SYN\_TimerWheel**](structSYN__TimerWheel.md) \* wheel, [**SYN\_TimerWheelNode**](structSYN__TimerWheelNode.md) \* node) <br>_Cancel a scheduled timer node._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_timer\_wheel\_init**](#function-syn_timer_wheel_init) ([**SYN\_TimerWheel**](structSYN__TimerWheel.md) \* wheel) <br>_Initialize a timing wheel scheduler._  |
|  size\_t | [**syn\_timer\_wheel\_step**](#function-syn_timer_wheel_step) ([**SYN\_TimerWheel**](structSYN__TimerWheel.md) \* wheel) <br>_Advance the timing wheel by one tick and fire expired timers._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**timer\_wheel\_partition\_bucket**](#function-timer_wheel_partition_bucket) ([**SYN\_TimerWheel**](structSYN__TimerWheel.md) \* wheel, uint32\_t bucket\_idx, [**SYN\_TimerWheelNode**](structSYN__TimerWheelNode.md) \*\* out\_exp\_head) <br>_Partition a bucket into still-rotating nodes and ready-to-fire nodes._  |


























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
## Public Static Functions Documentation




### function timer\_wheel\_partition\_bucket 

_Partition a bucket into still-rotating nodes and ready-to-fire nodes._ 
```C++
static void timer_wheel_partition_bucket (
    SYN_TimerWheel * wheel,
    uint32_t bucket_idx,
    SYN_TimerWheelNode ** out_exp_head
) 
```





**Parameters:**


* `wheel` Timer wheel instance. 
* `bucket_idx` Index of bucket to partition. 
* `out_exp_head` Output pointer receiving head of expired nodes list. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_timer_wheel.c`

