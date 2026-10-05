

# Struct SYN\_TaskProfile



[**ClassList**](annotated.md) **>** [**SYN\_TaskProfile**](structSYN__TaskProfile.md)



_Task Execution Profile Metrics Structure._ 

* `#include <syn_task_profile.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  float | [**cpu\_pct**](#variable-cpu_pct)  <br> |
|  uint32\_t | [**last\_start\_us**](#variable-last_start_us)  <br> |
|  uint32\_t | [**max\_yield\_us**](#variable-max_yield_us)  <br> |
|  uint32\_t | [**step\_count**](#variable-step_count)  <br> |
|  const char \* | [**task\_name**](#variable-task_name)  <br> |
|  uint32\_t | [**total\_time\_us**](#variable-total_time_us)  <br> |












































## Public Attributes Documentation




### variable cpu\_pct 

```C++
float SYN_TaskProfile::cpu_pct;
```



Computed CPU utilization percentage 
 


        

<hr>



### variable last\_start\_us 

```C++
uint32_t SYN_TaskProfile::last_start_us;
```



Microsecond timestamp of step start 
 


        

<hr>



### variable max\_yield\_us 

```C++
uint32_t SYN_TaskProfile::max_yield_us;
```



Maximum continuous execution time in us 


        

<hr>



### variable step\_count 

```C++
uint32_t SYN_TaskProfile::step_count;
```



Total number of times task executed 
 


        

<hr>



### variable task\_name 

```C++
const char* SYN_TaskProfile::task_name;
```



Pointer to task name string 
 


        

<hr>



### variable total\_time\_us 

```C++
uint32_t SYN_TaskProfile::total_time_us;
```



Total accumulated execution time in us 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/debug/syn_task_profile.h`

