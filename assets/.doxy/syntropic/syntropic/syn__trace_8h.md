

# File syn\_trace.h



[**FileList**](files.md) **>** [**debug**](dir_6f573363694cd388330fe66cf20dcff2.md) **>** [**syn\_trace.h**](syn__trace_8h.md)

[Go to the source code of this file](syn__trace_8h_source.md)

_Lightweight trace buffer — timestamped event recorder._ [More...](#detailed-description)

* `#include "../port/syn_port_system.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`
* `#include <stddef.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Trace**](structSYN__Trace.md) <br>_Circular trace event buffer._  |
| struct | [**SYN\_TraceEntry**](structSYN__TraceEntry.md) <br>_Single trace event entry._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_TracePrintFunc**](#typedef-syn_traceprintfunc)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_trace\_clear**](#function-syn_trace_clear) ([**SYN\_Trace**](structSYN__Trace.md) \* trace) <br>_Clear the trace buffer._  |
|  void | [**syn\_trace\_dump**](#function-syn_trace_dump) (const [**SYN\_Trace**](structSYN__Trace.md) \* trace, [**SYN\_TracePrintFunc**](syn__trace_8h.md#typedef-syn_traceprintfunc) print) <br>_Dump all entries via a print callback._  |
|  void | [**syn\_trace\_enable**](#function-syn_trace_enable) ([**SYN\_Trace**](structSYN__Trace.md) \* trace, [**bool**](syn__defs_8h.md#enum-bool) enable) <br>_Enable/disable recording._  |
|  void | [**syn\_trace\_init**](#function-syn_trace_init) ([**SYN\_Trace**](structSYN__Trace.md) \* trace, [**SYN\_TraceEntry**](structSYN__TraceEntry.md) \* entries, uint16\_t capacity) <br>_Initialize the trace buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_trace\_read**](#function-syn_trace_read) (const [**SYN\_Trace**](structSYN__Trace.md) \* trace, uint32\_t index, [**SYN\_TraceEntry**](structSYN__TraceEntry.md) \* entry) <br>_Read an entry by index (0 = oldest available)._  |
|  void | [**syn\_trace\_record**](#function-syn_trace_record) ([**SYN\_Trace**](structSYN__Trace.md) \* trace, uint16\_t event\_id, uint16\_t value) <br>_Record a trace event._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**syn\_trace\_count**](#function-syn_trace_count) (const [**SYN\_Trace**](structSYN__Trace.md) \* trace) <br>_Get total event count (may exceed capacity — indicates wrapping)._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_TRACE**](syn__trace_8h.md#define-syn_trace) (trace, id, val) `[**syn\_trace\_record**](syn__trace_8c.md#function-syn_trace_record)((trace), (id), (val))`<br> |

## Detailed Description


Records events (id + optional 16-bit value) into a fixed-size circular buffer for post-mortem debugging. No UART, no printf — just stamp and store. Read it out with a debugger or dump over CLI.




**
**


```C++
static SYN_TraceEntry entries[64];
static SYN_Trace trace;
syn_trace_init(&trace, entries, 64);

SYN_TRACE(&trace, EVT_ISR_ENTER, irq_num);
SYN_TRACE(&trace, EVT_TASK_SWITCH, task_id);

// Dump from CLI or debugger:
syn_trace_dump(&trace, print_func);
```
 





    
## Public Types Documentation




### typedef SYN\_TracePrintFunc 

```C++
typedef void(* SYN_TracePrintFunc) (const char *str);
```



Print callback for dump. 


        

<hr>
## Public Functions Documentation




### function syn\_trace\_clear 

_Clear the trace buffer._ 
```C++
void syn_trace_clear (
    SYN_Trace * trace
) 
```





**Parameters:**


* `trace` Trace instance to reset. 




        

<hr>



### function syn\_trace\_dump 

_Dump all entries via a print callback._ 
```C++
void syn_trace_dump (
    const SYN_Trace * trace,
    SYN_TracePrintFunc print
) 
```



Output format: "[tick] event\_id=0xNNNN value=0xNNNN\n"




**Parameters:**


* `trace` Trace instance. 
* `print` Print function (e.g., write to UART). 




        

<hr>



### function syn\_trace\_enable 

_Enable/disable recording._ 
```C++
void syn_trace_enable (
    SYN_Trace * trace,
    bool enable
) 
```





**Parameters:**


* `trace` Trace instance. 
* `enable` True to enable recording, false to pause. 




        

<hr>



### function syn\_trace\_init 

_Initialize the trace buffer._ 
```C++
void syn_trace_init (
    SYN_Trace * trace,
    SYN_TraceEntry * entries,
    uint16_t capacity
) 
```





**Parameters:**


* `trace` Trace instance to initialize. 
* `entries` Array of trace entries (caller-owned). 
* `capacity` Number of entries in the array. 




        

<hr>



### function syn\_trace\_read 

_Read an entry by index (0 = oldest available)._ 
```C++
bool syn_trace_read (
    const SYN_Trace * trace,
    uint32_t index,
    SYN_TraceEntry * entry
) 
```





**Parameters:**


* `trace` Trace instance. 
* `index` Index from oldest available. 
* `entry` Output entry. 



**Returns:**

true if a valid entry was read. 





        

<hr>



### function syn\_trace\_record 

_Record a trace event._ 
```C++
void syn_trace_record (
    SYN_Trace * trace,
    uint16_t event_id,
    uint16_t value
) 
```





**Parameters:**


* `trace` Trace instance. 
* `event_id` Application-defined event ID. 
* `value` Optional 16-bit payload. 




        

<hr>
## Public Static Functions Documentation




### function syn\_trace\_count 

_Get total event count (may exceed capacity — indicates wrapping)._ 
```C++
static inline uint32_t syn_trace_count (
    const SYN_Trace * trace
) 
```





**Parameters:**


* `trace` Trace instance. 



**Returns:**

Total number of events written since initialization. 





        

<hr>
## Macro Definition Documentation





### define SYN\_TRACE 

```C++
#define SYN_TRACE (
    trace,
    id,
    val
) `syn_trace_record ((trace), (id), (val))`
```



Convenience macro with auto-timestamping. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/debug/syn_trace.h`

