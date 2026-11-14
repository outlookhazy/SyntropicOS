

# Struct SYN\_Mutex



[**ClassList**](annotated.md) **>** [**SYN\_Mutex**](structSYN__Mutex.md)



_Priority-inheriting recursive mutex structure._ 

* `#include <syn_mutex.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_evicted**](#variable-has_evicted)  <br> |
|  uint16\_t | [**lock\_count**](#variable-lock_count)  <br> |
|  struct [**SYN\_Mutex**](structSYN__Mutex.md) \* | [**next\_contended**](#variable-next_contended)  <br> |
|  struct [**SYN\_Mutex**](structSYN__Mutex.md) \* | [**next\_held**](#variable-next_held)  <br> |
|  [**SYN\_Task**](structSYN__Task.md) \* | [**owner**](#variable-owner)  <br> |
|  struct [**SYN\_Sched**](structSYN__Sched.md) \* | [**wait\_sched**](#variable-wait_sched)  <br> |
|  [**SYN\_Task**](structSYN__Task.md) \* | [**waiters**](#variable-waiters)  <br> |












































## Public Attributes Documentation




### variable has\_evicted 

```C++
bool SYN_Mutex::has_evicted;
```



True if waiters[] overflowed 


        

<hr>



### variable lock\_count 

```C++
uint16_t SYN_Mutex::lock_count;
```



Recursive lock acquisition depth 


        

<hr>



### variable next\_contended 

```C++
struct SYN_Mutex* SYN_Mutex::next_contended;
```



Next mutex in wait\_sched list 


        

<hr>



### variable next\_held 

```C++
struct SYN_Mutex* SYN_Mutex::next_held;
```



Next mutex in owner lock list 


        

<hr>



### variable owner 

```C++
SYN_Task* SYN_Mutex::owner;
```



Current owner task (NULL if unlocked) 


        

<hr>



### variable wait\_sched 

```C++
struct SYN_Sched* SYN_Mutex::wait_sched;
```



Scheduler tracking external waiters 


        

<hr>



### variable waiters 

```C++
SYN_Task* SYN_Mutex::waiters[SYN_MUTEX_MAX_WAITERS];
```



Active contended waiter tasks 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_mutex.h`

