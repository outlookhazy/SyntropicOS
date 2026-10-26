

# Struct SYN\_ProfileEntry



[**ClassList**](annotated.md) **>** [**SYN\_ProfileEntry**](structSYN__ProfileEntry.md)



_Per-task profile entry — timing stats and CPU usage._ 

* `#include <syn_profiler.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**\_start\_tick**](#variable-_start_tick)  <br> |
|  uint16\_t | [**cpu\_percent\_x10**](#variable-cpu_percent_x10)  <br> |
|  const char \* | [**name**](#variable-name)  <br> |
|  uint32\_t | [**peak\_us**](#variable-peak_us)  <br> |
|  uint32\_t | [**run\_count**](#variable-run_count)  <br> |
|  uint32\_t | [**total\_us**](#variable-total_us)  <br> |












































## Public Attributes Documentation




### variable \_start\_tick 

```C++
uint32_t SYN_ProfileEntry::_start_tick;
```



Internal: tick at task\_begin() 
 


        

<hr>



### variable cpu\_percent\_x10 

```C++
uint16_t SYN_ProfileEntry::cpu_percent_x10;
```



CPU% × 10 (e.g., 125 = 12.5%) 
 


        

<hr>



### variable name 

```C++
const char* SYN_ProfileEntry::name;
```



Task name 
 


        

<hr>



### variable peak\_us 

```C++
uint32_t SYN_ProfileEntry::peak_us;
```



Peak single-run time (µs) 
 


        

<hr>



### variable run\_count 

```C++
uint32_t SYN_ProfileEntry::run_count;
```



Number of runs in current period 
 


        

<hr>



### variable total\_us 

```C++
uint32_t SYN_ProfileEntry::total_us;
```



Total time in current period (µs) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/debug/syn_profiler.h`

