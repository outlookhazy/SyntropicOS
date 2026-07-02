

# File syn\_pid.c



[**FileList**](files.md) **>** [**control**](dir_d63400c13859c7c99cca12fba8ce6e16.md) **>** [**syn\_pid.c**](syn__pid_8c.md)

[Go to the source code of this file](syn__pid_8c_source.md)

_PID controller implementation._ 

* `#include "syn_pid.h"`
* `#include "../common/syn_defs.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_pid\_init**](#function-syn_pid_init) ([**SYN\_PID**](structSYN__PID.md) \* pid, const [**SYN\_PID\_Config**](structSYN__PID__Config.md) \* cfg) <br>_Initialize PID controller._  |
|  void | [**syn\_pid\_reset**](#function-syn_pid_reset) ([**SYN\_PID**](structSYN__PID.md) \* pid) <br>_Reset integral and derivative state._  |
|  void | [**syn\_pid\_set\_gains**](#function-syn_pid_set_gains) ([**SYN\_PID**](structSYN__PID.md) \* pid, int32\_t kp, int32\_t ki, int32\_t kd) <br>_Update gains at runtime._  |
|  void | [**syn\_pid\_set\_limits**](#function-syn_pid_set_limits) ([**SYN\_PID**](structSYN__PID.md) \* pid, int32\_t out\_min, int32\_t out\_max) <br>_Set output clamping limits._  |
|  int32\_t | [**syn\_pid\_update**](#function-syn_pid_update) ([**SYN\_PID**](structSYN__PID.md) \* pid, int32\_t setpoint, int32\_t measured, uint32\_t dt\_ms) <br>_Compute one PID update step._  |




























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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_pid.c`

