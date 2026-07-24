

# File syn\_pt\_sem.h



[**FileList**](files.md) **>** [**pt**](dir_a1b594c7ca8b98b889036d2814e4f947.md) **>** [**syn\_pt\_sem.h**](syn__pt__sem_8h.md)

[Go to the source code of this file](syn__pt__sem_8h_source.md)

_Lightweight counting semaphores for protothreads._ [More...](#detailed-description)

* `#include "syn_pt.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_PT\_Sem**](structSYN__PT__Sem.md) <br>_Counting semaphore for protothreads._  |
























## Public Static Functions

| Type | Name |
| ---: | :--- |
|  int | [**pt\_sem\_trywait**](#function-pt_sem_trywait) ([**SYN\_PT\_Sem**](structSYN__PT__Sem.md) \* sem) <br>_Try to acquire the semaphore without blocking._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**PT\_SEM\_COUNT**](syn__pt__sem_8h.md#define-pt_sem_count) (sem) `((sem)-&gt;count)`<br> |
| define  | [**PT\_SEM\_INIT**](syn__pt__sem_8h.md#define-pt_sem_init) (sem, initial) `((sem)-&gt;count = (int16\_t)(initial))`<br>_Initialize a semaphore with the given count._  |
| define  | [**PT\_SEM\_SIGNAL**](syn__pt__sem_8h.md#define-pt_sem_signal) (sem) `((sem)-&gt;count++)`<br>_Increment the semaphore count, unblocking a waiting protothread._  |
| define  | [**PT\_SEM\_TRYWAIT**](syn__pt__sem_8h.md#define-pt_sem_trywait) (sem) `[**pt\_sem\_trywait**](syn__pt__sem_8h.md#function-pt_sem_trywait)(sem)`<br>_Macro wrapper for pt\_sem\_trywait._  |
| define  | [**PT\_SEM\_WAIT**](syn__pt__sem_8h.md#define-pt_sem_wait) (pt, sem) `/* multi line expression */`<br>_Block the protothread until the semaphore count is &gt; 0, then decrement it._  |

## Detailed Description


Provides inter-protothread synchronization with zero RAM overhead beyond the 2-byte counter itself. All operations are macro/inline — no .c file is needed.




**
**


```C++
static SYN_PT_Sem data_ready;
PT_SEM_INIT(&data_ready, 0);

// Producer (can be called from ISR):
PT_SEM_SIGNAL(&data_ready);

// Consumer (inside a protothread):
PT_SEM_WAIT(pt, &data_ready);
// ... data is now available ...
```
 





    
## Public Static Functions Documentation




### function pt\_sem\_trywait 

_Try to acquire the semaphore without blocking._ 
```C++
static inline int pt_sem_trywait (
    SYN_PT_Sem * sem
) 
```





**Parameters:**


* `sem` Semaphore to try. 



**Returns:**

1 if acquired (count was &gt; 0 and was decremented), 0 otherwise. 





        

<hr>
## Macro Definition Documentation





### define PT\_SEM\_COUNT 

```C++
#define PT_SEM_COUNT (
    sem
) `((sem)->count)`
```



Return the current count. 


        

<hr>



### define PT\_SEM\_INIT 

_Initialize a semaphore with the given count._ 
```C++
#define PT_SEM_INIT (
    sem,
    initial
) `((sem)->count = (int16_t)(initial))`
```





**Parameters:**


* `sem` Pointer to the semaphore. 
* `initial` Initial count (typically 0 or 1). 




        

<hr>



### define PT\_SEM\_SIGNAL 

_Increment the semaphore count, unblocking a waiting protothread._ 
```C++
#define PT_SEM_SIGNAL (
    sem
) `((sem)->count++)`
```



Safe to call from ISR context.




**Parameters:**


* `sem` Semaphore to signal. 




        

<hr>



### define PT\_SEM\_TRYWAIT 

_Macro wrapper for pt\_sem\_trywait._ 
```C++
#define PT_SEM_TRYWAIT (
    sem
) `pt_sem_trywait (sem)`
```




<hr>



### define PT\_SEM\_WAIT 

_Block the protothread until the semaphore count is &gt; 0, then decrement it._ 
```C++
#define PT_SEM_WAIT (
    pt,
    sem
) `/* multi line expression */`
```



Must be called from within a protothread (between PT\_BEGIN/PT\_END).




**Parameters:**


* `pt` Protothread. 
* `sem` Semaphore to wait on. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/pt/syn_pt_sem.h`

