

# File syn\_event.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_event.h**](syn__event_8h.md)

[Go to the source code of this file](syn__event_8h_source.md)

_Event flag groups — 32-bit bitmask signaling._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../util/syn_bits.h"`
* `#include "../pt/syn_pt.h"`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_EventGroup**](structSYN__EventGroup.md) <br>_Event flag group — a 32-bit bitmask of event flags._  |
























## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_event\_check\_all**](#function-syn_event_check_all) (const [**SYN\_EventGroup**](structSYN__EventGroup.md) \* grp, uint32\_t mask) <br>_Check if ALL bits in_ `mask` _are set._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_event\_check\_any**](#function-syn_event_check_any) (const [**SYN\_EventGroup**](structSYN__EventGroup.md) \* grp, uint32\_t mask) <br>_Check if ANY bit in_ `mask` _is set._ |
|  void | [**syn\_event\_clear**](#function-syn_event_clear) ([**SYN\_EventGroup**](structSYN__EventGroup.md) \* grp, uint32\_t mask) <br>_Clear one or more event flags._  |
|  uint32\_t | [**syn\_event\_get**](#function-syn_event_get) (const [**SYN\_EventGroup**](structSYN__EventGroup.md) \* grp) <br>_Get the current value of all flags._  |
|  void | [**syn\_event\_init**](#function-syn_event_init) ([**SYN\_EventGroup**](structSYN__EventGroup.md) \* grp) <br>_Initialize an event group (clear all flags)._  |
|  void | [**syn\_event\_set**](#function-syn_event_set) ([**SYN\_EventGroup**](structSYN__EventGroup.md) \* grp, uint32\_t mask) <br>_Set one or more event flags._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**PT\_WAIT\_EVENT**](syn__event_8h.md#define-pt_wait_event) (pt, grp, mask) `/* multi line expression */`<br>_Block the protothread until ALL bits in_ `mask` _are set, then auto-clear them._ |
| define  | [**PT\_WAIT\_EVENT\_ANY**](syn__event_8h.md#define-pt_wait_event_any) (pt, grp, mask) `/* multi line expression */`<br>_Block the protothread until ANY bit in_ `mask` _is set, then auto-clear the bits that were set._ |

## Detailed Description


Event flags provide a lightweight way to signal between tasks and ISRs. Each event group is a 32-bit word where each bit represents a distinct event. Tasks can wait for any combination of bits to be set.


Setting flags is ISR-safe (single atomic write on 32-bit targets).




**
**


```C++
#define EVT_DATA_READY   SYN_BIT(0)
#define EVT_TIMEOUT      SYN_BIT(1)

static SYN_EventGroup events;
syn_event_init(&events);

// ISR or another task:
syn_event_set(&events, EVT_DATA_READY);

// In a protothread:
PT_WAIT_EVENT(pt, &events, EVT_DATA_READY);
// EVT_DATA_READY is now auto-cleared, process data...
```
 





    
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

_Initialize an event group (clear all flags)._ 
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



ISR-safe on 32-bit targets (single word write).




**Parameters:**


* `grp` Event group. 
* `mask` Bitmask of flags to set. 




        

<hr>
## Macro Definition Documentation





### define PT\_WAIT\_EVENT 

_Block the protothread until ALL bits in_ `mask` _are set, then auto-clear them._
```C++
#define PT_WAIT_EVENT (
    pt,
    grp,
    mask
) `/* multi line expression */`
```





**Parameters:**


* `pt` Protothread. 
* `grp` Event group. 
* `mask` Bitmask of required flags. 




        

<hr>



### define PT\_WAIT\_EVENT\_ANY 

_Block the protothread until ANY bit in_ `mask` _is set, then auto-clear the bits that were set._
```C++
#define PT_WAIT_EVENT_ANY (
    pt,
    grp,
    mask
) `/* multi line expression */`
```





**Parameters:**


* `pt` Protothread. 
* `grp` Event group. 
* `mask` Bitmask of flags to wait for (any of them). 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_event.h`

