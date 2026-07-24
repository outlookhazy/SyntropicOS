

# File syn\_motor\_ctrl.h



[**FileList**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_motor\_ctrl.h**](syn__motor__ctrl_8h.md)

[Go to the source code of this file](syn__motor__ctrl_8h_source.md)

_Closed-loop motor controller — generic feedback + PID + motor._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../control/syn_pid.h"`
* `#include "../motor/syn_motor_output.h"`
* `#include "../util/syn_ramp.h"`
* `#include "../util/syn_scurve.h"`
* `#include "../port/syn_port_system.h"`
* `#include "../system/syn_errlog.h"`
* `#include "../log/syn_datalog.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) <br>_Motor controller instance (opaque — use API to access)._  |
| struct | [**SYN\_MotorCtrl\_Config**](structSYN__MotorCtrl__Config.md) <br>_Motor controller configuration (passed to init, copied internally)._  |
| struct | [**SYN\_MotorCtrl\_Metrics**](structSYN__MotorCtrl__Metrics.md) <br>_Accumulated metrics for a single move/trajectory._  |
| struct | [**SYN\_MotorCtrl\_Sample**](structSYN__MotorCtrl__Sample.md) <br>_One sample of control-loop telemetry._  |
| struct | [**SYN\_MotorCtrl\_Trajectory**](structSYN__MotorCtrl__Trajectory.md) <br>_Trajectory setpoint for feedforward control._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_MotorCtrl\_Mode**](#enum-syn_motorctrl_mode)  <br>_Control loop operating mode._  |
| typedef int32\_t(\* | [**SYN\_MotorCtrl\_ReadPos**](#typedef-syn_motorctrl_readpos)  <br>_Position feedback function._  |
| typedef void(\* | [**SYN\_MotorCtrl\_StallCallback**](#typedef-syn_motorctrl_stallcallback)  <br>_Callback invoked when a motor stall is detected._  |
| enum  | [**SYN\_MotorCtrl\_State**](#enum-syn_motorctrl_state)  <br>_Motor controller runtime state._  |
| typedef void(\* | [**SYN\_MotorCtrl\_TargetCallback**](#typedef-syn_motorctrl_targetcallback)  <br>_Callback invoked when position reaches the target deadband._  |




















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
|  int32\_t | [**syn\_motor\_ctrl\_error**](#function-syn_motor_ctrl_error) (const [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Get last position error._  |
|  int32\_t | [**syn\_motor\_ctrl\_ff\_output**](#function-syn_motor_ctrl_ff_output) (const [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Get last feedforward output component._  |
|  const [**SYN\_MotorCtrl\_Metrics**](structSYN__MotorCtrl__Metrics.md) \* | [**syn\_motor\_ctrl\_get\_metrics**](#function-syn_motor_ctrl_get_metrics) (const [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Get accumulated move metrics._  |
|  [**SYN\_MotorCtrl\_Mode**](syn__motor__ctrl_8h.md#enum-syn_motorctrl_mode) | [**syn\_motor\_ctrl\_mode**](#function-syn_motor_ctrl_mode) (const [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Get the current operating mode._  |
|  uint32\_t | [**syn\_motor\_ctrl\_move\_duration**](#function-syn_motor_ctrl_move_duration) (const [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Get move duration in milliseconds._  |
|  int32\_t | [**syn\_motor\_ctrl\_output**](#function-syn_motor_ctrl_output) (const [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Get total output (PID + feedforward)._  |
|  int32\_t | [**syn\_motor\_ctrl\_pid\_output**](#function-syn_motor_ctrl_pid_output) (const [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Get last PID output component._  |
|  int32\_t | [**syn\_motor\_ctrl\_position**](#function-syn_motor_ctrl_position) (const [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Get measured position._  |
|  uint32\_t | [**syn\_motor\_ctrl\_settle\_time**](#function-syn_motor_ctrl_settle_time) (const [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Get settling time in milliseconds (0 if never settled)._  |
|  [**SYN\_MotorCtrl\_State**](syn__motor__ctrl_8h.md#enum-syn_motorctrl_state) | [**syn\_motor\_ctrl\_state**](#function-syn_motor_ctrl_state) (const [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Get the current controller state._  |
|  int32\_t | [**syn\_motor\_ctrl\_velocity**](#function-syn_motor_ctrl_velocity) (const [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl) <br>_Get measured velocity._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_MCTRL\_DATALOG\_ID**](syn__motor__ctrl_8h.md#define-syn_mctrl_datalog_id)  `0x4D43  /\* 'MC' \*/`<br> |
| define  | [**SYN\_MOTOR\_CTRL\_DEFAULTS**](syn__motor__ctrl_8h.md#define-syn_motor_ctrl_defaults) (motor\_out, read\_fn, read\_ctx, hz, out\_max) `/* multi line expression */`<br>_Convenience macro for a motor controller config with sane defaults._  |

## Detailed Description


Combines any position/velocity feedback source with PID control to drive DC or stepper motors in closed loop. The feedback source is a function pointer, so it works equally well with:
* Rotary encoder (syn\_encoder)
* Potentiometer / linear pot on ADC (syn\_adc)
* Hall sensor, absolute encoder, or any other position source






**
**


* **Velocity**: PID maintains target units/second
* **Position**: PID drives to target position value
* **Trajectory**: PID + feedforward tracks a trajectory from an external generator (syn\_ramp, syn\_scurve, or application code)






**
**


```C++
// Feedback: read encoder position
int32_t encoder_feedback(void *ctx) {
    return syn_encoder_position((SYN_Encoder *)ctx);
}

