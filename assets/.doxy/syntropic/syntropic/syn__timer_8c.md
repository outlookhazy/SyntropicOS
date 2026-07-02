

# File syn\_timer.c



[**FileList**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_timer.c**](syn__timer_8c.md)

[Go to the source code of this file](syn__timer_8c_source.md)

_Software timer implementation._ 

* `#include "syn_timer.h"`
* `#include "../port/syn_port_system.h"`
* `#include "../util/syn_assert.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_timer\_expired**](#function-syn_timer_expired) ([**SYN\_Timer**](structSYN__Timer.md) \* timer) <br>_Poll whether the timer has expired._  |
|  void | [**syn\_timer\_init**](#function-syn_timer_init) ([**SYN\_Timer**](structSYN__Timer.md) \* timer, uint32\_t period\_ms, [**bool**](syn__defs_8h.md#enum-bool) periodic, [**SYN\_TimerCallback**](syn__timer_8h.md#typedef-syn_timercallback) callback, void \* user\_data) <br>_Initialize a software timer._  |
|  uint32\_t | [**syn\_timer\_next\_expiry**](#function-syn_timer_next_expiry) (const [**SYN\_Timer**](structSYN__Timer.md) \* timers, size\_t count) <br>_Get the earliest expiry tick among a set of active timers._  |
|  uint32\_t | [**syn\_timer\_remaining**](#function-syn_timer_remaining) (const [**SYN\_Timer**](structSYN__Timer.md) \* timer) <br>_Get the remaining time until the next expiry._  |
|  void | [**syn\_timer\_service**](#function-syn_timer_service) ([**SYN\_Timer**](structSYN__Timer.md) \* timers, size\_t count) <br>_Service an array of timers._  |
|  void | [**syn\_timer\_set\_period**](#function-syn_timer_set_period) ([**SYN\_Timer**](structSYN__Timer.md) \* timer, uint32\_t period\_ms) <br>_Change the timer's period. Takes effect on next start or next periodic reload._  |
|  void | [**syn\_timer\_start**](#function-syn_timer_start) ([**SYN\_Timer**](structSYN__Timer.md) \* timer) <br>_Start (or restart) the timer._  |
|  void | [**syn\_timer\_stop**](#function-syn_timer_stop) ([**SYN\_Timer**](structSYN__Timer.md) \* timer) <br>_Stop the timer. It will not fire until started again._  |




























## Public Functions Documentation




### function syn\_timer\_expired 

_Poll whether the timer has expired._ 
```C++
bool syn_timer_expired (
    SYN_Timer * timer
) 
```



If expired and periodic, the timer is automatically re-armed. If expired and one-shot, the timer is stopped.


This can be used instead of (or in addition to) the callback.




**Parameters:**


* `timer` Timer to check. 



**Returns:**

true if the timer has expired since the last check. 





        

<hr>



### function syn\_timer\_init 

_Initialize a software timer._ 
```C++
void syn_timer_init (
    SYN_Timer * timer,
    uint32_t period_ms,
    bool periodic,
    SYN_TimerCallback callback,
    void * user_data
) 
```



The timer is created in the stopped state. Call [**syn\_timer\_start()**](syn__timer_8h.md#function-syn_timer_start) to begin counting.




**Parameters:**


* `timer` Timer to initialize. 
* `period_ms` Period in milliseconds. 
* `periodic` true for repeating timer, false for one-shot. 
* `callback` Function called on expiry (may be NULL if using polling). 
* `user_data` User context pointer passed to callback. 




        

<hr>



### function syn\_timer\_next\_expiry 

_Get the earliest expiry tick among a set of active timers._ 
```C++
uint32_t syn_timer_next_expiry (
    const SYN_Timer * timers,
    size_t count
) 
```



Scans the timer array and returns the minimum target\_tick of any active timer whose deadline is in the future. Used by the tickless scheduler to account for software timer wakeups.




**Parameters:**


* `timers` Array of timers to scan. 
* `count` Number of timers in the array. 



**Returns:**

Earliest expiry tick, or UINT32\_MAX if no timers are active or all have already expired. 





        

<hr>



### function syn\_timer\_remaining 

_Get the remaining time until the next expiry._ 
```C++
uint32_t syn_timer_remaining (
    const SYN_Timer * timer
) 
```





**Parameters:**


* `timer` Timer to query. 



**Returns:**

Remaining milliseconds, or 0 if expired/stopped. 





        

<hr>



### function syn\_timer\_service 

_Service an array of timers._ 
```C++
void syn_timer_service (
    SYN_Timer * timers,
    size_t count
) 
```



Checks each timer for expiry and calls its callback if set. Call this from your main loop, a scheduler task, or a periodic ISR.




**Parameters:**


* `timers` Array of timers. 
* `count` Number of timers in the array. 




        

<hr>



### function syn\_timer\_set\_period 

_Change the timer's period. Takes effect on next start or next periodic reload._ 
```C++
void syn_timer_set_period (
    SYN_Timer * timer,
    uint32_t period_ms
) 
```





**Parameters:**


* `timer` Timer. 
* `period_ms` New period in milliseconds. 




        

<hr>



### function syn\_timer\_start 

_Start (or restart) the timer._ 
```C++
void syn_timer_start (
    SYN_Timer * timer
) 
```



Sets the target tick to now + period\_ms and marks the timer active.




**Parameters:**


* `timer` Timer to start. 




        

<hr>



### function syn\_timer\_stop 

_Stop the timer. It will not fire until started again._ 
```C++
void syn_timer_stop (
    SYN_Timer * timer
) 
```





**Parameters:**


* `timer` Timer to stop. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_timer.c`

