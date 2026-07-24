

# File syn\_profiler.h



[**FileList**](files.md) **>** [**debug**](dir_6f573363694cd388330fe66cf20dcff2.md) **>** [**syn\_profiler.h**](syn__profiler_8h.md)

[Go to the source code of this file](syn__profiler_8h_source.md)

_Task profiler — CPU time tracking per scheduler task._ [More...](#detailed-description)

* `#include "../port/syn_port_system.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`
* `#include <stddef.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_ProfileEntry**](structSYN__ProfileEntry.md) <br>_Per-task profile entry — timing stats and CPU usage._  |
| struct | [**SYN\_Profiler**](structSYN__Profiler.md) <br>_Task profiler instance._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_ProfilerPrintFunc**](#typedef-syn_profilerprintfunc)  <br>_Print callback for profiler dump output._  |




















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


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  const [**SYN\_ProfileEntry**](structSYN__ProfileEntry.md) \* | [**syn\_profiler\_get**](#function-syn_profiler_get) (const [**SYN\_Profiler**](structSYN__Profiler.md) \* prof, uint8\_t index) <br>_Get a profile entry (read-only)._  |


























## Detailed Description


Hooks into the scheduler to measure how much time each task consumes. Reports percentage, peak time, and call count.




**
**


```C++
static SYN_ProfileEntry prof_entries[8];
static SYN_Profiler profiler;
syn_profiler_init(&profiler, prof_entries, 8);

// In scheduler loop (before/after each task run):
syn_profiler_task_begin(&profiler, task_index);
// ... run task ...
syn_profiler_task_end(&profiler, task_index);

// Periodically (e.g., every 1s):
syn_profiler_update(&profiler);  // calculates percentages
syn_profiler_dump(&profiler, print_func);
```
 





    
## Public Types Documentation




### typedef SYN\_ProfilerPrintFunc 

_Print callback for profiler dump output._ 
```C++
typedef void(* SYN_ProfilerPrintFunc) (const char *str);
```





**Parameters:**


* `str` Null-terminated string to output. 




        

<hr>
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
## Public Static Functions Documentation




### function syn\_profiler\_get 

_Get a profile entry (read-only)._ 
```C++
static inline const SYN_ProfileEntry * syn_profiler_get (
    const SYN_Profiler * prof,
    uint8_t index
) 
```





**Parameters:**


* `prof` Profiler. 
* `index` Task index. 



**Returns:**

Profile entry, or NULL if index out of range. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/debug/syn_profiler.h`

