

# Struct SYN\_Profiler



[**ClassList**](annotated.md) **>** [**SYN\_Profiler**](structSYN__Profiler.md)



_Task profiler instance._ 

* `#include <syn_profiler.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**capacity**](#variable-capacity)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**enabled**](#variable-enabled)  <br> |
|  [**SYN\_ProfileEntry**](structSYN__ProfileEntry.md) \* | [**entries**](#variable-entries)  <br> |
|  uint32\_t | [**period\_ms**](#variable-period_ms)  <br> |
|  uint32\_t | [**period\_start**](#variable-period_start)  <br> |












































## Public Attributes Documentation




### variable capacity 

```C++
uint8_t SYN_Profiler::capacity;
```



Number of entries 
 


        

<hr>



### variable enabled 

```C++
bool SYN_Profiler::enabled;
```



Profiling active 
 


        

<hr>



### variable entries 

```C++
SYN_ProfileEntry* SYN_Profiler::entries;
```



Profile entry array 
 


        

<hr>



### variable period\_ms 

```C++
uint32_t SYN_Profiler::period_ms;
```



Measurement period (ms) 
 


        

<hr>



### variable period\_start 

```C++
uint32_t SYN_Profiler::period_start;
```



Tick at start of current period 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/debug/syn_profiler.h`

