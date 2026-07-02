

# File syn\_workqueue.c



[**FileList**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_workqueue.c**](syn__workqueue_8c.md)

[Go to the source code of this file](syn__workqueue_8c_source.md)

_Deferred work queue implementation._ 

* `#include "syn_workqueue.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_workqueue\_init**](#function-syn_workqueue_init) ([**SYN\_WorkQueue**](structSYN__WorkQueue.md) \* wq, [**SYN\_WorkItem**](structSYN__WorkItem.md) \* buf, size\_t capacity) <br>_Initialize the work queue._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_workqueue\_post**](#function-syn_workqueue_post) ([**SYN\_WorkQueue**](structSYN__WorkQueue.md) \* wq, [**SYN\_WorkFunc**](syn__workqueue_8h.md#typedef-syn_workfunc) func, void \* ctx) <br>_Post a work item. ISR-safe (single producer)._  |
|  size\_t | [**syn\_workqueue\_process**](#function-syn_workqueue_process) ([**SYN\_WorkQueue**](structSYN__WorkQueue.md) \* wq) <br>_Process all pending work items._  |




























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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_workqueue.c`

