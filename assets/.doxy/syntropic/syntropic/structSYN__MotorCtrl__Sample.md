

# Struct SYN\_MotorCtrl\_Sample



[**ClassList**](annotated.md) **>** [**SYN\_MotorCtrl\_Sample**](structSYN__MotorCtrl__Sample.md)



_One sample of control-loop telemetry._ [More...](#detailed-description)

* `#include <syn_motor_ctrl.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int32\_t | [**ff\_output**](#variable-ff_output)  <br> |
|  int32\_t | [**measured\_pos**](#variable-measured_pos)  <br> |
|  int32\_t | [**measured\_vel**](#variable-measured_vel)  <br> |
|  int32\_t | [**pid\_output**](#variable-pid_output)  <br> |
|  int32\_t | [**target\_pos**](#variable-target_pos)  <br> |
|  int32\_t | [**target\_vel**](#variable-target_vel)  <br> |
|  uint32\_t | [**tick\_ms**](#variable-tick_ms)  <br> |
|  int32\_t | [**total\_output**](#variable-total_output)  <br> |












































## Detailed Description


Written to the attached [**SYN\_DataLog**](structSYN__DataLog.md) every update() call. At 1000 Hz with a 4 KB buffer you get ~1.2 seconds of capture. 


    
## Public Attributes Documentation




### variable ff\_output 

```C++
int32_t SYN_MotorCtrl_Sample::ff_output;
```



Feedforward contribution 
 


        

<hr>



### variable measured\_pos 

```C++
int32_t SYN_MotorCtrl_Sample::measured_pos;
```



Measured position (from feedback) 
 


        

<hr>



### variable measured\_vel 

```C++
int32_t SYN_MotorCtrl_Sample::measured_vel;
```



Measured velocity 
 


        

<hr>



### variable pid\_output 

```C++
int32_t SYN_MotorCtrl_Sample::pid_output;
```



PID feedback contribution 
 


        

<hr>



### variable target\_pos 

```C++
int32_t SYN_MotorCtrl_Sample::target_pos;
```



Trajectory/target position 
 


        

<hr>



### variable target\_vel 

```C++
int32_t SYN_MotorCtrl_Sample::target_vel;
```



Trajectory/target velocity 
 


        

<hr>



### variable tick\_ms 

```C++
uint32_t SYN_MotorCtrl_Sample::tick_ms;
```



Timestamp 
 


        

<hr>



### variable total\_output 

```C++
int32_t SYN_MotorCtrl_Sample::total_output;
```



Final clamped output to motor 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_motor_ctrl.h`

