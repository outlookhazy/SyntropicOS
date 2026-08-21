

# Struct SYN\_MotorCtrl\_Metrics



[**ClassList**](annotated.md) **>** [**SYN\_MotorCtrl\_Metrics**](structSYN__MotorCtrl__Metrics.md)



_Accumulated metrics for a single move/trajectory._ [More...](#detailed-description)

* `#include <syn_motor_ctrl.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int64\_t | [**error\_sq\_sum**](#variable-error_sq_sum)  <br> |
|  int32\_t | [**max\_error**](#variable-max_error)  <br> |
|  uint32\_t | [**move\_start\_tick**](#variable-move_start_tick)  <br> |
|  int32\_t | [**overshoot**](#variable-overshoot)  <br> |
|  int32\_t | [**peak\_output**](#variable-peak_output)  <br> |
|  uint32\_t | [**sample\_count**](#variable-sample_count)  <br> |
|  uint32\_t | [**settle\_tick**](#variable-settle_tick)  <br> |












































## Detailed Description


Updated every update() call. Read back after a move completes to evaluate tracking performance without storing per-sample data. Call [**syn\_motor\_ctrl\_reset\_metrics()**](syn__motor__ctrl_8c.md#function-syn_motor_ctrl_reset_metrics) before a move to start fresh. 


    
## Public Attributes Documentation




### variable error\_sq\_sum 

```C++
int64_t SYN_MotorCtrl_Metrics::error_sq_sum;
```



Sum of error² (for RMS computation) 
 


        

<hr>



### variable max\_error 

```C++
int32_t SYN_MotorCtrl_Metrics::max_error;
```



Peak \|position\_error\| during move 
 


        

<hr>



### variable move\_start\_tick 

```C++
uint32_t SYN_MotorCtrl_Metrics::move_start_tick;
```



Tick when move started 
 


        

<hr>



### variable overshoot 

```C++
int32_t SYN_MotorCtrl_Metrics::overshoot;
```



Max overshoot past target (position) 
 


        

<hr>



### variable peak\_output 

```C++
int32_t SYN_MotorCtrl_Metrics::peak_output;
```



Peak \|output\| applied to motor 
 


        

<hr>



### variable sample\_count 

```C++
uint32_t SYN_MotorCtrl_Metrics::sample_count;
```



Number of update() calls in this move 
 


        

<hr>



### variable settle\_tick 

```C++
uint32_t SYN_MotorCtrl_Metrics::settle_tick;
```



Tick when first entered deadband (0=never) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_motor_ctrl.h`

