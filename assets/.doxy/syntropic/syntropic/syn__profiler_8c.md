

# File syn\_profiler.c



[**FileList**](files.md) **>** [**debug**](dir_6f573363694cd388330fe66cf20dcff2.md) **>** [**syn\_profiler.c**](syn__profiler_8c.md)

[Go to the source code of this file](syn__profiler_8c_source.md)

_Task profiler implementation._ 

* `#include "syn_profiler.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_fmt.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_profiler\_dump**](#function-syn_profiler_dump) (const [**SYN\_Profiler**](structSYN__Profiler.md) \* prof, [**SYN\_ProfilerPrintFunc**](syn__profiler_8h.md#typedef-syn_profilerprintfunc) print) <br>_Dump profiler results via print callback._  |
|  void | [**syn\_profiler\_enable**](#function-syn_profiler_enable) ([**SYN\_Profiler**](structSYN__Profiler.md) \* prof, [**bool**](syn__defs_8h.md#enum-bool) enable) <br>_Enable/disable profiling._  |
|  void | [**syn\_profiler\_init**](#function-syn_profiler_init) ([**SYN\_Profiler**](structSYN__Profiler.md) \* prof, [**SYN\_ProfileEntry**](structSYN__ProfileEntry.md) \* entries, uint8\_t capacity) <br>_Initialize the profiler._  |
|  void | [**syn\_profiler\_register**](#function-syn_profiler_register) ([**SYN\_Profiler**](structSYN__Profiler.md) \* prof, uint8\_t index, const char \* name) <br>_Register a task for profiling._  |
|  void | [**syn\_profiler\_task\_begin**](#function-syn_profiler_task_begin) ([**SYN\_Profiler**](structSYN__Profiler.md) \* prof, uint8\_t index) <br>_Mark the start of a task's execution._  |
|  void | [**syn\_profiler\_task\_end**](#function-syn_profiler_task_end) ([**SYN\_Profiler**](structSYN__Profiler.md) \* prof, uint8\_t index) <br>_Mark the end of a task's execution._  |
|  void | [**syn\_profiler\_update**](#function-syn_profiler_update) ([**SYN\_Profiler**](structSYN__Profiler.md) \* prof) <br>_Update CPU percentages and reset counters for the next period._  |




























## Public Functions Documentation




### function syn\_profiler\_dump 

_Dump profiler results via print callback._ 
```C++
void syn_profiler_dump (
    const SYN_Profiler * prof,
    SYN_ProfilerPrintFunc print
) 
```





**Parameters:**


* `prof` Profiler. 
* `print` Print function. 




        

<hr>



### function syn\_profiler\_enable 

_Enable/disable profiling._ 
```C++
void syn_profiler_enable (
    SYN_Profiler * prof,
    bool enable
) 
```





**Parameters:**


* `prof` Profiler. 
* `enable` true to enable, false to disable. 




        

<hr>



### function syn\_profiler\_init 

_Initialize the profiler._ 
```C++
void syn_profiler_init (
    SYN_Profiler * prof,
    SYN_ProfileEntry * entries,
    uint8_t capacity
) 
```





**Parameters:**


* `prof` Profiler instance. 
* `entries` Array of profile entries (one per task). 
* `capacity` Number of entries. 




        

<hr>



### function syn\_profiler\_register 

_Register a task for profiling._ 
```C++
void syn_profiler_register (
    SYN_Profiler * prof,
    uint8_t index,
    const char * name
) 
```





**Parameters:**


* `prof` Profiler. 
* `index` Task index (must match scheduler task index). 
* `name` Task name for display. 




        

<hr>



### function syn\_profiler\_task\_begin 

_Mark the start of a task's execution._ 
```C++
void syn_profiler_task_begin (
    SYN_Profiler * prof,
    uint8_t index
) 
```





**Parameters:**


* `prof` Profiler. 
* `index` Task index. 




        

<hr>



### function syn\_profiler\_task\_end 

_Mark the end of a task's execution._ 
```C++
void syn_profiler_task_end (
    SYN_Profiler * prof,
    uint8_t index
) 
```





**Parameters:**


* `prof` Profiler. 
* `index` Task index. 




        

<hr>



### function syn\_profiler\_update 

_Update CPU percentages and reset counters for the next period._ 
```C++
void syn_profiler_update (
    SYN_Profiler * prof
) 
```



Call periodically (e.g., every 1 second).




**Parameters:**


* `prof` Profiler. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/debug/syn_profiler.c`

