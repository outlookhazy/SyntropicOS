

# File syn\_pt.h



[**FileList**](files.md) **>** [**pt**](dir_a1b594c7ca8b98b889036d2814e4f947.md) **>** [**syn\_pt.h**](syn__pt_8h.md)

[Go to the source code of this file](syn__pt_8h_source.md)

_Protothreads — stackless cooperative coroutines for C._ [More...](#detailed-description)

* `#include "../common/syn_barrier.h"`
* `#include "../common/syn_compiler.h"`
* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_system.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_PT**](structSYN__PT.md) <br>_Protothread control block._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_PT\_Status**](#enum-syn_pt_status)  <br>_Return value from a protothread function._  |






















## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_pt\_arm\_ms\_deadline**](#function-syn_pt_arm_ms_deadline) (uint32\_t \* target, uint32\_t now, uint32\_t ms) <br>_Internal helper: single-evaluation, INT32\_MAX-clamped millisecond deadline arming._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_pt\_arm\_us\_deadline**](#function-syn_pt_arm_us_deadline) (uint32\_t \* target, uint32\_t now, uint32\_t us) <br>_Internal helper: single-evaluation, INT32\_MAX-clamped microsecond deadline arming._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**PT\_BEGIN**](syn__pt_8h.md#define-pt_begin) (pt) `/* multi line expression */`<br>_Open a protothread body. Must be the first statement in the protothread function, before any PT\_\* operations._  |
| define  | [**PT\_BLOCK\_CONDITION**](syn__pt_8h.md#define-pt_block_condition) (pt, task, cond) `/* multi line expression */`<br>_Sleep task (SYN\_TASK\_BLOCKED) until explicitly resumed._  |
| define  | [**PT\_BLOCK\_EVENT**](syn__pt_8h.md#define-pt_block_event) (pt, task, grp, mask) `/* multi line expression */`<br>_Block until ANY bit in_ `mask` _is set in the event group._ |
| define  | [**PT\_BLOCK\_EVENT\_WITH\_TIMEOUT**](syn__pt_8h.md#define-pt_block_event_with_timeout) (pt, task, grp, mask, timeout\_ms, out\_flags) `/* multi line expression */`<br>_Block task execution (SYN\_TASK\_BLOCKED) until ANY bit in_ `mask` _is set OR_`timeout_ms` _milliseconds elapse._ |
| define  | [**PT\_CONTINUE**](syn__pt_8h.md#define-pt_continue) (pt) `/* multi line expression */`<br>_Yield after completing a work chunk with PT\_MORE._  |
| define  | [**PT\_DELAY\_MS**](syn__pt_8h.md#define-pt_delay_ms) (pt, target, ms) `/* multi line expression */`<br>_Non-blocking delay for_ `ms` _milliseconds._ |
| define  | [**PT\_DELAY\_US**](syn__pt_8h.md#define-pt_delay_us) (pt, target, us) `/* multi line expression */`<br>_Non-blocking sub-millisecond delay for_ `us` _microseconds._ |
| define  | [**PT\_END**](syn__pt_8h.md#define-pt_end) (pt) `/* multi line expression */`<br>_Close a protothread body. Returns PT\_ENDED and resets the continuation so the thread can be restarted if desired._  |
| define  | [**PT\_EXIT**](syn__pt_8h.md#define-pt_exit) (pt) `/* multi line expression */`<br>_Abort the protothread immediately and return PT\_EXITED._  |
| define  | [**PT\_INIT**](syn__pt_8h.md#define-pt_init) (pt) `((pt)-&gt;lc = 0)`<br>_Initialize (or reset) a protothread so it starts from the top._  |
| define  | [**PT\_IS\_IDLE**](syn__pt_8h.md#define-pt_is_idle) (pt) `((pt)-&gt;lc == 0)`<br> |
| define  | [**PT\_IS\_RUNNING**](syn__pt_8h.md#define-pt_is_running) (pt) `((pt)-&gt;lc != 0)`<br> |
| define  | [**PT\_PASS**](syn__pt_8h.md#define-pt_pass) (pt) `/* multi line expression */`<br>_Pass the remainder of this scheduler round with PT\_IDLE._  |
| define  | [**PT\_RESTART**](syn__pt_8h.md#define-pt_restart) (pt) `/* multi line expression */`<br>_Reset the protothread and restart from PT\_BEGIN on next call._  |
| define  | [**PT\_SPAWN**](syn__pt_8h.md#define-pt_spawn) (pt, child, func) `/* multi line expression */`<br>_Spawn a child protothread and block until it exits._  |
| define  | [**PT\_TASK\_DELAY\_MS**](syn__pt_8h.md#define-pt_task_delay_ms) (pt, task, ms) `[**PT\_DELAY\_MS**](syn__pt_8h.md#define-pt_delay_ms)(pt, &(task)-&gt;delay\_until, ms)`<br>_Convenience form for use with the scheduler's_ [_**SYN\_Task**_](structSYN__Task.md) _(milliseconds)._ |
| define  | [**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until) (pt, cond) `/* multi line expression */`<br>_Block until_ `cond` _evaluates to true._ |
| define  | [**PT\_WAIT\_WHILE**](syn__pt_8h.md#define-pt_wait_while) (pt, cond) `[**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until)(pt, !(cond))`<br>_Block while_ `cond` _is true. Dual of PT\_WAIT\_UNTIL._ |

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





**
**


```C++
// High-level C code
SYN_PT_Status my_task(SYN_PT *pt) {
    PT_BEGIN(pt);
    PT_WAIT_UNTIL(pt, flag == 1);
    PT_END(pt);
}

// Expands preprocessor output to:
SYN_PT_Status my_task(SYN_PT *pt) {
    char _pt_yield_flag = 1;
    switch (pt->lc) {
    case 0:
        // On resume, switch(pt->lc) jumps directly to case 12:
        pt->lc = 12; case 12:
        if (!(flag == 1)) return PT_IDLE;
    }
    _pt_yield_flag = 0;
    PT_INIT(pt);
    return PT_ENDED;
}
```
 





    
## Public Types Documentation




### enum SYN\_PT\_Status 

_Return value from a protothread function._ 
```C++
enum SYN_PT_Status {
    PT_IDLE = 0,
    PT_MORE = 1,
    PT_EXITED = 2,
    PT_ENDED = 3
};
```




<hr>
## Public Static Functions Documentation




### function syn\_pt\_arm\_ms\_deadline 

_Internal helper: single-evaluation, INT32\_MAX-clamped millisecond deadline arming._ 
```C++
static inline bool syn_pt_arm_ms_deadline (
    uint32_t * target,
    uint32_t now,
    uint32_t ms
) 
```





**Parameters:**


* `target` Pointer to deadline storage variable. 
* `now` Current millisecond tick counter. 
* `ms` Requested delay in milliseconds. 



**Returns:**

true if a non-zero deadline was armed, false if `ms` is 0. 





        

<hr>



### function syn\_pt\_arm\_us\_deadline 

_Internal helper: single-evaluation, INT32\_MAX-clamped microsecond deadline arming._ 
```C++
static inline bool syn_pt_arm_us_deadline (
    uint32_t * target,
    uint32_t now,
    uint32_t us
) 
```





**Parameters:**


* `target` Pointer to deadline storage variable. 
* `now` Current microsecond tick counter. 
* `us` Requested delay in microseconds. 



**Returns:**

true if a non-zero deadline was armed, false if `us` is 0. 





        

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



### define PT\_BLOCK\_CONDITION 

_Sleep task (SYN\_TASK\_BLOCKED) until explicitly resumed._ 
```C++
#define PT_BLOCK_CONDITION (
    pt,
    task,
    cond
) `/* multi line expression */`
```





**Warning:**

This macro does NOT poll `cond` on each scheduler tick. Once the task yields as BLOCKED, the scheduler skips it entirely. The condition is only re-evaluated when the task is explicitly woken by an ISR or driver calling syn\_task\_resume(task).


For polled condition blocking, use [**PT\_WAIT\_UNTIL(pt, cond)**](syn__pt_8h.md#define-pt_wait_until) instead. For event-flag-driven blocking, use [**PT\_BLOCK\_EVENT(pt, task, grp, mask)**](syn__pt_8h.md#define-pt_block_event).




**Parameters:**


* `pt` Protothread. 
* `task` Pointer to the [**SYN\_Task**](structSYN__Task.md) struct. 
* `cond` Boolean condition expression to evaluate. 




        

<hr>



### define PT\_BLOCK\_EVENT 

_Block until ANY bit in_ `mask` _is set in the event group._
```C++
#define PT_BLOCK_EVENT (
    pt,
    task,
    grp,
    mask
) `/* multi line expression */`
```



Unlike PT\_WAIT\_EVENT (which polls every scheduler pass), this macro sets the task state to BLOCKED. The scheduler skips the task entirely until the event fires, saving CPU and enabling proper tickless sleep.


Matched flags are atomically captured and auto-cleared when the scheduler dispatches the woken task.




**Parameters:**


* `pt` Protothread. 
* `task` Pointer to the [**SYN\_Task**](structSYN__Task.md) struct. 
* `grp` Pointer to [**SYN\_EventGroup**](syn__event_8h.md#typedef-syn_eventgroup). 
* `mask` Bitmask of event flags to wait for (any of them). 




        

<hr>



### define PT\_BLOCK\_EVENT\_WITH\_TIMEOUT 

_Block task execution (SYN\_TASK\_BLOCKED) until ANY bit in_ `mask` _is set OR_`timeout_ms` _milliseconds elapse._
```C++
#define PT_BLOCK_EVENT_WITH_TIMEOUT (
    pt,
    task,
    grp,
    mask,
    timeout_ms,
    out_flags
) `/* multi line expression */`
```



Combines event-flag blocking with millisecond deadline tracking. The task is skipped by the scheduler until either the event fires or the timeout deadline passes.


Matched event flags are captured into `out_flags` (if non-NULL) and auto-cleared ONLY if the event actually fired before the timeout. Check `*out_flags != 0` to determine if the event fired vs timed out.




**Parameters:**


* `pt` Protothread. 
* `task` Pointer to the [**SYN\_Task**](structSYN__Task.md) struct. 
* `grp` Pointer to [**SYN\_EventFlags**](structSYN__EventFlags.md). 
* `mask` Bitmask of event flags to wait for. 
* `timeout_ms` Timeout duration in milliseconds. 
* `out_flags` Pointer to uint32\_t receiving matched flags (or 0 on timeout). 




        

<hr>



### define PT\_CONTINUE 

_Yield after completing a work chunk with PT\_MORE._ 
```C++
#define PT_CONTINUE (
    pt
) `/* multi line expression */`
```



Signals that the protothread actively has more work remaining and should remain in the scheduler's Work tier. 


        

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




**Note:**

The target is reset to 0 (the scheduler's "no deadline pending" sentinel) once the delay completes. A deadline left in place would read as "in the future" again after the tick counter advances 2^31 ms (~24.8 days) past it, freezing the task. A computed deadline of exactly 0 (tick wrap) is nudged to 1 (or 0xFFFFFFFF when clamped == INT32\_MAX) so it cannot alias the sentinel while keeping `(int32_t)(dl - now) > 0`. Durations exceeding INT32\_MAX ms are clamped to INT32\_MAX ms, and `ms` is evaluated exactly once.




**Parameters:**


* `pt` Protothread. 
* `target` Pointer to a uint32\_t that will hold the deadline tick. 
* `ms` Delay duration in milliseconds. 




        

<hr>



### define PT\_DELAY\_US 

_Non-blocking sub-millisecond delay for_ `us` _microseconds._
```C++
#define PT_DELAY_US (
    pt,
    target,
    us
) `/* multi line expression */`
```



Requires a user-supplied persistent `uint32_t` variable (e.g., `static uint32_t us_target;`). Does not modify `SYN_Task.delay_until` to avoid corrupting the millisecond scheduler timebase.




**Parameters:**


* `pt` Protothread. 
* `target` Pointer to a uint32\_t variable holding the microsecond deadline. 
* `us` Delay duration in microseconds. 




        

<hr>



### define PT\_END 

_Close a protothread body. Returns PT\_ENDED and resets the continuation so the thread can be restarted if desired._ 
```C++
#define PT_END (
    pt
) `/* multi line expression */`
```




<hr>



### define PT\_EXIT 

_Abort the protothread immediately and return PT\_EXITED._ 
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



### define PT\_PASS 

_Pass the remainder of this scheduler round with PT\_IDLE._ 
```C++
#define PT_PASS (
    pt
) `/* multi line expression */`
```



Signals that the protothread has no immediate work right now so other ready tasks (including lower priorities) can run this round. 


        

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



Propagates the child's PT\_IDLE or PT\_MORE status verbatim while running.




**Parameters:**


* `pt` Parent protothread. 
* `child` Child protothread ([**SYN\_PT**](structSYN__PT.md) struct). 
* `func` Expression that calls the child's function, e.g. `child_func(&child_pt)`.



**Warning:**

The `func` expression is evaluated on EVERY scheduler poll until the child exits. Never pass expressions with side-effects (e.g. counter++, function calls that modify state). Pre-evaluate arguments before calling PT\_SPAWN.


Usage: 
```C++
static SYN_PT child_pt;
PT_SPAWN(pt, &child_pt, child_func(&child_pt));
```
 


        

<hr>



### define PT\_TASK\_DELAY\_MS 

_Convenience form for use with the scheduler's_ [_**SYN\_Task**_](structSYN__Task.md) _(milliseconds)._
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



Each time the protothread is polled and `cond` is false, it returns PT\_IDLE and resumes at this point on the next call. 


        

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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/pt/syn_pt.h`

