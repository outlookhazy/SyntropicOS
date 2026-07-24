

# File syn\_mailbox.h



[**FileList**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_mailbox.h**](syn__mailbox_8h.md)

[Go to the source code of this file](syn__mailbox_8h_source.md)

_Typed message queue — fixed-size SPSC ring for inter-task IPC._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`
* `#include <string.h>`
* `#include "../common/syn_barrier.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Mailbox**](structSYN__Mailbox.md) <br>_Typed message queue — fixed-size SPSC ring buffer._  |
























## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_mailbox\_empty**](#function-syn_mailbox_empty) (const [**SYN\_Mailbox**](structSYN__Mailbox.md) \* mb) <br>_Check if the mailbox is empty._  |
|  void | [**syn\_mailbox\_flush**](#function-syn_mailbox_flush) ([**SYN\_Mailbox**](structSYN__Mailbox.md) \* mb) <br>_Flush all messages._  |
|  size\_t | [**syn\_mailbox\_free**](#function-syn_mailbox_free) (const [**SYN\_Mailbox**](structSYN__Mailbox.md) \* mb) <br>_Number of free slots._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_mailbox\_full**](#function-syn_mailbox_full) (const [**SYN\_Mailbox**](structSYN__Mailbox.md) \* mb) <br>_Check if the mailbox is full._  |
|  void | [**syn\_mailbox\_init**](#function-syn_mailbox_init) ([**SYN\_Mailbox**](structSYN__Mailbox.md) \* mb, void \* buf, size\_t msg\_size, size\_t capacity) <br>_Initialize a mailbox at runtime._  |
|  uint32\_t | [**syn\_mailbox\_overflows**](#function-syn_mailbox_overflows) (const [**SYN\_Mailbox**](structSYN__Mailbox.md) \* mb) <br>_Get overflow count._  |
|  const void \* | [**syn\_mailbox\_peek**](#function-syn_mailbox_peek) (const [**SYN\_Mailbox**](structSYN__Mailbox.md) \* mb) <br>_Peek at the next message without consuming it._  |
|  size\_t | [**syn\_mailbox\_pending**](#function-syn_mailbox_pending) (const [**SYN\_Mailbox**](structSYN__Mailbox.md) \* mb) <br>_Number of messages currently in the mailbox._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_mailbox\_post**](#function-syn_mailbox_post) ([**SYN\_Mailbox**](structSYN__Mailbox.md) \* mb, const void \* msg) <br>_Post a message. ISR-safe (single producer)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_mailbox\_receive**](#function-syn_mailbox_receive) ([**SYN\_Mailbox**](structSYN__Mailbox.md) \* mb, void \* msg) <br>_Receive a message. Single consumer._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_MAILBOX\_DEFINE**](syn__mailbox_8h.md#define-syn_mailbox_define) (name, type, count) `/* multi line expression */`<br>_Static mailbox definition macro._  |

## Detailed Description


Passes fixed-size messages between producer (ISR or task) and consumer (task). Lock-free single-producer single-consumer design.


When SYN\_USE\_MULTICORE is enabled, acquire/release memory barriers are inserted at every index access, making the mailbox safe for cross-core communication (core A posts, core B receives). When multicore is disabled (the default), these compile to plain volatile reads and writes — zero overhead.




**
**




The mailbox has exactly two indices:
* `head` — written ONLY by the producer
* `tail` — written ONLY by the consumer




**Producer `syn_mailbox_post()`:**
* Read `tail` (LOAD\_ACQUIRE) — observe consumer's latest progress
* If full, return false
* memcpy message data into `buf[head * msg_size]`
* Write `head = next` (STORE\_RELEASE) — data committed before index




**Consumer `syn_mailbox_receive()`:**
* Read `head` (LOAD\_ACQUIRE) — observe producer's latest index
* If empty, return false
* memcpy message from `buf[tail * msg_size]` — safe: data visible
* Write `tail = next` (STORE\_RELEASE) — slot freed for producer






**
**


```C++
typedef struct { uint16_t id; int32_t value; } SensorMsg;

// Static allocation
SYN_MAILBOX_DEFINE(sensor_mbox, SensorMsg, 8);

// Producer (ISR or other core):
SensorMsg msg = { .id = 1, .value = 2345 };
syn_mailbox_post(&sensor_mbox, &msg);

