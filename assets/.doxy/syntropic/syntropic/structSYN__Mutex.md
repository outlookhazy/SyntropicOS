

# Struct SYN\_Mutex



[**ClassList**](annotated.md) **>** [**SYN\_Mutex**](structSYN__Mutex.md)



_Priority-inheriting recursive mutex structure._ 

* `#include <syn_mutex.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**lock\_count**](#variable-lock_count)  <br> |
|  uint8\_t | [**original\_prio**](#variable-original_prio)  <br> |
|  [**SYN\_Task**](structSYN__Task.md) \* | [**owner**](#variable-owner)  <br> |












































## Public Attributes Documentation




### variable lock\_count 

```C++
uint16_t SYN_Mutex::lock_count;
```



Recursive lock acquisition depth 
 


        

<hr>



### variable original\_prio 

```C++
uint8_t SYN_Mutex::original_prio;
```



Base priority of owner before mutex boost 
 


        

<hr>



### variable owner 

```C++
SYN_Task* SYN_Mutex::owner;
```



Pointer to current owner task (NULL if unlocked) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_mutex.h`

