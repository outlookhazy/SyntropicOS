

# File syn\_event.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_event.h**](syn__event_8h.md)

[Go to the source code of this file](syn__event_8h_source.md)

_Event flag groups — 32-bit bitmask signaling._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../util/syn_bits.h"`
* `#include "../pt/syn_pt.h"`
* `#include "../sched/syn_event_flags.h"`
* `#include <stdbool.h>`

















## Public Types

| Type | Name |
| ---: | :--- |
| typedef [**SYN\_EventFlags**](structSYN__EventFlags.md) | [**SYN\_EventGroup**](#typedef-syn_eventgroup)  <br>_Event flag group — alias for_ [_**SYN\_EventFlags**_](structSYN__EventFlags.md) _._ |






















## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_event\_check\_all**](#function-syn_event_check_all) (const [**SYN\_EventGroup**](syn__event_8h.md#typedef-syn_eventgroup) \* grp, uint32\_t mask) <br>_Check if ALL bits in_ `mask` _are set._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_event\_check\_any**](#function-syn_event_check_any) (const [**SYN\_EventGroup**](syn__event_8h.md#typedef-syn_eventgroup) \* grp, uint32\_t mask) <br>_Check if ANY bit in_ `mask` _is set._ |
|  void | [**syn\_event\_clear**](#function-syn_event_clear) ([**SYN\_EventGroup**](syn__event_8h.md#typedef-syn_eventgroup) \* grp, uint32\_t mask) <br>_Clear one or more event flags._  |
|  uint32\_t | [**syn\_event\_get**](#function-syn_event_get) (const [**SYN\_EventGroup**](syn__event_8h.md#typedef-syn_eventgroup) \* grp) <br>_Get the current value of all flags._  |
|  void | [**syn\_event\_init**](#function-syn_event_init) ([**SYN\_EventGroup**](syn__event_8h.md#typedef-syn_eventgroup) \* grp) <br>_Initialize an event group._  |
|  void | [**syn\_event\_set**](#function-syn_event_set) ([**SYN\_EventGroup**](syn__event_8h.md#typedef-syn_eventgroup) \* grp, uint32\_t mask) <br>_Set one or more event flags._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**PT\_WAIT\_EVENT**](syn__event_8h.md#define-pt_wait_event) (pt, grp, mask) `/* multi line expression */`<br>_Block the protothread until ALL bits in_ `mask` _are set, then auto-clear._ |
| define  | [**PT\_WAIT\_EVENT\_ANY**](syn__event_8h.md#define-pt_wait_event_any) (pt, grp, mask) `/* multi line expression */`<br>_Block the protothread until ANY bit in_ `mask` _is set, then auto-clear._ |

## Detailed Description


Unified event signaling delegating directly to [**SYN\_EventFlags**](structSYN__EventFlags.md). 


    
## Public Types Documentation




### typedef SYN\_EventGroup 

_Event flag group — alias for_ [_**SYN\_EventFlags**_](structSYN__EventFlags.md) _._
```C++
typedef SYN_EventFlags SYN_EventGroup;
```




<hr>
## Public Static Functions Documentation




### function syn\_event\_check\_all 

_Check if ALL bits in_ `mask` _are set._
```C++
static inline bool syn_event_check_all (
    const SYN_EventGroup * grp,
    uint32_t mask
) 
```





**Parameters:**


* `grp` Event group. 
* `mask` Bitmask to test. 



**Returns:**

true if every bit in `mask` is set in the event group. 





        

<hr>



### function syn\_event\_check\_any 

_Check if ANY bit in_ `mask` _is set._
```C++
static inline bool syn_event_check_any (
    const SYN_EventGroup * grp,
    uint32_t mask
) 
```





**Parameters:**


* `grp` Event group. 
* `mask` Bitmask to test. 



**Returns:**

true if at least one bit in `mask` is set. 





        

<hr>



### function syn\_event\_clear 

_Clear one or more event flags._ 
```C++
static inline void syn_event_clear (
    SYN_EventGroup * grp,
    uint32_t mask
) 
```





**Parameters:**


* `grp` Event group. 
* `mask` Bitmask of flags to clear. 




        

<hr>



### function syn\_event\_get 

_Get the current value of all flags._ 
```C++
static inline uint32_t syn_event_get (
    const SYN_EventGroup * grp
) 
```





**Parameters:**


* `grp` Event group. 



**Returns:**

Current flags. 





        

<hr>



### function syn\_event\_init 

_Initialize an event group._ 
```C++
static inline void syn_event_init (
    SYN_EventGroup * grp
) 
```





**Parameters:**


* `grp` Event group. 




        

<hr>



### function syn\_event\_set 

_Set one or more event flags._ 
```C++
static inline void syn_event_set (
    SYN_EventGroup * grp,
    uint32_t mask
) 
```





**Parameters:**


* `grp` Event group. 
* `mask` Bitmask of flags to set. 




        

<hr>
## Macro Definition Documentation





### define PT\_WAIT\_EVENT 

_Block the protothread until ALL bits in_ `mask` _are set, then auto-clear._
```C++
#define PT_WAIT_EVENT (
    pt,
    grp,
    mask
) `/* multi line expression */`
```




<hr>



### define PT\_WAIT\_EVENT\_ANY 

_Block the protothread until ANY bit in_ `mask` _is set, then auto-clear._
```C++
#define PT_WAIT_EVENT_ANY (
    pt,
    grp,
    mask
) `/* multi line expression */`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_event.h`

