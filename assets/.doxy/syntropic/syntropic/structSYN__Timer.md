

# Struct SYN\_Timer



[**ClassList**](annotated.md) **>** [**SYN\_Timer**](structSYN__Timer.md)



_Software timer descriptor._ 

* `#include <syn_timer.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  [**SYN\_TimerCallback**](syn__timer_8h.md#typedef-syn_timercallback) | [**callback**](#variable-callback)  <br> |
|  uint32\_t | [**period\_ms**](#variable-period_ms)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**periodic**](#variable-periodic)  <br> |
|  uint32\_t | [**target\_tick**](#variable-target_tick)  <br> |
|  void \* | [**user\_data**](#variable-user_data)  <br> |












































## Public Attributes Documentation




### variable active 

```C++
bool SYN_Timer::active;
```



Currently running? 
 


        

<hr>



### variable callback 

```C++
SYN_TimerCallback SYN_Timer::callback;
```



Called on expiry (may be NULL) 
 


        

<hr>



### variable period\_ms 

```C++
uint32_t SYN_Timer::period_ms;
```



Timer period in milliseconds 
 


        

<hr>



### variable periodic 

```C++
bool SYN_Timer::periodic;
```



true = auto-repeat, false = one-shot 


        

<hr>



### variable target\_tick 

```C++
uint32_t SYN_Timer::target_tick;
```



Next expiry tick 
 


        

<hr>



### variable user\_data 

```C++
void* SYN_Timer::user_data;
```



User context for callback 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_timer.h`

