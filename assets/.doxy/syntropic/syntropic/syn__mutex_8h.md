

# File syn\_mutex.h



[**FileList**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_mutex.h**](syn__mutex_8h.md)

[Go to the source code of this file](syn__mutex_8h_source.md)

_Priority-inheriting mutual exclusion primitive for tasks and protothreads._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../pt/syn_pt.h"`
* `#include "syn_task.h"`
* `#include <stdbool.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Mutex**](structSYN__Mutex.md) <br>_Priority-inheriting recursive mutex structure._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Task**](structSYN__Task.md) \* | [**syn\_mutex\_get\_owner**](#function-syn_mutex_get_owner) (const [**SYN\_Mutex**](structSYN__Mutex.md) \* mutex) <br>_Get the current owner of the mutex._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mutex\_init**](#function-syn_mutex_init) ([**SYN\_Mutex**](structSYN__Mutex.md) \* mutex) <br>_Initialize a mutex in unlocked state._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_mutex\_is\_locked**](#function-syn_mutex_is_locked) (const [**SYN\_Mutex**](structSYN__Mutex.md) \* mutex) <br>_Check if the mutex is currently locked._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mutex\_try\_lock**](#function-syn_mutex_try_lock) ([**SYN\_Mutex**](structSYN__Mutex.md) \* mutex, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Attempt to acquire the mutex without blocking._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mutex\_unlock**](#function-syn_mutex_unlock) ([**SYN\_Mutex**](structSYN__Mutex.md) \* mutex, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Release a previously acquired mutex._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**PT\_MUTEX\_INIT**](syn__mutex_8h.md#define-pt_mutex_init) (mutex) `[**syn\_mutex\_init**](syn__mutex_8c.md#function-syn_mutex_init)(mutex)`<br>_Initialize a mutex._  |
| define  | [**PT\_MUTEX\_LOCK**](syn__mutex_8h.md#define-pt_mutex_lock) (pt, task, mutex) `[**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until)(pt, [**syn\_mutex\_try\_lock**](syn__mutex_8c.md#function-syn_mutex_try_lock)((mutex), (task)) == SYN\_OK)`<br>_Block the protothread until the mutex is acquired._  |
| define  | [**PT\_MUTEX\_TRYLOCK**](syn__mutex_8h.md#define-pt_mutex_trylock) (mutex, task) `([**syn\_mutex\_try\_lock**](syn__mutex_8c.md#function-syn_mutex_try_lock)((mutex), (task)) == SYN\_OK)`<br>_Non-blocking try-lock macro for protothreads._  |
| define  | [**PT\_MUTEX\_UNLOCK**](syn__mutex_8h.md#define-pt_mutex_unlock) (mutex, task) `[**syn\_mutex\_unlock**](syn__mutex_8c.md#function-syn_mutex_unlock)((mutex), (task))`<br>_Unlock a mutex held by the task._  |

## Detailed Description


Provides recursive mutual exclusion with priority inheritance to prevent unbounded priority inversion. If a higher-priority task attempts to acquire a mutex held by a lower-priority task, the owner's priority is temporarily boosted to match the waiting task until the mutex is released. 


    
## Public Functions Documentation




### function syn\_mutex\_get\_owner 

_Get the current owner of the mutex._ 
```C++
SYN_Task * syn_mutex_get_owner (
    const SYN_Mutex * mutex
) 
```





**Parameters:**


* `mutex` Pointer to mutex structure. 



**Returns:**

Pointer to owner task, or NULL if unlocked. 





        

<hr>



### function syn\_mutex\_init 

_Initialize a mutex in unlocked state._ 
```C++
SYN_Status syn_mutex_init (
    SYN_Mutex * mutex
) 
```





**Parameters:**


* `mutex` Pointer to mutex structure. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM if mutex is NULL. 





        

<hr>



### function syn\_mutex\_is\_locked 

_Check if the mutex is currently locked._ 
```C++
bool syn_mutex_is_locked (
    const SYN_Mutex * mutex
) 
```





**Parameters:**


* `mutex` Pointer to mutex structure. 



**Returns:**

true if locked, false otherwise. 





        

<hr>



### function syn\_mutex\_try\_lock 

_Attempt to acquire the mutex without blocking._ 
```C++
SYN_Status syn_mutex_try_lock (
    SYN_Mutex * mutex,
    SYN_Task * task
) 
```



If the mutex is already locked by another task and `task` has higher priority (numerically lower value) than the owner, the owner's priority is automatically boosted.




**Parameters:**


* `mutex` Pointer to mutex structure. 
* `task` Pointer to acquiring task (optional, NULL for standalone lock). 



**Returns:**

SYN\_OK if acquired, SYN\_BUSY if held by another task, or SYN\_INVALID\_PARAM. 





        

<hr>



### function syn\_mutex\_unlock 

_Release a previously acquired mutex._ 
```C++
SYN_Status syn_mutex_unlock (
    SYN_Mutex * mutex,
    SYN_Task * task
) 
```



Decrements the recursion count. When the count reaches zero, the owner's priority is restored to its base priority and ownership is cleared.




**Parameters:**


* `mutex` Pointer to mutex structure. 
* `task` Pointer to owner task releasing the lock. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if not locked or wrong owner, or SYN\_INVALID\_PARAM. 





        

<hr>
## Macro Definition Documentation





### define PT\_MUTEX\_INIT 

_Initialize a mutex._ 
```C++
#define PT_MUTEX_INIT (
    mutex
) `syn_mutex_init (mutex)`
```




<hr>



### define PT\_MUTEX\_LOCK 

_Block the protothread until the mutex is acquired._ 
```C++
#define PT_MUTEX_LOCK (
    pt,
    task,
    mutex
) `PT_WAIT_UNTIL (pt, syn_mutex_try_lock ((mutex), (task)) == SYN_OK )`
```



Cooperatively waits via PT\_WAIT\_UNTIL. On contention, automatically boosts the holding task's priority to prevent unbounded priority inversion.




**Parameters:**


* `pt` Protothread control block. 
* `task` Pointer to calling [**SYN\_Task**](structSYN__Task.md). 
* `mutex` Pointer to [**SYN\_Mutex**](structSYN__Mutex.md). 




        

<hr>



### define PT\_MUTEX\_TRYLOCK 

_Non-blocking try-lock macro for protothreads._ 
```C++
#define PT_MUTEX_TRYLOCK (
    mutex,
    task
) `( syn_mutex_try_lock ((mutex), (task)) == SYN_OK )`
```





**Parameters:**


* `mutex` Pointer to [**SYN\_Mutex**](structSYN__Mutex.md). 
* `task` Pointer to calling [**SYN\_Task**](structSYN__Task.md). 



**Returns:**

true if acquired, false if busy. 





        

<hr>



### define PT\_MUTEX\_UNLOCK 

_Unlock a mutex held by the task._ 
```C++
#define PT_MUTEX_UNLOCK (
    mutex,
    task
) `syn_mutex_unlock ((mutex), (task))`
```





**Parameters:**


* `mutex` Pointer to [**SYN\_Mutex**](structSYN__Mutex.md). 
* `task` Pointer to calling [**SYN\_Task**](structSYN__Task.md). 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_mutex.h`

