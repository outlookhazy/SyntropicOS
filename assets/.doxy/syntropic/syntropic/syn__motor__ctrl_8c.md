

# File syn\_motor\_ctrl.c



[**FileList**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_motor\_ctrl.c**](syn__motor__ctrl_8c.md)

[Go to the source code of this file](syn__motor__ctrl_8c_source.md)

_Closed-loop motor controller implementation._ [More...](#detailed-description)

* `#include "syn_motor_ctrl.h"`
* `#include "../util/syn_assert.h"`
* `#include "../system/syn_errlog.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_motor\_ctrl\_clear\_stall**](#function-syn_motor_ctrl_clear_stall) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Clear a stall condition and return to idle._  |
|  void | [**syn\_motor\_ctrl\_estop**](#function-syn_motor_ctrl_estop) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Emergency stop — immediate brake/coast._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_motor\_ctrl\_init**](#function-syn_motor_ctrl_init) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl, const [**SYN\_MotorCtrl\_Config**](structSYN__MotorCtrl__Config.md) \* cfg) <br>_Initialize the closed-loop controller._  |
|  void | [**syn\_motor\_ctrl\_move\_to**](#function-syn_motor_ctrl_move_to) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl, int32\_t target, int32\_t max\_velocity, int32\_t acceleration) <br>_Move to position with a built-in trapezoidal velocity profile._  |
|  void | [**syn\_motor\_ctrl\_move\_to\_scurve**](#function-syn_motor_ctrl_move_to_scurve) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl, int32\_t target, int32\_t max\_velocity, int32\_t max\_accel, int32\_t max\_jerk) <br>_Move to position with a built-in jerk-limited S-curve profile._  |
|  void | [**syn\_motor\_ctrl\_on\_stall**](#function-syn_motor_ctrl_on_stall) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl, [**SYN\_MotorCtrl\_StallCallback**](syn__motor__ctrl_8h.md#typedef-syn_motorctrl_stallcallback) cb, void \* ctx) <br>_Register stall callback._  |
|  void | [**syn\_motor\_ctrl\_on\_target**](#function-syn_motor_ctrl_on_target) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl, [**SYN\_MotorCtrl\_TargetCallback**](syn__motor__ctrl_8h.md#typedef-syn_motorctrl_targetcallback) cb, void \* ctx) <br>_Register on-target callback (position mode)._  |
|  void | [**syn\_motor\_ctrl\_reset\_metrics**](#function-syn_motor_ctrl_reset_metrics) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Reset move metrics. Call before starting a move you want to measure._  |
|  int32\_t | [**syn\_motor\_ctrl\_rms\_error**](#function-syn_motor_ctrl_rms_error) (const [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Compute RMS tracking error from accumulated metrics._  |
|  void | [**syn\_motor\_ctrl\_set\_datalog**](#function-syn_motor_ctrl_set_datalog) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl, [**SYN\_DataLog**](structSYN__DataLog.md) \* log) <br>_Attach a datalog for tuning capture._  |
|  void | [**syn\_motor\_ctrl\_set\_ff\_gains**](#function-syn_motor_ctrl_set_ff_gains) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl, int32\_t ff\_kv, int32\_t ff\_ka) <br>_Update feedforward gains at runtime._  |
|  void | [**syn\_motor\_ctrl\_set\_gains**](#function-syn_motor_ctrl_set_gains) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl, int32\_t kp, int32\_t ki, int32\_t kd) <br>_Update PID gains at runtime._  |
|  void | [**syn\_motor\_ctrl\_set\_output**](#function-syn_motor_ctrl_set_output) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl, int32\_t output) <br>_Drive the motor at a fixed output level (open-loop)._  |
|  void | [**syn\_motor\_ctrl\_set\_position**](#function-syn_motor_ctrl_set_position) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl, int32\_t target) <br>_Set position target (absolute feedback units)._  |
|  void | [**syn\_motor\_ctrl\_set\_trajectory**](#function-syn_motor_ctrl_set_trajectory) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl, const [**SYN\_MotorCtrl\_Trajectory**](structSYN__MotorCtrl__Trajectory.md) \* traj) <br>_Feed a trajectory point for feedforward + PID tracking._  |
|  void | [**syn\_motor\_ctrl\_set\_velocity**](#function-syn_motor_ctrl_set_velocity) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl, int32\_t units\_per\_sec) <br>_Set velocity target (units/second)._  |
|  void | [**syn\_motor\_ctrl\_stop**](#function-syn_motor_ctrl_stop) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Stop the motor and enter idle mode._  |
|  [**SYN\_MotorCtrl\_State**](syn__motor__ctrl_8h.md#enum-syn_motorctrl_state) | [**syn\_motor\_ctrl\_update**](#function-syn_motor_ctrl_update) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Run the control loop. Call at cfg.update\_hz rate._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**apply\_output**](#function-apply_output) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl, int32\_t output) <br>_Drive the motor at the given output level via the vtable._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**at\_limit**](#function-at_limit) (const [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl, int32\_t pos, int32\_t output) <br>_Check if position is at a soft limit in the given direction._  |
|  void | [**brake\_motor**](#function-brake_motor) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Active-brake the motor._  |
|  int32\_t | [**isqrt**](#function-isqrt) (int64\_t n) <br>_Integer square root via binary search._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**limits\_enabled**](#function-limits_enabled) (const [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Check if soft position limits are configured._  |
|  int32\_t | [**read\_position**](#function-read_position) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Read the current position from the encoder/sensor._  |
|  void | [**stop\_motor**](#function-stop_motor) ([**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Coast/stop the motor (no active braking)._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_MCTRL\_ERR\_LIMIT**](syn__motor__ctrl_8c.md#define-syn_mctrl_err_limit)  `0x0101`<br> |
| define  | [**SYN\_MCTRL\_ERR\_STALL**](syn__motor__ctrl_8c.md#define-syn_mctrl_err_stall)  `0x0100`<br> |

## Detailed Description


Control loop:
* Read feedback via read\_pos() function pointer
* Compute velocity = delta × update\_hz
* Compute PID error (velocity or position mode)
* Enforce soft position limits
* Apply PID output to motor via [**SYN\_MotorOutput**](structSYN__MotorOutput.md) interface
* Check stall condition 




    
## Public Functions Documentation




### function syn\_motor\_ctrl\_clear\_stall 

_Clear a stall condition and return to idle._ 
```C++
void syn_motor_ctrl_clear_stall (
    SYN_MotorCtrl * ctrl
) 
```



After a stall, the controller locks in STALLED state until this is called.




**Parameters:**


* `ctrl` Controller instance. 




        

<hr>



### function syn\_motor\_ctrl\_estop 

_Emergency stop — immediate brake/coast._ 
```C++
void syn_motor_ctrl_estop (
    SYN_MotorCtrl * ctrl
) 
```





**Parameters:**


* `ctrl` Controller instance. 




        

<hr>



### function syn\_motor\_ctrl\_init 

_Initialize the closed-loop controller._ 
```C++
SYN_Status syn_motor_ctrl_init (
    SYN_MotorCtrl * ctrl,
    const SYN_MotorCtrl_Config * cfg
) 
```





**Parameters:**


* `ctrl` Controller instance to initialize. 
* `cfg` Configuration (copied internally). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_motor\_ctrl\_move\_to 

_Move to position with a built-in trapezoidal velocity profile._ 
```C++
void syn_motor_ctrl_move_to (
    SYN_MotorCtrl * ctrl,
    int32_t target,
    int32_t max_velocity,
    int32_t acceleration
) 
```



This is the "batteries included" path — no external profile generator needed. The controller internally generates a trapezoidal ramp (accelerate → cruise → decelerate) and feeds it through the normal trajectory/feedforward path each update() call.


Equivalent to manually calling [**syn\_ramp\_set\_target\_trapezoid()**](syn__ramp_8c.md#function-syn_ramp_set_target_trapezoid) and feeding the output into set\_trajectory() each tick, but with zero user-side wiring.




**Parameters:**


* `ctrl` Controller instance. 
* `target` Target position in feedback units. 
* `max_velocity` Maximum velocity (units/second). 
* `acceleration` Acceleration (units/second²). 




        

<hr>



### function syn\_motor\_ctrl\_move\_to\_scurve 

_Move to position with a built-in jerk-limited S-curve profile._ 
```C++
void syn_motor_ctrl_move_to_scurve (
    SYN_MotorCtrl * ctrl,
    int32_t target,
    int32_t max_velocity,
    int32_t max_accel,
    int32_t max_jerk
) 
```



Like move\_to(), but uses a 7-phase S-curve trajectory that bounds jerk for the smoothest possible motion. The S-curve natively provides position, velocity, and acceleration — all three are fed into the trajectory/feedforward path.


All kinematic limits are in per-second units:
* max\_velocity: units/second
* max\_accel: units/second²
* max\_jerk: units/second³






**Parameters:**


* `ctrl` Controller instance. 
* `target` Target position in feedback units. 
* `max_velocity` Maximum velocity (units/second). 
* `max_accel` Maximum acceleration (units/second²). 
* `max_jerk` Maximum jerk (units/second³). 




        

<hr>



### function syn\_motor\_ctrl\_on\_stall 

_Register stall callback._ 
```C++
void syn_motor_ctrl_on_stall (
    SYN_MotorCtrl * ctrl,
    SYN_MotorCtrl_StallCallback cb,
    void * ctx
) 
```





**Parameters:**


* `ctrl` Controller instance. 
* `cb` Callback function, or NULL to unregister. 
* `ctx` User context passed to callback. 




        

<hr>



### function syn\_motor\_ctrl\_on\_target 

_Register on-target callback (position mode)._ 
```C++
void syn_motor_ctrl_on_target (
    SYN_MotorCtrl * ctrl,
    SYN_MotorCtrl_TargetCallback cb,
    void * ctx
) 
```





**Parameters:**


* `ctrl` Controller instance. 
* `cb` Callback function, or NULL to unregister. 
* `ctx` User context passed to callback. 




        

<hr>



### function syn\_motor\_ctrl\_reset\_metrics 

_Reset move metrics. Call before starting a move you want to measure._ 
```C++
void syn_motor_ctrl_reset_metrics (
    SYN_MotorCtrl * ctrl
) 
```





**Parameters:**


* `ctrl` Controller instance. 




        

<hr>



### function syn\_motor\_ctrl\_rms\_error 

_Compute RMS tracking error from accumulated metrics._ 
```C++
int32_t syn_motor_ctrl_rms_error (
    const SYN_MotorCtrl * ctrl
) 
```



Returns the integer square root of (error\_sq\_sum / sample\_count). Only valid if sample\_count &gt; 0.




**Parameters:**


* `ctrl` Controller instance. 



**Returns:**

RMS tracking error in feedback units. 





        

<hr>



### function syn\_motor\_ctrl\_set\_datalog 

_Attach a datalog for tuning capture._ 
```C++
void syn_motor_ctrl_set_datalog (
    SYN_MotorCtrl * ctrl,
    SYN_DataLog * log
) 
```



When attached, every update() writes a [**SYN\_MotorCtrl\_Sample**](structSYN__MotorCtrl__Sample.md) frame to the datalog at full control-loop rate.




**Parameters:**


* `ctrl` Controller. 
* `log` Datalog instance (caller-owned), or NULL to detach. 




        

<hr>



### function syn\_motor\_ctrl\_set\_ff\_gains 

_Update feedforward gains at runtime._ 
```C++
void syn_motor_ctrl_set_ff_gains (
    SYN_MotorCtrl * ctrl,
    int32_t ff_kv,
    int32_t ff_ka
) 
```





**Parameters:**


* `ctrl` Controller instance. 
* `ff_kv` Velocity feedforward gain (÷ 1 &lt;&lt; ff\_scale). 
* `ff_ka` Acceleration feedforward gain (÷ 1 &lt;&lt; ff\_scale). 




        

<hr>



### function syn\_motor\_ctrl\_set\_gains 

_Update PID gains at runtime._ 
```C++
void syn_motor_ctrl_set_gains (
    SYN_MotorCtrl * ctrl,
    int32_t kp,
    int32_t ki,
    int32_t kd
) 
```





**Parameters:**


* `ctrl` Controller instance. 
* `kp` Proportional gain (÷ 1 &lt;&lt; pid\_scale). 
* `ki` Integral gain (÷ 1 &lt;&lt; pid\_scale). 
* `kd` Derivative gain (÷ 1 &lt;&lt; pid\_scale). 




        

<hr>



### function syn\_motor\_ctrl\_set\_output 

_Drive the motor at a fixed output level (open-loop)._ 
```C++
void syn_motor_ctrl_set_output (
    SYN_MotorCtrl * ctrl,
    int32_t output
) 
```



Bypasses PID — the output value is passed directly to the motor. Useful for manual jogging, testing motor wiring, or when simple open-loop control is sufficient. Position feedback is still read (for soft limits and stall detection) but not used for control.




**Parameters:**


* `ctrl` Controller instance. 
* `output` Output level (clamped to [output\_min, output\_max]). 




        

<hr>



### function syn\_motor\_ctrl\_set\_position 

_Set position target (absolute feedback units)._ 
```C++
void syn_motor_ctrl_set_position (
    SYN_MotorCtrl * ctrl,
    int32_t target
) 
```



Switches to position mode. Controller drives toward target and enters ON\_TARGET state when within deadband.




**Parameters:**


* `ctrl` Controller instance. 
* `target` Absolute position in feedback units. 




        

<hr>



### function syn\_motor\_ctrl\_set\_trajectory 

_Feed a trajectory point for feedforward + PID tracking._ 
```C++
void syn_motor_ctrl_set_trajectory (
    SYN_MotorCtrl * ctrl,
    const SYN_MotorCtrl_Trajectory * traj
) 
```



Call this each update with the current output of your trajectory generator (syn\_ramp, syn\_scurve, or application code). The controller uses PID on position error plus feedforward from the trajectory's velocity and acceleration.




**Parameters:**


* `ctrl` Controller instance. 
* `traj` Trajectory point (position, velocity, acceleration). 




        

<hr>



### function syn\_motor\_ctrl\_set\_velocity 

_Set velocity target (units/second)._ 
```C++
void syn_motor_ctrl_set_velocity (
    SYN_MotorCtrl * ctrl,
    int32_t units_per_sec
) 
```



Switches to velocity mode. Positive = forward, negative = reverse.




**Parameters:**


* `ctrl` Controller instance. 
* `units_per_sec` Target velocity in feedback units per second. 




        

<hr>



### function syn\_motor\_ctrl\_stop 

_Stop the motor and enter idle mode._ 
```C++
void syn_motor_ctrl_stop (
    SYN_MotorCtrl * ctrl
) 
```





**Parameters:**


* `ctrl` Controller instance. 




        

<hr>



### function syn\_motor\_ctrl\_update 

_Run the control loop. Call at cfg.update\_hz rate._ 
```C++
SYN_MotorCtrl_State syn_motor_ctrl_update (
    SYN_MotorCtrl * ctrl
) 
```



Reads feedback, computes PID, drives motor output. If a datalog is attached, writes a [**SYN\_MotorCtrl\_Sample**](structSYN__MotorCtrl__Sample.md) each call.




**Parameters:**


* `ctrl` Controller instance. 



**Returns:**

Current controller state. 





        

<hr>
## Public Static Functions Documentation




### function apply\_output 

_Drive the motor at the given output level via the vtable._ 
```C++
static void apply_output (
    SYN_MotorCtrl * ctrl,
    int32_t output
) 
```





**Parameters:**


* `ctrl` Motor controller. 
* `output` Signed output value. 




        

<hr>



### function at\_limit 

_Check if position is at a soft limit in the given direction._ 
```C++
static bool at_limit (
    const SYN_MotorCtrl * ctrl,
    int32_t pos,
    int32_t output
) 
```





**Parameters:**


* `ctrl` Motor controller. 
* `pos` Current position. 
* `output` Intended output (sign indicates direction). 



**Returns:**

true if movement would exceed a limit. 





        

<hr>



### function brake\_motor 

_Active-brake the motor._ 
```C++
static void brake_motor (
    SYN_MotorCtrl * ctrl
) 
```





**Parameters:**


* `ctrl` Motor controller. 




        

<hr>



### function isqrt 

_Integer square root via binary search._ 
```C++
static int32_t isqrt (
    int64_t n
) 
```





**Parameters:**


* `n` Input value. 



**Returns:**

Floor of sqrt(n). 





        

<hr>



### function limits\_enabled 

_Check if soft position limits are configured._ 
```C++
static bool limits_enabled (
    const SYN_MotorCtrl * ctrl
) 
```





**Parameters:**


* `ctrl` Motor controller. 



**Returns:**

true if limits are set. 





        

<hr>



### function read\_position 

_Read the current position from the encoder/sensor._ 
```C++
static int32_t read_position (
    SYN_MotorCtrl * ctrl
) 
```





**Parameters:**


* `ctrl` Motor controller. 



**Returns:**

Current position. 





        

<hr>



### function stop\_motor 

_Coast/stop the motor (no active braking)._ 
```C++
static void stop_motor (
    SYN_MotorCtrl * ctrl
) 
```





**Parameters:**


* `ctrl` Motor controller. 




        

<hr>
## Macro Definition Documentation





### define SYN\_MCTRL\_ERR\_LIMIT 

```C++
#define SYN_MCTRL_ERR_LIMIT `0x0101`
```



Soft position limit hit. 
 


        

<hr>



### define SYN\_MCTRL\_ERR\_STALL 

```C++
#define SYN_MCTRL_ERR_STALL `0x0100`
```



Stall condition detected. 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_motor_ctrl.c`

