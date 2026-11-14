

# Struct SYN\_PT\_Sem



[**ClassList**](annotated.md) **>** [**SYN\_PT\_Sem**](structSYN__PT__Sem.md)



_Counting semaphore for protothreads._ [More...](#detailed-description)

* `#include <syn_pt_sem.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  volatile int16\_t | [**count**](#variable-count)  <br> |












































## Detailed Description


Costs 2 bytes of RAM. The count is signed to allow detecting underflow bugs, but normal operation keeps it &gt;= 0. 


    
## Public Attributes Documentation




### variable count 

```C++
volatile int16_t SYN_PT_Sem::count;
```



Current semaphore count (&gt;= 0 means available) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/pt/syn_pt_sem.h`

