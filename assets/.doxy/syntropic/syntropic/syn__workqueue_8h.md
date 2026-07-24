

# File syn\_workqueue.h



[**FileList**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_workqueue.h**](syn__workqueue_8h.md)

[Go to the source code of this file](syn__workqueue_8h_source.md)

_Deferred work queue — ISR-safe function dispatch._ [More...](#detailed-description)

* `#include "../util/syn_spsc_queue.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_WorkItem**](structSYN__WorkItem.md) <br> |
| struct | [**SYN\_WorkQueue**](structSYN__WorkQueue.md) <br> |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_WorkFunc**](#typedef-syn_workfunc)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_workqueue\_init**](#function-syn_workqueue_init) ([**SYN\_WorkQueue**](structSYN__WorkQueue.md) \* wq, [**SYN\_WorkItem**](structSYN__WorkItem.md) \* buf, size\_t capacity) <br>_Initialize the work queue._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_workqueue\_post**](#function-syn_workqueue_post) ([**SYN\_WorkQueue**](structSYN__WorkQueue.md) \* wq, [**SYN\_WorkFunc**](syn__workqueue_8h.md#typedef-syn_workfunc) func, void \* ctx) <br>_Post a work item. ISR-safe (single producer)._  |
|  size\_t | [**syn\_workqueue\_process**](#function-syn_workqueue_process) ([**SYN\_WorkQueue**](structSYN__WorkQueue.md) \* wq) <br>_Process all pending work items._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_workqueue\_empty**](#function-syn_workqueue_empty) (const [**SYN\_WorkQueue**](structSYN__WorkQueue.md) \* wq) <br>_Check if the queue is empty._  |
|  uint32\_t | [**syn\_workqueue\_overflows**](#function-syn_workqueue_overflows) (const [**SYN\_WorkQueue**](structSYN__WorkQueue.md) \* wq) <br>_Get overflow count (posts dropped)._  |
|  size\_t | [**syn\_workqueue\_pending**](#function-syn_workqueue_pending) (const [**SYN\_WorkQueue**](structSYN__WorkQueue.md) \* wq) <br>_Get the number of pending items._  |


























## Detailed Description


ISRs push work items (function + context) into a lock-free ring. The main loop drains the queue by calling [**syn\_workqueue\_process()**](syn__workqueue_8h.md#function-syn_workqueue_process).




**
**


```C++
SYN_WorkItem items[8];
SYN_WorkQueue wq;
syn_workqueue_init(&wq, items, 8);

// In ISR:
syn_workqueue_post(&wq, my_handler, my_data);

// In main loop:
syn_workqueue_process(&wq);
```
 





    
## Public Types Documentation




### typedef SYN\_WorkFunc 

```C++
typedef void(* SYN_WorkFunc) (void *ctx);
```



Work function signature. 


        

<hr>
## Public Functions Documentation




### function syn\_workqueue\_init 

_Initialize the work queue._ 
```C++
void syn_workqueue_init (
    SYN_WorkQueue * wq,
    SYN_WorkItem * buf,
    size_t capacity
) 
```





**Parameters:**


* `wq` Pointer to work queue context to initialize. 
* `buf` Backing storage array for work items. 
* `capacity` Number of elements in the buffer. 




        

<hr>



### function syn\_workqueue\_post 

_Post a work item. ISR-safe (single producer)._ 
```C++
bool syn_workqueue_post (
    SYN_WorkQueue * wq,
    SYN_WorkFunc func,
    void * ctx
) 
```





**Parameters:**


* `wq` Pointer to work queue context. 
* `func` Callback function to defer. 
* `ctx` User data passed to the callback. 



**Returns:**

true if posted, false if queue is full. 





        

<hr>



### function syn\_workqueue\_process 

_Process all pending work items._ 
```C++
size_t syn_workqueue_process (
    SYN_WorkQueue * wq
) 
```



Call from main loop. Executes each queued function in FIFO order.




**Parameters:**


* `wq` Pointer to work queue context. 



**Returns:**

Number of items processed. 





        

<hr>
## Public Static Functions Documentation




### function syn\_workqueue\_empty 

_Check if the queue is empty._ 
```C++
static inline bool syn_workqueue_empty (
    const SYN_WorkQueue * wq
) 
```





**Parameters:**


* `wq` Pointer to work queue context. 



**Returns:**

true if empty. 





        

<hr>



### function syn\_workqueue\_overflows 

_Get overflow count (posts dropped)._ 
```C++
static inline uint32_t syn_workqueue_overflows (
    const SYN_WorkQueue * wq
) 
```





**Parameters:**


* `wq` Pointer to work queue context. 



**Returns:**

Count of dropped posts. 





        

<hr>



### function syn\_workqueue\_pending 

_Get the number of pending items._ 
```C++
static inline size_t syn_workqueue_pending (
    const SYN_WorkQueue * wq
) 
```





**Parameters:**


* `wq` Pointer to work queue context. 



**Returns:**

Number of pending items. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_workqueue.h`

