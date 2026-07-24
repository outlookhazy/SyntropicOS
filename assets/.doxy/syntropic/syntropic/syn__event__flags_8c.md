

# File syn\_event\_flags.c



[**FileList**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_event\_flags.c**](syn__event__flags_8c.md)

[Go to the source code of this file](syn__event__flags_8c_source.md)

_32-bit Event Flag Group Synchronization implementation._ 

* `#include "syn_event_flags.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_event\_flags\_clear**](#function-syn_event_flags_clear) ([**SYN\_EventFlags**](structSYN__EventFlags.md) \* ef, uint32\_t flags\_mask) <br>_Clear event flags (Bitwise AND NOT)._  |
|  uint32\_t | [**syn\_event\_flags\_get**](#function-syn_event_flags_get) (const [**SYN\_EventFlags**](structSYN__EventFlags.md) \* ef) <br>_Get current event flags value without clearing._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_event\_flags\_init**](#function-syn_event_flags_init) ([**SYN\_EventFlags**](structSYN__EventFlags.md) \* ef) <br>_Initialize an Event Flags instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_event\_flags\_set**](#function-syn_event_flags_set) ([**SYN\_EventFlags**](structSYN__EventFlags.md) \* ef, uint32\_t flags\_mask) <br>_Set event flags (Bitwise OR)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_event\_flags\_wait**](#function-syn_event_flags_wait) ([**SYN\_EventFlags**](structSYN__EventFlags.md) \* ef, uint32\_t wait\_mask, uint32\_t mode, uint32\_t \* out\_flags) <br>_Non-blocking evaluate/wait for event flags condition._  |




























## Public Functions Documentation




### function syn\_event\_flags\_clear 

_Clear event flags (Bitwise AND NOT)._ 
```C++
SYN_Status syn_event_flags_clear (
    SYN_EventFlags * ef,
    uint32_t flags_mask
) 
```





**Parameters:**


* `ef` Event flags context. 
* `flags_mask` Bitmask of flags to clear. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_event\_flags\_get 

_Get current event flags value without clearing._ 
```C++
uint32_t syn_event_flags_get (
    const SYN_EventFlags * ef
) 
```





**Parameters:**


* `ef` Event flags context. 



**Returns:**

Current 32-bit flags value. 





        

<hr>



### function syn\_event\_flags\_init 

_Initialize an Event Flags instance._ 
```C++
SYN_Status syn_event_flags_init (
    SYN_EventFlags * ef
) 
```





**Parameters:**


* `ef` Event flags context. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_event\_flags\_set 

_Set event flags (Bitwise OR)._ 
```C++
SYN_Status syn_event_flags_set (
    SYN_EventFlags * ef,
    uint32_t flags_mask
) 
```





**Parameters:**


* `ef` Event flags context. 
* `flags_mask` Bitmask of flags to set. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_event\_flags\_wait 

_Non-blocking evaluate/wait for event flags condition._ 
```C++
SYN_Status syn_event_flags_wait (
    SYN_EventFlags * ef,
    uint32_t wait_mask,
    uint32_t mode,
    uint32_t * out_flags
) 
```





**Parameters:**


* `ef` Event flags context. 
* `wait_mask` Bitmask of flags to evaluate. 
* `mode` Combination of SYN\_EVENT\_FLAGS\_WAIT\_ANY/ALL/AUTO\_CLEAR. 
* `out_flags` Optional pointer receiving matched flags. 



**Returns:**

SYN\_OK if condition is satisfied, or SYN\_BUSY if not met. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_event_flags.c`

