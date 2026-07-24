

# File syn\_pid.h



[**FileList**](files.md) **>** [**control**](dir_d63400c13859c7c99cca12fba8ce6e16.md) **>** [**syn\_pid.h**](syn__pid_8h.md)

[Go to the source code of this file](syn__pid_8h_source.md)

_General-purpose PID controller — integer arithmetic._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_PID**](structSYN__PID.md) <br>_PID controller instance — config + accumulated state._  |
| struct | [**SYN\_PID\_Config**](structSYN__PID__Config.md) <br>_PID controller configuration._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_pid\_init**](#function-syn_pid_init) ([**SYN\_PID**](structSYN__PID.md) \* pid, const [**SYN\_PID\_Config**](structSYN__PID__Config.md) \* cfg) <br>_Initialize PID controller._  |
|  void | [**syn\_pid\_reset**](#function-syn_pid_reset) ([**SYN\_PID**](structSYN__PID.md) \* pid) <br>_Reset integral and derivative state._  |
|  void | [**syn\_pid\_set\_gains**](#function-syn_pid_set_gains) ([**SYN\_PID**](structSYN__PID.md) \* pid, int32\_t kp, int32\_t ki, int32\_t kd) <br>_Update gains at runtime._  |
|  void | [**syn\_pid\_set\_limits**](#function-syn_pid_set_limits) ([**SYN\_PID**](structSYN__PID.md) \* pid, int32\_t out\_min, int32\_t out\_max) <br>_Set output clamping limits._  |
|  int32\_t | [**syn\_pid\_update**](#function-syn_pid_update) ([**SYN\_PID**](structSYN__PID.md) \* pid, int32\_t setpoint, int32\_t measured, uint32\_t dt\_ms) <br>_Compute one PID update step._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  int32\_t | [**syn\_pid\_output**](#function-syn_pid_output) (const [**SYN\_PID**](structSYN__PID.md) \* pid) <br>_Get the last computed output._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_PID\_GAINS**](syn__pid_8h.md#define-syn_pid_gains) (kp\_f, ki\_f, kd\_f, scale\_val, omin, omax) `/* multi line expression */`<br>_Convenience macro to create a PID config from human-readable gains._  |

## Detailed Description


A discrete-time PID with anti-windup, output clamping, derivative filtering, and configurable gain scaling. Works for motor speed, temperature, position, or any closed-loop control.




**
**

All gains are integers divided by `scale`. This allows fractional control without floating-point:



|Parameter   |Effective value   |Notes    |
|-----|-----|-----|
|kp   |kp / scale   |Proportional    |
|ki   |ki / (scale × 1000)   |**Note the ×1000 for time normalization**    |
|kd   |kd / scale   |Derivative (with dt normalization)   |






The I-term accumulates error×dt\_ms, so the division by 1000 converts millisecond-accumulation into seconds. This means `ki` must be ~1000× larger than you'd expect compared to kp/kd for equivalent effect.




**
**

Use [**SYN\_PID\_GAINS()**](syn__pid_8h.md#define-syn_pid_gains) to set gains from intuitive values: 
```C++
// Instead of manually computing scaled values:
SYN_PID_Config cfg = SYN_PID_GAINS(
    1.5,    // kp (proportional)
    0.5,    // ki (integral, per-second — NOT per-millisecond)
    0.1,    // kd (derivative)
    256,    // scale
    -1000, 1000  // output range
);
// Produces: kp=384, ki=128000, kd=25, scale=256
```





**
**


```C++
SYN_PID pid;
SYN_PID_Config cfg = {
    .kp = 100, .ki = 10000, .kd = 50,
    .scale = 100,        // gains are /100
    .out_min = -1000, .out_max = 1000,
    .d_filter_alpha = 128,
};
syn_pid_init(&pid, &cfg);

// In control loop (e.g., every 10ms):
int32_t output = syn_pid_update(&pid, setpoint, measured, dt_ms);
```
 





    
## Public Functions Documentation




### function syn\_pid\_init 

_Initialize PID controller._ 
```C++
void syn_pid_init (
    SYN_PID * pid,
    const SYN_PID_Config * cfg
) 
```



If integral\_max is 0, it is auto-computed from ki, scale, and out\_max to allow full integral authority without windup.




**Parameters:**


* `pid` PID instance. 
* `cfg` Configuration (copied internally). 




        

<hr>



### function syn\_pid\_reset 

_Reset integral and derivative state._ 
```C++
void syn_pid_reset (
    SYN_PID * pid
) 
```





**Parameters:**


* `pid` PID instance. 




        

<hr>



### function syn\_pid\_set\_gains 

_Update gains at runtime._ 
```C++
void syn_pid_set_gains (
    SYN_PID * pid,
    int32_t kp,
    int32_t ki,
    int32_t kd
) 
```





**Parameters:**


* `pid` PID instance. 
* `kp` Proportional gain. 
* `ki` Integral gain. 
* `kd` Derivative gain. 




        

<hr>



### function syn\_pid\_set\_limits 

_Set output clamping limits._ 
```C++
void syn_pid_set_limits (
    SYN_PID * pid,
    int32_t out_min,
    int32_t out_max
) 
```





**Parameters:**


* `pid` PID instance. 
* `out_min` Minimum output. 
* `out_max` Maximum output. 




        

<hr>



### function syn\_pid\_update 

_Compute one PID update step._ 
```C++
int32_t syn_pid_update (
    SYN_PID * pid,
    int32_t setpoint,
    int32_t measured,
    uint32_t dt_ms
) 
```





**Parameters:**


* `pid` PID instance. 
* `setpoint` Desired value. 
* `measured` Actual measured value. 
* `dt_ms` Time since last update in milliseconds. (0 is treated as 1 to avoid division by zero.) 



**Returns:**

Clamped output value. 





        

<hr>
## Public Static Functions Documentation




### function syn\_pid\_output 

_Get the last computed output._ 
```C++
static inline int32_t syn_pid_output (
    const SYN_PID * pid
) 
```





**Parameters:**


* `pid` PID instance. 



**Returns:**

Last output value. 





        

<hr>
## Macro Definition Documentation





### define SYN\_PID\_GAINS 

_Convenience macro to create a PID config from human-readable gains._ 
```C++
#define SYN_PID_GAINS (
    kp_f,
    ki_f,
    kd_f,
    scale_val,
    omin,
    omax
) `/* multi line expression */`
```



Accepts floating-point gain values and a scale factor. Computes the integer ki with the ×1000 factor built in, so you specify ki as a normal per-second integral gain.




**Parameters:**


* `kp_f` Proportional gain (float). 
* `ki_f` Integral gain per second (float). 
* `kd_f` Derivative gain (float). 
* `scale_val` Integer scale divisor (e.g., 256). 
* `omin` Minimum output. 
* `omax` Maximum output. 



**Returns:**

[**SYN\_PID\_Config**](structSYN__PID__Config.md) struct initializer.




**
**


```C++
SYN_PID_Config cfg = SYN_PID_GAINS(1.5, 0.5, 0.1, 256, -1000, 1000);
// kp = (int)(1.5 * 256)       = 384
// ki = (int)(0.5 * 256 * 1000) = 128000
// kd = (int)(0.1 * 256)       = 25
```
 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_pid.h`

