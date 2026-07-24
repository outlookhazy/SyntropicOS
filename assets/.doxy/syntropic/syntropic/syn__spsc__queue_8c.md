

# File syn\_spsc\_queue.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_spsc\_queue.c**](syn__spsc__queue_8c.md)

[Go to the source code of this file](syn__spsc__queue_8c_source.md)

_Lock-free Single-Producer Single-Consumer (SPSC) queue implementation._ 

* `#include "syn_spsc_queue.h"`
* `#include "../common/syn_barrier.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_spsc\_queue\_count**](#function-syn_spsc_queue_count) (const [**SYN\_SPSC\_Queue**](structSYN__SPSC__Queue.md) \* q) <br>_Get current number of items stored in queue._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_spsc\_queue\_init**](#function-syn_spsc_queue_init) ([**SYN\_SPSC\_Queue**](structSYN__SPSC__Queue.md) \* q, void \* elem\_buf, size\_t elem\_size, size\_t capacity) <br>_Initialize a Lock-Free SPSC Queue._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_spsc\_queue\_is\_empty**](#function-syn_spsc_queue_is_empty) (const [**SYN\_SPSC\_Queue**](structSYN__SPSC__Queue.md) \* q) <br>_Check if the queue is empty._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_spsc\_queue\_is\_full**](#function-syn_spsc_queue_is_full) (const [**SYN\_SPSC\_Queue**](structSYN__SPSC__Queue.md) \* q) <br>_Check if the queue is full._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_spsc\_queue\_pop**](#function-syn_spsc_queue_pop) ([**SYN\_SPSC\_Queue**](structSYN__SPSC__Queue.md) \* q, void \* out\_item) <br>_Pop an item from the queue (Consumer only)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_spsc\_queue\_push**](#function-syn_spsc_queue_push) ([**SYN\_SPSC\_Queue**](structSYN__SPSC__Queue.md) \* q, const void \* item) <br>_Push an item into the queue (Producer only)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**spsc\_advance**](#function-spsc_advance) (size\_t idx, size\_t capacity) <br> |


























## Public Functions Documentation




### function syn\_spsc\_queue\_count 

_Get current number of items stored in queue._ 
```C++
size_t syn_spsc_queue_count (
    const SYN_SPSC_Queue * q
) 
```





**Parameters:**


* `q` Queue context. 



**Returns:**

Count of items in queue. 





        

<hr>



### function syn\_spsc\_queue\_init 

_Initialize a Lock-Free SPSC Queue._ 
```C++
SYN_Status syn_spsc_queue_init (
    SYN_SPSC_Queue * q,
    void * elem_buf,
    size_t elem_size,
    size_t capacity
) 
```





**Parameters:**


* `q` Pointer to queue control structure. 
* `elem_buf` Pointer to caller-provided backing memory buffer. 
* `elem_size` Size of each element in bytes (must be &gt; 0). 
* `capacity` Number of elements buffer can hold (must be &gt; 0). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_spsc\_queue\_is\_empty 

_Check if the queue is empty._ 
```C++
bool syn_spsc_queue_is_empty (
    const SYN_SPSC_Queue * q
) 
```





**Parameters:**


* `q` Queue context. 



**Returns:**

true if empty. 





        

<hr>



### function syn\_spsc\_queue\_is\_full 

_Check if the queue is full._ 
```C++
bool syn_spsc_queue_is_full (
    const SYN_SPSC_Queue * q
) 
```





**Parameters:**


* `q` Queue context. 



**Returns:**

true if full. 





        

<hr>



### function syn\_spsc\_queue\_pop 

_Pop an item from the queue (Consumer only)._ 
```C++
SYN_Status syn_spsc_queue_pop (
    SYN_SPSC_Queue * q,
    void * out_item
) 
```





**Parameters:**


* `q` Queue context. 
* `out_item` Pointer to memory receiving popped item. 



**Returns:**

SYN\_OK on success, or SYN\_ERROR if empty. 





        

<hr>



### function syn\_spsc\_queue\_push 

_Push an item into the queue (Producer only)._ 
```C++
SYN_Status syn_spsc_queue_push (
    SYN_SPSC_Queue * q,
    const void * item
) 
```





**Parameters:**


* `q` Queue context. 
* `item` Pointer to item to copy into queue. 



**Returns:**

SYN\_OK on success, or SYN\_BUSY if full. 





        

<hr>
## Public Static Functions Documentation




### function spsc\_advance 

```C++
static inline size_t spsc_advance (
    size_t idx,
    size_t capacity
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_spsc_queue.c`

