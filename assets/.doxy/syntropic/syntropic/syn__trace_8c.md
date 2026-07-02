

# File syn\_trace.c



[**FileList**](files.md) **>** [**debug**](dir_6f573363694cd388330fe66cf20dcff2.md) **>** [**syn\_trace.c**](syn__trace_8c.md)

[Go to the source code of this file](syn__trace_8c_source.md)

_Trace buffer implementation._ 

* `#include "syn_trace.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_fmt.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_trace\_clear**](#function-syn_trace_clear) ([**SYN\_Trace**](structSYN__Trace.md) \* trace) <br>_Clear the trace buffer._  |
|  void | [**syn\_trace\_dump**](#function-syn_trace_dump) (const [**SYN\_Trace**](structSYN__Trace.md) \* trace, [**SYN\_TracePrintFunc**](syn__trace_8h.md#typedef-syn_traceprintfunc) print) <br>_Dump all entries via a print callback._  |
|  void | [**syn\_trace\_enable**](#function-syn_trace_enable) ([**SYN\_Trace**](structSYN__Trace.md) \* trace, [**bool**](syn__defs_8h.md#enum-bool) enable) <br>_Enable/disable recording._  |
|  void | [**syn\_trace\_init**](#function-syn_trace_init) ([**SYN\_Trace**](structSYN__Trace.md) \* trace, [**SYN\_TraceEntry**](structSYN__TraceEntry.md) \* entries, uint16\_t capacity) <br>_Initialize the trace buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_trace\_read**](#function-syn_trace_read) (const [**SYN\_Trace**](structSYN__Trace.md) \* trace, uint32\_t index, [**SYN\_TraceEntry**](structSYN__TraceEntry.md) \* entry) <br>_Read an entry by index (0 = oldest available)._  |
|  void | [**syn\_trace\_record**](#function-syn_trace_record) ([**SYN\_Trace**](structSYN__Trace.md) \* trace, uint16\_t event\_id, uint16\_t value) <br>_Record a trace event._  |




























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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/debug/syn_trace.c`

