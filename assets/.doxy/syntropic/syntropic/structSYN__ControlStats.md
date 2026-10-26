

# Struct SYN\_ControlStats



[**ClassList**](annotated.md) **>** [**SYN\_ControlStats**](structSYN__ControlStats.md)



_Raw accumulators for control performance analysis._ 

* `#include <syn_control_stats.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int32\_t | [**last\_output**](#variable-last_output)  <br> |
|  int32\_t | [**max\_error**](#variable-max_error)  <br> |
|  uint32\_t | [**samples**](#variable-samples)  <br> |
|  int64\_t | [**sum\_abs\_delta**](#variable-sum_abs_delta)  <br> |
|  int64\_t | [**sum\_itae**](#variable-sum_itae)  <br> |
|  int64\_t | [**sum\_sq\_err**](#variable-sum_sq_err)  <br> |
|  int64\_t | [**sum\_sq\_out**](#variable-sum_sq_out)  <br> |












































## Public Attributes Documentation




### variable last\_output 

```C++
int32_t SYN_ControlStats::last_output;
```



Previous output for delta calculation 
 


        

<hr>



### variable max\_error 

```C++
int32_t SYN_ControlStats::max_error;
```



Maximum absolute error observed 
 


        

<hr>



### variable samples 

```C++
uint32_t SYN_ControlStats::samples;
```



Number of samples collected 
 


        

<hr>



### variable sum\_abs\_delta 

```C++
int64_t SYN_ControlStats::sum_abs_delta;
```



Accumulator for Jitter (Output Slew) 
 


        

<hr>



### variable sum\_itae 

```C++
int64_t SYN_ControlStats::sum_itae;
```



Integral of Time-weighted Absolute Error 


        

<hr>



### variable sum\_sq\_err 

```C++
int64_t SYN_ControlStats::sum_sq_err;
```



Accumulator for RMS Error 
 


        

<hr>



### variable sum\_sq\_out 

```C++
int64_t SYN_ControlStats::sum_sq_out;
```



Accumulator for Control Effort (RMS Out) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_control_stats.h`

