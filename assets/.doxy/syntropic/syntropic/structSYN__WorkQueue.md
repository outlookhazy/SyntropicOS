

# Struct SYN\_WorkQueue



[**ClassList**](annotated.md) **>** [**SYN\_WorkQueue**](structSYN__WorkQueue.md)



[More...](#detailed-description)

* `#include <syn_workqueue.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**overflow**](#variable-overflow)  <br> |
|  [**SYN\_SPSC\_Queue**](structSYN__SPSC__Queue.md) | [**queue**](#variable-queue)  <br> |












































## Detailed Description


Work queue (lock-free SPSC ring). 


    
## Public Attributes Documentation




### variable overflow 

```C++
uint32_t SYN_WorkQueue::overflow;
```



Counter: posts dropped due to full 


        

<hr>



### variable queue 

```C++
SYN_SPSC_Queue SYN_WorkQueue::queue;
```



Embedded lock-free SPSC queue 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_workqueue.h`

