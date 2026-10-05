

# Struct SYN\_MotorCtrl\_Trajectory



[**ClassList**](annotated.md) **>** [**SYN\_MotorCtrl\_Trajectory**](structSYN__MotorCtrl__Trajectory.md)



_Trajectory setpoint for feedforward control._ [More...](#detailed-description)

* `#include <syn_motor_ctrl.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int32\_t | [**acceleration**](#variable-acceleration)  <br> |
|  int32\_t | [**position**](#variable-position)  <br> |
|  int32\_t | [**velocity**](#variable-velocity)  <br> |












































## Detailed Description


Produced by an external trajectory generator (syn\_ramp, syn\_scurve, or application code) and fed to the controller each update. 


    
## Public Attributes Documentation




### variable acceleration 

```C++
int32_t SYN_MotorCtrl_Trajectory::acceleration;
```



Target acceleration (units/sec²) 
 


        

<hr>



### variable position 

```C++
int32_t SYN_MotorCtrl_Trajectory::position;
```



Target position (feedback units) 
 


        

<hr>



### variable velocity 

```C++
int32_t SYN_MotorCtrl_Trajectory::velocity;
```



Target velocity (units/sec) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_motor_ctrl.h`

