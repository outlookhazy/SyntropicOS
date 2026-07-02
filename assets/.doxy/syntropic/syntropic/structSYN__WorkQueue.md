

# Struct SYN\_WorkQueue



[**ClassList**](annotated.md) **>** [**SYN\_WorkQueue**](structSYN__WorkQueue.md)



[More...](#detailed-description)

* `#include <syn_workqueue.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  size\_t | [**capacity**](#variable-capacity)  <br> |
|  volatile size\_t | [**head**](#variable-head)  <br> |
|  [**SYN\_WorkItem**](structSYN__WorkItem.md) \* | [**items**](#variable-items)  <br> |
|  uint32\_t | [**overflow**](#variable-overflow)  <br> |
|  volatile size\_t | [**tail**](#variable-tail)  <br> |












































## Detailed Description


Work queue (lock-free SPSC ring). 


    
## Public Attributes Documentation




### variable capacity 

```C++
size_t SYN_WorkQueue::capacity;
```



Maximum items the queue can hold 


        

<hr>



### variable head 

```C++
volatile size_t SYN_WorkQueue::head;
```



Written by producer (ISR) 
 


        

<hr>



### variable items 

```C++
SYN_WorkItem* SYN_WorkQueue::items;
```



Array of work item slots (caller-owned) 


        

<hr>



### variable overflow 

```C++
uint32_t SYN_WorkQueue::overflow;
```



Counter: posts dropped due to full 
 


        

<hr>



### variable tail 

```C++
volatile size_t SYN_WorkQueue::tail;
```



Written by consumer (main loop) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_workqueue.h`