// Consumer (main loop or other core):
SensorMsg rx;
while (syn_mailbox_receive(&sensor_mbox, &rx)) {
    process(rx.id, rx.value);
}
```
 





    
## Public Static Functions Documentation




### function syn\_mailbox\_empty 

_Check if the mailbox is empty._ 
```C++
static inline bool syn_mailbox_empty (
    const SYN_Mailbox * mb
) 
```





**Parameters:**


* `mb` Mailbox. 



**Returns:**

true if empty. 





        

<hr>



### function syn\_mailbox\_flush 

_Flush all messages._ 
```C++
static inline void syn_mailbox_flush (
    SYN_Mailbox * mb
) 
```





**Parameters:**


* `mb` Mailbox. 




        

<hr>



### function syn\_mailbox\_free 

_Number of free slots._ 
```C++
static inline size_t syn_mailbox_free (
    const SYN_Mailbox * mb
) 
```





**Parameters:**


* `mb` Mailbox. 



**Returns:**

Free slot count. 





        

<hr>



### function syn\_mailbox\_full 

_Check if the mailbox is full._ 
```C++
static inline bool syn_mailbox_full (
    const SYN_Mailbox * mb
) 
```





**Parameters:**


* `mb` Mailbox. 



**Returns:**

true if full. 





        

<hr>



### function syn\_mailbox\_init 

_Initialize a mailbox at runtime._ 
```C++
static inline void syn_mailbox_init (
    SYN_Mailbox * mb,
    void * buf,
    size_t msg_size,
    size_t capacity
) 
```





**Parameters:**


* `mb` Mailbox instance. 
* `buf` Message storage buffer. 
* `msg_size` Size of one message in bytes. 
* `capacity` Maximum number of messages. 




        

<hr>



### function syn\_mailbox\_overflows 

_Get overflow count._ 
```C++
static inline uint32_t syn_mailbox_overflows (
    const SYN_Mailbox * mb
) 
```





**Parameters:**


* `mb` Mailbox. 



**Returns:**

Number of dropped messages. 





        

<hr>



### function syn\_mailbox\_peek 

_Peek at the next message without consuming it._ 
```C++
static inline const void * syn_mailbox_peek (
    const SYN_Mailbox * mb
) 
```





**Parameters:**


* `mb` Mailbox. 



**Returns:**

Pointer to the message, or NULL if empty. Valid until the next receive(). 





        

<hr>



### function syn\_mailbox\_pending 

_Number of messages currently in the mailbox._ 
```C++
static inline size_t syn_mailbox_pending (
    const SYN_Mailbox * mb
) 
```





**Parameters:**


* `mb` Mailbox. 



**Returns:**

Message count. 





        

<hr>



### function syn\_mailbox\_post 

_Post a message. ISR-safe (single producer)._ 
```C++
static inline bool syn_mailbox_post (
    SYN_Mailbox * mb,
    const void * msg
) 
```



Uses STORE\_RELEASE on the head update to ensure message data is committed before the consumer can see the new index.




**Parameters:**


* `mb` Mailbox. 
* `msg` Pointer to message data (msg\_size bytes copied). 



**Returns:**

true if posted, false if full. 





        

<hr>



### function syn\_mailbox\_receive 

_Receive a message. Single consumer._ 
```C++
static inline bool syn_mailbox_receive (
    SYN_Mailbox * mb,
    void * msg
) 
```



Uses LOAD\_ACQUIRE on the head read to ensure all message data written by the producer is visible. Uses STORE\_RELEASE on the tail update to ensure the producer sees the freed slot.




**Parameters:**


* `mb` Mailbox. 
* `msg` Pointer to receive buffer (msg\_size bytes copied out). 



**Returns:**

true if a message was received, false if empty. 





        

<hr>
## Macro Definition Documentation





### define SYN\_MAILBOX\_DEFINE 

_Static mailbox definition macro._ 
```C++
#define SYN_MAILBOX_DEFINE (
    name,
    type,
    count
) `/* multi line expression */`
```



Creates a mailbox instance with embedded storage. 

**Parameters:**


* `name` Variable name. 
* `type` Message struct type. 
* `count` Max number of messages. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_mailbox.h`

