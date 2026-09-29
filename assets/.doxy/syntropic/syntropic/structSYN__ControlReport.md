

# Struct SYN\_ControlReport



[**ClassList**](annotated.md) **>** [**SYN\_ControlReport**](structSYN__ControlReport.md)



_Human-readable performance report._ 

* `#include <syn_control_stats.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int32\_t | [**control\_effort**](#variable-control_effort)  <br> |
|  int32\_t | [**itae**](#variable-itae)  <br> |
|  int32\_t | [**jitter**](#variable-jitter)  <br> |
|  int32\_t | [**max\_error**](#variable-max_error)  <br> |
|  int32\_t | [**rms\_error**](#variable-rms_error)  <br> |












































## Public Attributes Documentation




### variable control\_effort 

```C++
int32_t SYN_ControlReport::control_effort;
```



RMS of output (scaled 0-10000 for 0-100%) 


        

<hr>



### variable itae 

```C++
int32_t SYN_ControlReport::itae;
```



ITAE score (lower is better) 
 


        

<hr>



### variable jitter 

```C++
int32_t SYN_ControlReport::jitter;
```



Average absolute output delta (noise) 
 


        

<hr>



### variable max\_error 

```C++
int32_t SYN_ControlReport::max_error;
```



Maximum absolute error (counts) 
 


        

<hr>



### variable rms\_error 

```C++
int32_t SYN_ControlReport::rms_error;
```



Root Mean Square error (counts) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_control_stats.h`

