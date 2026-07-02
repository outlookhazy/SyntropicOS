

# File syn\_pt.h



[**FileList**](files.md) **>** [**pt**](dir_a1b594c7ca8b98b889036d2814e4f947.md) **>** [**syn\_pt.h**](syn__pt_8h.md)

[Go to the source code of this file](syn__pt_8h_source.md)

_Protothreads — stackless cooperative coroutines for C._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../common/syn_compiler.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_PT**](structSYN__PT.md) <br>_Protothread control block._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_PT\_Status**](#enum-syn_pt_status)  <br>_Return value from a protothread function._  |















































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**PT\_BEGIN**](syn__pt_8h.md#define-pt_begin) (pt) `/* multi line expression */`<br>_Open a protothread body. Must be the first statement in the protothread function, before any PT\_\* operations._  |
| define  | [**PT\_DELAY\_MS**](syn__pt_8h.md#define-pt_delay_ms) (pt, target, ms) `/* multi line expression */`<br>_Non-blocking delay for_ `ms` _milliseconds._ |
| define  | [**PT\_END**](syn__pt_8h.md#define-pt_end) (pt) `/* multi line expression */`<br>_Close a protothread body. Returns PT\_EXITED and resets the continuation so the thread can be restarted if desired._  |
| define  | [**PT\_EXIT**](syn__pt_8h.md#define-pt_exit) (pt) `/* multi line expression */`<br>_Terminate the protothread immediately._  |
| define  | [**PT\_INIT**](syn__pt_8h.md#define-pt_init) (pt) `((pt)-&gt;lc = 0)`<br>_Initialize (or reset) a protothread so it starts from the top._  |
| define  | [**PT\_IS\_IDLE**](syn__pt_8h.md#define-pt_is_idle) (pt) `((pt)-&gt;lc == 0)`<br> |
| define  | [**PT\_IS\_RUNNING**](syn__pt_8h.md#define-pt_is_running) (pt) `((pt)-&gt;lc != 0)`<br> |
| define  | [**PT\_RESTART**](syn__pt_8h.md#define-pt_restart) (pt) `/* multi line expression */`<br>_Reset the protothread and restart from PT\_BEGIN on next call._  |
| define  | [**PT\_SPAWN**](syn__pt_8h.md#define-pt_spawn) (pt, child, func) `/* multi line expression */`<br>_Spawn a child protothread and block until it exits._  |
| define  | [**PT\_TASK\_DELAY\_MS**](syn__pt_8h.md#define-pt_task_delay_ms) (pt, task, ms) `[**PT\_DELAY\_MS**](syn__pt_8h.md#define-pt_delay_ms)(pt, &(task)-&gt;delay\_until, ms)`<br>_Convenience form for use with the scheduler's_ [_**SYN\_Task**_](structSYN__Task.md) _._ |
| define  | [**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until) (pt, cond) `/* multi line expression */`<br>_Block until_ `cond` _evaluates to true._ |
| define  | [**PT\_WAIT\_WHILE**](syn__pt_8h.md#define-pt_wait_while) (pt, cond) `[**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until)(pt, !(cond))`<br>_Block while_ `cond` _is true. Dual of PT\_WAIT\_UNTIL._ |
| define  | [**PT\_YIELD**](syn__pt_8h.md#define-pt_yield) (pt) `/* multi line expression */`<br>_Yield control unconditionally. The protothread will resume at this point on the next call._  |
| define  | [**PT\_YIELD\_UNTIL**](syn__pt_8h.md#define-pt_yield_until) (pt, cond) `/* multi line expression */`<br>_Yield while_ `cond` _is true._ |

## Detailed Description


Protothreads provide lightweight, cooperative multitasking without requiring a separate stack per thread. Each protothread is a normal C function that uses macros to save and restore its execution position via the switch/\_\_LINE\_\_ continuation trick (Duff's device).


A protothread costs **2 bytes of RAM** (the continuation variable).




**
**


* You cannot use a bare `switch` statement inside a protothread body (use `if`/`else if` chains instead).
* Local variables are NOT preserved across yield/wait points. Store persistent state in `static` variables, globals, or a struct passed via the task's `user_data` pointer.
* A protothread must reside entirely within a single function.






**
**


```C++
static SYN_PT pt;
PT_INIT(&pt);

while (1) {
    my_protothread(&pt);
    // ... poll other protothreads ...
}
```





**
**


```C++
// See syn_sched.h for the full pattern.
static SYN_PT_Status my_task(SYN_PT *pt, SYN_Task *task) {
    PT_BEGIN(pt);
    // ...
    PT_END(pt);
}
```
 





    
## Public Types Documentation




### enum SYN\_PT\_Status 

_Return value from a protothread function._ 
```C++
enum SYN_PT_Status {
    PT_WAITING = 0,
    PT_YIELDED = 1,
    PT_EXITED = 2,
    PT_ENDED = 3
};
```




<hr>
## Macro Definition Documentation





### define PT\_BEGIN 

_Open a protothread body. Must be the first statement in the protothread function, before any PT\_\* operations._ 
```C++
#define PT_BEGIN (
    pt
) `/* multi line expression */`
```



Expands to the head of a switch statement that jumps to the last saved continuation point. 


        

<hr>



### define PT\_DELAY\_MS 

_Non-blocking delay for_ `ms` _milliseconds._
```C++
#define PT_DELAY_MS (
    pt,
    target,
    ms
) `/* multi line expression */`
```



Requires a `uint32_t` variable to store the deadline. When used with the scheduler, pass `&(task)->delay_until`. For standalone use, pass any persistent `uint32_t *`.




**Parameters:**


* `pt` Protothread. 
* `target` Pointer to a uint32\_t that will hold the deadline tick. 
* `ms` Delay duration in milliseconds. 




        

<hr>



### define PT\_END 

_Close a protothread body. Returns PT\_EXITED and resets the continuation so the thread can be restarted if desired._ 
```C++
#define PT_END (
    pt
) `/* multi line expression */`
```




<hr>



### define PT\_EXIT 

_Terminate the protothread immediately._ 
```C++
#define PT_EXIT (
    pt
) `/* multi line expression */`
```




<hr>



### define PT\_INIT 

_Initialize (or reset) a protothread so it starts from the top._ 
```C++
#define PT_INIT (
    pt
) `((pt)->lc = 0)`
```




<hr>



### define PT\_IS\_IDLE 

```C++
#define PT_IS_IDLE (
    pt
) `((pt)->lc == 0)`
```



Check if a protothread has not yet started or has been reset. 


        

<hr>



### define PT\_IS\_RUNNING 

```C++
#define PT_IS_RUNNING (
    pt
) `((pt)->lc != 0)`
```



Check if a protothread is still running (has not exited). 


        

<hr>



### define PT\_RESTART 

_Reset the protothread and restart from PT\_BEGIN on next call._ 
```C++
#define PT_RESTART (
    pt
) `/* multi line expression */`
```




<hr>



### define PT\_SPAWN 

_Spawn a child protothread and block until it exits._ 
```C++
#define PT_SPAWN (
    pt,
    child,
    func
) `/* multi line expression */`
```





**Parameters:**


* `pt` Parent protothread. 
* `child` Child protothread ([**SYN\_PT**](structSYN__PT.md) struct). 
* `func` Expression that calls the child's function, e.g. `child_func(&child_pt)`.

Usage: 
```C++
static SYN_PT child_pt;
PT_SPAWN(pt, &child_pt, child_func(&child_pt));
```
 


        

<hr>



### define PT\_TASK\_DELAY\_MS 

_Convenience form for use with the scheduler's_ [_**SYN\_Task**_](structSYN__Task.md) _._
```C++
#define PT_TASK_DELAY_MS (
    pt,
    task,
    ms
) `PT_DELAY_MS (pt, &(task)->delay_until, ms)`
```





**Parameters:**


* `pt` Protothread. 
* `task` Pointer to the [**SYN\_Task**](structSYN__Task.md) struct. 
* `ms` Delay duration in milliseconds. 




        

<hr>



### define PT\_WAIT\_UNTIL 

_Block until_ `cond` _evaluates to true._
```C++
#define PT_WAIT_UNTIL (
    pt,
    cond
) `/* multi line expression */`
```



Each time the protothread is polled and `cond` is false, it returns PT\_WAITING and resumes at this point on the next call. 


        

<hr>



### define PT\_WAIT\_WHILE 

_Block while_ `cond` _is true. Dual of PT\_WAIT\_UNTIL._
```C++
#define PT_WAIT_WHILE (
    pt,
    cond
) `PT_WAIT_UNTIL (pt, !(cond))`
```




<hr>



### define PT\_YIELD 

_Yield control unconditionally. The protothread will resume at this point on the next call._ 
```C++
#define PT_YIELD (
    pt
) `/* multi line expression */`
```




<hr>



### define PT\_YIELD\_UNTIL 

_Yield while_ `cond` _is true._
```C++
#define PT_YIELD_UNTIL (
    pt,
    cond
) `/* multi line expression */`
```



Like PT\_WAIT\_WHILE, but returns PT\_YIELDED instead of PT\_WAITING, signaling the scheduler that this thread actively wants to run. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/pt/syn_pt.h`

