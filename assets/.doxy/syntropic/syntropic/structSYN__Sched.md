

# Struct SYN\_Sched



[**ClassList**](annotated.md) **>** [**SYN\_Sched**](structSYN__Sched.md)



_Scheduler control block._ [More...](#detailed-description)

* `#include <syn_sched.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  size\_t | [**rr\_index**](#variable-rr_index)  <br> |
|  size\_t | [**task\_count**](#variable-task_count)  <br> |
|  [**SYN\_Task**](structSYN__Task.md) \* | [**tasks**](#variable-tasks)  <br> |












































## Detailed Description


The scheduler does not own the task array — you allocate it and pass a pointer. This means zero hidden allocation. 


    
## Public Attributes Documentation




### variable rr\_index 

```C++
size_t SYN_Sched::rr_index;
```



Round-robin index for equal priority 
 


        

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

