

# Struct SYN\_MotorCtrl



[**ClassList**](annotated.md) **>** [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md)



_Motor controller instance (opaque — use API to access)._ 

* `#include <syn_motor_ctrl.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_MotorCtrl\_Config**](structSYN__MotorCtrl__Config.md) | [**cfg**](#variable-cfg)  <br> |
|  [**SYN\_DataLog**](structSYN__DataLog.md) \* | [**datalog**](#variable-datalog)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**enabled**](#variable-enabled)  <br>_True when the controller is enabled and driving output._  |
|  int32\_t | [**ff\_output**](#variable-ff_output)  <br> |
|  int32\_t | [**last\_position**](#variable-last_position)  <br> |
|  uint32\_t | [**last\_update\_tick**](#variable-last_update_tick)  <br>_Tick of last update() call._  |
|  int32\_t | [**measured\_position**](#variable-measured_position)  <br> |
|  int32\_t | [**measured\_velocity**](#variable-measured_velocity)  <br> |
|  [**SYN\_MotorCtrl\_Metrics**](structSYN__MotorCtrl__Metrics.md) | [**metrics**](#variable-metrics)  <br>_Move metrics — accumulated during each move, zero buffer cost._  |
|  [**SYN\_MotorCtrl\_Mode**](syn__motor__ctrl_8h.md#enum-syn_motorctrl_mode) | [**mode**](#variable-mode)  <br> |
|  [**SYN\_MotorCtrl\_StallCallback**](syn__motor__ctrl_8h.md#typedef-syn_motorctrl_stallcallback) | [**on\_stall**](#variable-on_stall)  <br>_Registered stall callback (NULL = none)._  |
|  void \* | [**on\_stall\_ctx**](#variable-on_stall_ctx)  <br>_User context passed to stall callback._  |
|  [**SYN\_MotorCtrl\_TargetCallback**](syn__motor__ctrl_8h.md#typedef-syn_motorctrl_targetcallback) | [**on\_target**](#variable-on_target)  <br>_Registered on-target callback (NULL = none)._  |
|  void \* | [**on\_target\_ctx**](#variable-on_target_ctx)  <br>_User context passed to on-target callback._  |
|  [**SYN\_PID**](structSYN__PID.md) | [**pid**](#variable-pid)  <br> |
|  int32\_t | [**pid\_output**](#variable-pid_output)  <br> |
|  [**SYN\_Ramp**](structSYN__Ramp.md) | [**profile**](#variable-profile)  <br>_Built-in ramp for move\_to() (not used when set\_trajectory is active)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**profile\_active**](#variable-profile_active)  <br>_True when the built-in ramp profile is actively driving._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**scurve\_active**](#variable-scurve_active)  <br>_True when the built-in S-curve profile is actively driving._  |
|  [**SYN\_SCurve**](structSYN__SCurve.md) | [**scurve\_profile**](#variable-scurve_profile)  <br>_Built-in S-curve for move\_to\_scurve()._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**stall\_active**](#variable-stall_active)  <br>_True while output is nonzero but position isn't changing._  |
|  uint32\_t | [**stall\_start\_tick**](#variable-stall_start_tick)  <br>_Tick when stall condition was first detected._  |
|  [**SYN\_MotorCtrl\_State**](syn__motor__ctrl_8h.md#enum-syn_motorctrl_state) | [**state**](#variable-state)  <br> |
|  int32\_t | [**target\_position**](#variable-target_position)  <br> |
|  int32\_t | [**target\_velocity**](#variable-target_velocity)  <br> |
|  int32\_t | [**total\_output**](#variable-total_output)  <br> |
|  [**SYN\_MotorCtrl\_Trajectory**](structSYN__MotorCtrl__Trajectory.md) | [**trajectory**](#variable-trajectory)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**trajectory\_active**](#variable-trajectory_active)  <br> |












































## Public Attributes Documentation




### variable cfg 

```C++
SYN_MotorCtrl_Config SYN_MotorCtrl::cfg;
```



Configuration snapshot 
 


        

<hr>



### variable datalog 

```C++
SYN_DataLog* SYN_MotorCtrl::datalog;
```



Attached datalog for telemetry 


        

<hr>



### variable enabled 

_True when the controller is enabled and driving output._ 
```C++
bool SYN_MotorCtrl::enabled;
```




<hr>



### variable ff\_output 

```C++
int32_t SYN_MotorCtrl::ff_output;
```



Last feedforward output 
 


        

<hr>



### variable last\_position 

```C++
int32_t SYN_MotorCtrl::last_position;
```



Position at last update 


        

<hr>



### variable last\_update\_tick 

_Tick of last update() call._ 
```C++
uint32_t SYN_MotorCtrl::last_update_tick;
```




<hr>



### variable measured\_position 

```C++
int32_t SYN_MotorCtrl::measured_position;
```



Current position 
 


        

<hr>



### variable measured\_velocity 

```C++
int32_t SYN_MotorCtrl::measured_velocity;
```



Current velocity 
 


        

<hr>



### variable metrics 

_Move metrics — accumulated during each move, zero buffer cost._ 
```C++
SYN_MotorCtrl_Metrics SYN_MotorCtrl::metrics;
```




<hr>



### variable mode 

```C++
SYN_MotorCtrl_Mode SYN_MotorCtrl::mode;
```



Current operating mode 
 


        

<hr>



### variable on\_stall 

_Registered stall callback (NULL = none)._ 
```C++
SYN_MotorCtrl_StallCallback SYN_MotorCtrl::on_stall;
```




<hr>



### variable on\_stall\_ctx 

_User context passed to stall callback._ 
```C++
void* SYN_MotorCtrl::on_stall_ctx;
```




<hr>



### variable on\_target 

_Registered on-target callback (NULL = none)._ 
```C++
SYN_MotorCtrl_TargetCallback SYN_MotorCtrl::on_target;
```




<hr>



### variable on\_target\_ctx 

_User context passed to on-target callback._ 
```C++
void* SYN_MotorCtrl::on_target_ctx;
```




<hr>



### variable pid 

```C++
SYN_PID SYN_MotorCtrl::pid;
```



Embedded PID controller 
 


        

<hr>



### variable pid\_output 

```C++
int32_t SYN_MotorCtrl::pid_output;
```



Last PID output 
 


        

<hr>



### variable profile 

_Built-in ramp for move\_to() (not used when set\_trajectory is active)._ 
```C++
SYN_Ramp SYN_MotorCtrl::profile;
```




<hr>



### variable profile\_active 

_True when the built-in ramp profile is actively driving._ 
```C++
bool SYN_MotorCtrl::profile_active;
```




<hr>



### variable scurve\_active 

_True when the built-in S-curve profile is actively driving._ 
```C++
bool SYN_MotorCtrl::scurve_active;
```




<hr>



### variable scurve\_profile 

_Built-in S-curve for move\_to\_scurve()._ 
```C++
SYN_SCurve SYN_MotorCtrl::scurve_profile;
```




<hr>



### variable stall\_active 

_True while output is nonzero but position isn't changing._ 
```C++
bool SYN_MotorCtrl::stall_active;
```




<hr>



### variable stall\_start\_tick 

_Tick when stall condition was first detected._ 
```C++
uint32_t SYN_MotorCtrl::stall_start_tick;
```




<hr>



### variable state 

```C++
SYN_MotorCtrl_State SYN_MotorCtrl::state;
```



Current runtime state 
 


        

<hr>



### variable target\_position 

```C++
int32_t SYN_MotorCtrl::target_position;
```



Target position value 
 


        

<hr>



### variable target\_velocity 

```C++
int32_t SYN_MotorCtrl::target_velocity;
```



Units per second 
 


        

<hr>



### variable total\_output 

```C++
int32_t SYN_MotorCtrl::total_output;
```



Last combined output 
 


        

<hr>



### variable trajectory 

```C++
SYN_MotorCtrl_Trajectory SYN_MotorCtrl::trajectory;
```



Current trajectory point 


        

<hr>



### variable trajectory\_active 

```C++
bool SYN_MotorCtrl::trajectory_active;
```



True when using set\_trajectory 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_motor_ctrl.h`

