

# File syn\_autotune.c



[**FileList**](files.md) **>** [**control**](dir_d63400c13859c7c99cca12fba8ce6e16.md) **>** [**syn\_autotune.c**](syn__autotune_8c.md)

[Go to the source code of this file](syn__autotune_8c_source.md)



* `#include "syn_autotune.h"`
* `#include "../util/syn_assert.h"`
* `#include "../port/syn_port_system.h"`
* `#include <string.h>`
* `#include <stdio.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_autotune\_abort**](#function-syn_autotune_abort) ([**SYN\_AutoTune**](structSYN__AutoTune.md) \* at) <br>_Abort the auto-tune and stop the motor immediately._  |
|  void | [**syn\_autotune\_apply**](#function-syn_autotune_apply) ([**SYN\_AutoTune**](structSYN__AutoTune.md) \* at) <br>_Apply computed gains to the motor controller._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_autotune\_init**](#function-syn_autotune_init) ([**SYN\_AutoTune**](structSYN__AutoTune.md) \* at, [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl, const [**SYN\_AutoTune\_Config**](structSYN__AutoTune__Config.md) \* cfg) <br>_Initialize the auto-tuner._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_autotune\_start**](#function-syn_autotune_start) ([**SYN\_AutoTune**](structSYN__AutoTune.md) \* at, [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* ctrl, const [**SYN\_AutoTune\_Limits**](structSYN__AutoTune__Limits.md) \* limits, [**SYN\_AutoTune\_Flags**](syn__autotune_8h.md#enum-syn_autotune_flags) flags, uint16\_t gain\_multiplier) <br>_Start a fully automatic tune sequence._  |
|  [**SYN\_AutoTune\_State**](syn__autotune_8h.md#enum-syn_autotune_state) | [**syn\_autotune\_update**](#function-syn_autotune_update) ([**SYN\_AutoTune**](structSYN__AutoTune.md) \* at) <br>_Update the auto-tuner — call from main loop._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**apply\_raw\_output**](#function-apply_raw_output) ([**SYN\_AutoTune**](structSYN__AutoTune.md) \* at, int32\_t output) <br> |
|  void | [**emergency\_stop**](#function-emergency_stop) ([**SYN\_AutoTune**](structSYN__AutoTune.md) \* at, [**SYN\_AutoTune\_AbortReason**](syn__autotune_8h.md#enum-syn_autotune_abortreason) reason) <br> |
|  int32\_t | [**ramp\_output**](#function-ramp_output) ([**SYN\_AutoTune**](structSYN__AutoTune.md) \* at, int32\_t target, uint32\_t elapsed) <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**safety\_ok**](#function-safety_ok) ([**SYN\_AutoTune**](structSYN__AutoTune.md) \* at, int32\_t pos, int32\_t velocity) <br> |
|  void | [**write\_log**](#function-write_log) ([**SYN\_AutoTune**](structSYN__AutoTune.md) \* at) <br> |


























## Public Functions Documentation




### function syn\_autotune\_abort 

_Abort the auto-tune and stop the motor immediately._ 
```C++
void syn_autotune_abort (
    SYN_AutoTune * at
) 
```





**Parameters:**


* `at` Auto-tuner instance. 




        

<hr>



### function syn\_autotune\_apply 

_Apply computed gains to the motor controller._ 
```C++
void syn_autotune_apply (
    SYN_AutoTune * at
) 
```



Copies ff\_kv, ff\_scale, and PID gains into the motor controller. Only valid when state == DONE.




**Parameters:**


* `at` Auto-tuner instance. 




        

<hr>



### function syn\_autotune\_init 

_Initialize the auto-tuner._ 
```C++
SYN_Status syn_autotune_init (
    SYN_AutoTune * at,
    SYN_MotorCtrl * ctrl,
    const SYN_AutoTune_Config * cfg
) 
```





**Parameters:**


* `at` Auto-tuner instance. 
* `ctrl` Motor controller to tune (tuner takes control while active). 
* `cfg` Configuration. position\_limit MUST be set. 



**Returns:**

SYN\_OK, or SYN\_ERROR if position\_limit is 0. 





        

<hr>



### function syn\_autotune\_start 

_Start a fully automatic tune sequence._ 
```C++
SYN_Status syn_autotune_start (
    SYN_AutoTune * at,
    SYN_MotorCtrl * ctrl,
    const SYN_AutoTune_Limits * limits,
    SYN_AutoTune_Flags flags,
    uint16_t gain_multiplier
) 
```



Runs probe → FF identification → braking → relay PID tune → braking as a single self-sequencing state machine. The user only provides physical constraints.




**Parameters:**


* `limits` Physical constraints (track limits, max velocity). 
* `flags` Feature flags (e.g., SYN\_ATUNE\_FLAG\_ALL). 
* `gain_multiplier` Safety margin for PID gains (percentage, e.g., 80). 



**Returns:**

SYN\_OK on success, or error code. 





        

<hr>



### function syn\_autotune\_update 

_Update the auto-tuner — call from main loop._ 
```C++
SYN_AutoTune_State syn_autotune_update (
    SYN_AutoTune * at
) 
```





**Parameters:**


* `at` Auto-tuner instance. 



**Returns:**

Current state. When DONE, results are in [**syn\_autotune\_result()**](syn__autotune_8h.md#function-syn_autotune_result). 





        

<hr>
## Public Static Functions Documentation




### function apply\_raw\_output 

```C++
static void apply_raw_output (
    SYN_AutoTune * at,
    int32_t output
) 
```




<hr>



### function emergency\_stop 

```C++
static void emergency_stop (
    SYN_AutoTune * at,
    SYN_AutoTune_AbortReason reason
) 
```




<hr>



### function ramp\_output 

```C++
static int32_t ramp_output (
    SYN_AutoTune * at,
    int32_t target,
    uint32_t elapsed
) 
```



Ramp output from 0 toward target over ramp\_ms. Returns current output. 


        

<hr>



### function safety\_ok 

```C++
static bool safety_ok (
    SYN_AutoTune * at,
    int32_t pos,
    int32_t velocity
) 
```



Check all safety conditions. Returns true if safe. 


        

<hr>



### function write\_log 

```C++
static void write_log (
    SYN_AutoTune * at
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_autotune.c`