SYN_MotorCtrl ctrl;
SYN_MotorCtrl_Config cfg = {
    .motor       = syn_dc_motor_output(&my_dc),
    .read_pos    = encoder_feedback,
    .read_pos_ctx = &my_encoder,
    .pid_kp = 200, .pid_ki = 50, .pid_kd = 10, .pid_scale = 8,
    .update_hz     = 1000,
    .output_min  = -1000, .output_max = 1000,
};
syn_motor_ctrl_init(&ctrl, &cfg);
syn_motor_ctrl_set_velocity(&ctrl, 500);  // 500 counts/sec
```





**
**


```C++
SYN_MotorCtrl_Config cfg = {
    // ... feedback, motor, PID as above ...
    .ff_kv    = 128,   // velocity feedforward gain
    .ff_ka    = 64,    // acceleration feedforward gain
    .ff_scale = 8,     // feedforward divisor = 1 << 8 = 256
};
syn_motor_ctrl_init(&ctrl, &cfg);

// In your control loop, feed trajectory from any generator:
SYN_MotorCtrl_Trajectory traj = {
    .position     = profile_position,
    .velocity     = profile_velocity,
    .acceleration = profile_acceleration,
};
syn_motor_ctrl_set_trajectory(&ctrl, &traj);
syn_motor_ctrl_update(&ctrl);
```
 





    
## Public Types Documentation




### enum SYN\_MotorCtrl\_Mode 

_Control loop operating mode._ 
```C++
enum SYN_MotorCtrl_Mode {
    SYN_MCTRL_MODE_IDLE = 0,
    SYN_MCTRL_MODE_VELOCITY = 1,
    SYN_MCTRL_MODE_POSITION = 2,
    SYN_MCTRL_MODE_OPEN_LOOP = 3
};
```




<hr>



### typedef SYN\_MotorCtrl\_ReadPos 

_Position feedback function._ 
```C++
typedef int32_t(* SYN_MotorCtrl_ReadPos) (void *ctx);
```



Returns the current position in whatever units the sensor provides (encoder counts, ADC ticks, millivolts — the PID doesn't care). The same units must be used for all targets and deadbands.




**Parameters:**


* `ctx` User context. 



**Returns:**

Current position in feedback units. 





        

<hr>



### typedef SYN\_MotorCtrl\_StallCallback 

_Callback invoked when a motor stall is detected._ 
```C++
typedef void(* SYN_MotorCtrl_StallCallback) (struct SYN_MotorCtrl *ctrl, void *ctx);
```





**Parameters:**


* `ctrl` Controller that detected the stall. 
* `ctx` User context. 




        

<hr>



### enum SYN\_MotorCtrl\_State 

_Motor controller runtime state._ 
```C++
enum SYN_MotorCtrl_State {
    SYN_MCTRL_STOPPED = 0,
    SYN_MCTRL_RUNNING = 1,
    SYN_MCTRL_ON_TARGET = 2,
    SYN_MCTRL_STALLED = 3,
    SYN_MCTRL_LIMIT = 4
};
```




<hr>



### typedef SYN\_MotorCtrl\_TargetCallback 

_Callback invoked when position reaches the target deadband._ 
```C++
typedef void(* SYN_MotorCtrl_TargetCallback) (struct SYN_MotorCtrl *ctrl, void *ctx);
```





**Parameters:**


* `ctrl` Controller that reached target. 
* `ctx` User context. 




        

<hr>
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




### function syn\_motor\_ctrl\_error 

_Get last position error._ 
```C++
static inline int32_t syn_motor_ctrl_error (
    const SYN_MotorCtrl * ctrl
) 
```





**Parameters:**


* `ctrl` Controller. 



**Returns:**

Position error in feedback units. 





        

<hr>



### function syn\_motor\_ctrl\_ff\_output 

_Get last feedforward output component._ 
```C++
static inline int32_t syn_motor_ctrl_ff_output (
    const SYN_MotorCtrl * ctrl
) 
```





**Parameters:**


* `ctrl` Controller. 



**Returns:**

Feedforward output. 





        

<hr>



### function syn\_motor\_ctrl\_get\_metrics 

_Get accumulated move metrics._ 
```C++
static inline const SYN_MotorCtrl_Metrics * syn_motor_ctrl_get_metrics (
    const SYN_MotorCtrl * ctrl
) 
```



Read this after a move completes to evaluate performance. Contains max error, RMS error data, overshoot, peak output, and timing — everything needed for tuning, in a single read.




**Parameters:**


* `ctrl` Controller. 



**Returns:**

Pointer to metrics struct. 





        

<hr>



### function syn\_motor\_ctrl\_mode 

_Get the current operating mode._ 
```C++
static inline SYN_MotorCtrl_Mode syn_motor_ctrl_mode (
    const SYN_MotorCtrl * ctrl
) 
```





**Parameters:**


* `ctrl` Controller. 



**Returns:**

Operating mode. 





        

<hr>



### function syn\_motor\_ctrl\_move\_duration 

_Get move duration in milliseconds._ 
```C++
static inline uint32_t syn_motor_ctrl_move_duration (
    const SYN_MotorCtrl * ctrl
) 
```





**Parameters:**


* `ctrl` Controller. 



**Returns:**

Duration in ms, or 0 if no move started. 





        

<hr>



### function syn\_motor\_ctrl\_output 

_Get total output (PID + feedforward)._ 
```C++
static inline int32_t syn_motor_ctrl_output (
    const SYN_MotorCtrl * ctrl
) 
```





**Parameters:**


* `ctrl` Controller. 



**Returns:**

Combined output. 





        

<hr>



### function syn\_motor\_ctrl\_pid\_output 

_Get last PID output component._ 
```C++
static inline int32_t syn_motor_ctrl_pid_output (
    const SYN_MotorCtrl * ctrl
) 
```





**Parameters:**


* `ctrl` Controller. 



**Returns:**

PID output. 





        

<hr>



### function syn\_motor\_ctrl\_position 

_Get measured position._ 
```C++
static inline int32_t syn_motor_ctrl_position (
    const SYN_MotorCtrl * ctrl
) 
```





**Parameters:**


* `ctrl` Controller. 



**Returns:**

Position in feedback units. 





        

<hr>



### function syn\_motor\_ctrl\_settle\_time 

_Get settling time in milliseconds (0 if never settled)._ 
```C++
static inline uint32_t syn_motor_ctrl_settle_time (
    const SYN_MotorCtrl * ctrl
) 
```





**Parameters:**


* `ctrl` Controller. 



**Returns:**

Settle time in ms. 





        

<hr>



### function syn\_motor\_ctrl\_state 

_Get the current controller state._ 
```C++
static inline SYN_MotorCtrl_State syn_motor_ctrl_state (
    const SYN_MotorCtrl * ctrl
) 
```





**Parameters:**


* `ctrl` Controller. 



**Returns:**

Runtime state. 





        

<hr>



### function syn\_motor\_ctrl\_velocity 

_Get measured velocity._ 
```C++
static inline int32_t syn_motor_ctrl_velocity (
    const SYN_MotorCtrl * ctrl
) 
```





**Parameters:**


* `ctrl` Controller. 



**Returns:**

Velocity in feedback units/second. 





        

<hr>
## Macro Definition Documentation





### define SYN\_MCTRL\_DATALOG\_ID 

```C++
#define SYN_MCTRL_DATALOG_ID `0x4D43  /* 'MC' */`
```



Datalog stream ID for motor controller telemetry. 


        

<hr>



### define SYN\_MOTOR\_CTRL\_DEFAULTS 

_Convenience macro for a motor controller config with sane defaults._ 
```C++
#define SYN_MOTOR_CTRL_DEFAULTS (
    motor_out,
    read_fn,
    read_ctx,
    hz,
    out_max
) `/* multi line expression */`
```



Provides a conservative P-only configuration that is stable on most systems. Override individual fields after initialization to tune.




**Parameters:**


* `motor_out` [**SYN\_MotorOutput**](structSYN__MotorOutput.md) (e.g., syn\_dc\_motor\_output(&motor)). 
* `read_fn` Position read function. 
* `read_ctx` Context for read\_fn (NULL if unused). 
* `hz` Control loop frequency (e.g., 1000). 
* `out_max` Maximum output magnitude (symmetric ±out\_max).



**
**


```C++
SYN_MotorCtrl_Config cfg = SYN_MOTOR_CTRL_DEFAULTS(
    syn_dc_motor_output(&motor), encoder_read, NULL, 1000, 1000
);
cfg.pid_ki = 500;  // add integral if needed
syn_motor_ctrl_init(&ctrl, &cfg);
```
 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_motor_ctrl.h`

