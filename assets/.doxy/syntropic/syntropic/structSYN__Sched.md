

# Struct SYN\_Sched



[**ClassList**](annotated.md) **>** [**SYN\_Sched**](structSYN__Sched.md)



_Scheduler control block._ [More...](#detailed-description)

* `#include <syn_sched.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**max\_wait\_ms**](#variable-max_wait_ms)  <br> |
|  struct [**SYN\_TaskProfileManager**](structSYN__TaskProfileManager.md) \* | [**profiler**](#variable-profiler)  <br> |
|  uint16\_t | [**rr\_per\_prio**](#variable-rr_per_prio)  <br> |
|  [**SYN\_Task**](structSYN__Task.md) \* | [**running**](#variable-running)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**share\_contended**](#variable-share_contended)  <br> |
|  uint32\_t | [**share\_deadline\_ms**](#variable-share_deadline_ms)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**share\_has\_deadline**](#variable-share_has_deadline)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**share\_watch\_wakeups**](#variable-share_watch_wakeups)  <br> |
|  struct [**SYN\_Mutex**](structSYN__Mutex.md) \* | [**standalone\_mutexes**](#variable-standalone_mutexes)  <br> |
|  size\_t | [**task\_count**](#variable-task_count)  <br> |
|  [**SYN\_Task**](structSYN__Task.md) \* | [**tasks**](#variable-tasks)  <br> |












































## Detailed Description


The scheduler does not own the task array — you allocate it and pass a pointer. This means zero hidden allocation. 


    
## Public Attributes Documentation




### variable max\_wait\_ms 

```C++
uint16_t SYN_Sched::max_wait_ms[SYN_SCHED_PRIO_LEVELS];
```



Per-priority starvation bounds (ms) 


        

<hr>



### variable profiler 

```C++
struct SYN_TaskProfileManager* SYN_Sched::profiler;
```



Optional attached task profiler 


        

<hr>



### variable rr\_per\_prio 

```C++
uint16_t SYN_Sched::rr_per_prio[SYN_SCHED_PRIO_LEVELS];
```



Per-priority round-robin indices 


        

<hr>



### variable running 

```C++
SYN_Task* SYN_Sched::running;
```



Task currently executing (or NULL) 


        

<hr>



### variable share\_contended 

```C++
bool SYN_Sched::share_contended;
```



Equal/higher-prio MORE peer waiting 


        

<hr>



### variable share\_deadline\_ms 

```C++
uint32_t SYN_Sched::share_deadline_ms;
```



Earliest peer due\_ms during dispatch 


        

<hr>



### variable share\_has\_deadline 

```C++
bool SYN_Sched::share_has_deadline;
```



True when share\_deadline\_ms is valid 


        

<hr>



### variable share\_watch\_wakeups 

```C++
bool SYN_Sched::share_watch_wakeups;
```



Watch blocked/suspended urgent peers 


        

<hr>



### variable standalone\_mutexes 

```C++
struct SYN_Mutex* SYN_Sched::standalone_mutexes;
```



Locked ownerless mutexes with waiters 


        

<hr>



### variable task\_count 

```C++
size_t SYN_Sched::task_count;
```



Number of tasks in the array 


        

<hr>



### variable tasks 

```C++
SYN_Task* SYN_Sched::tasks;
```



Pointer to caller-owned task array 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_sched.h`

