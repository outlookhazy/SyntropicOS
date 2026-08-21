

# File syn\_mutex.c



[**FileList**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_mutex.c**](syn__mutex_8c.md)

[Go to the source code of this file](syn__mutex_8c_source.md)

_Priority-inheriting recursive mutex implementation._ 

* `#include "../port/syn_port_system.h"`
* `#include "../util/syn_assert.h"`
* `#include "syn_mutex.h"`
* `#include "syn_sched.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Task**](structSYN__Task.md) \* | [**syn\_mutex\_get\_owner**](#function-syn_mutex_get_owner) (const [**SYN\_Mutex**](structSYN__Mutex.md) \* mutex) <br>_Get the current owner of the mutex._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mutex\_init**](#function-syn_mutex_init) ([**SYN\_Mutex**](structSYN__Mutex.md) \* mutex) <br>_Initialize a mutex in unlocked state._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_mutex\_is\_locked**](#function-syn_mutex_is_locked) (const [**SYN\_Mutex**](structSYN__Mutex.md) \* mutex) <br>_Check if the mutex is currently locked._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mutex\_try\_lock**](#function-syn_mutex_try_lock) ([**SYN\_Mutex**](structSYN__Mutex.md) \* mutex, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Attempt to acquire the mutex without blocking._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mutex\_unlock**](#function-syn_mutex_unlock) ([**SYN\_Mutex**](structSYN__Mutex.md) \* mutex, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Release a previously acquired mutex._  |




























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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_mutex.c`

