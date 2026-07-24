

# File syn\_autotune.h



[**FileList**](files.md) **>** [**control**](dir_d63400c13859c7c99cca12fba8ce6e16.md) **>** [**syn\_autotune.h**](syn__autotune_8h.md)

[Go to the source code of this file](syn__autotune_8h_source.md)

_Motor controller auto-tuner — feedforward ID + relay PID tuning._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../motor/syn_motor_ctrl.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_AutoTune**](structSYN__AutoTune.md) <br>_Auto-tuner instance._  |
| struct | [**SYN\_AutoTune\_Config**](structSYN__AutoTune__Config.md) <br>_Auto-tuner configuration._  |
| struct | [**SYN\_AutoTune\_Limits**](structSYN__AutoTune__Limits.md) <br>_Physical constraints for the one-call auto-tune._  |
| struct | [**SYN\_AutoTune\_LogFrame**](structSYN__AutoTune__LogFrame.md) <br>_Telemetry frame for Auto-Tune capture._  |
| struct | [**SYN\_AutoTune\_Result**](structSYN__AutoTune__Result.md) <br>_Auto-tune results (valid when state == DONE)._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_AutoTune\_AbortReason**](#enum-syn_autotune_abortreason)  <br>_Reason for auto-tune abort._  |
| enum  | [**SYN\_AutoTune\_Flags**](#enum-syn_autotune_flags)  <br>_Auto-tuner feature flags._  |
| enum  | [**SYN\_AutoTune\_Method**](#enum-syn_autotune_method)  <br>_PID gain formula selection for relay auto-tune._  |
| enum  | [**SYN\_AutoTune\_Mode**](#enum-syn_autotune_mode)  <br>_Auto-tune operating mode._  |
| enum  | [**SYN\_AutoTune\_State**](#enum-syn_autotune_state)  <br>_Auto-tuner state machine states._  |




















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
|  [**SYN\_AutoTune\_AbortReason**](syn__autotune_8h.md#enum-syn_autotune_abortreason) | [**syn\_autotune\_abort\_reason**](#function-syn_autotune_abort_reason) (const [**SYN\_AutoTune**](structSYN__AutoTune.md) \* at) <br>_Get the reason for abort (valid when state == ABORTED)._  |
|  const [**SYN\_AutoTune\_Result**](structSYN__AutoTune__Result.md) \* | [**syn\_autotune\_result**](#function-syn_autotune_result) (const [**SYN\_AutoTune**](structSYN__AutoTune.md) \* at) <br>_Get the tuning results (valid when state == DONE)._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_ATUNE\_LOG\_ID**](syn__autotune_8h.md#define-syn_atune_log_id)  `0x4154  /\* 'AT' \*/`<br>_Telemetry frame ID for Auto-Tune data._  |

## Detailed Description


Safety-first design for heavy machinery:
* Hard position limit — abort if displacement exceeds configured bound
* Velocity limit — cut power if speed exceeds safe threshold
* Motor controller soft limits are respected (checked every update)
* Watchdog timeout — auto-abort if update() stops being called
* Gradual ramp-up (not a step) to avoid jerking heavy loads




Two layers of API:



* **[**syn\_autotune\_start()**](syn__autotune_8h.md#function-syn_autotune_start)** (recommended) — one-call auto-tune. Runs PROBE → FF identification → braking → relay PID tuning → braking automatically. The user only provides track limits.
* **[**syn\_autotune\_init()**](syn__autotune_8h.md#function-syn_autotune_init)** (advanced) — manual mode selection. The caller configures and runs each phase (FF ident, relay) separately.




Both are non-blocking — call [**syn\_autotune\_update()**](syn__autotune_8h.md#function-syn_autotune_update) from your main loop.




**
**




**Step 1: Home the encoder.** Zero the encoder at one end of the track. The auto-tuner does NOT require zeroing in the middle — it works with any zero convention.


**Step 2: Configure the motor controller.** Set position\_min/max to your safe operating range (in encoder counts). These are the soft limits the auto-tuner will respect. 
```C++
SYN_MotorCtrl_Config cfg = SYN_MOTOR_CTRL_DEFAULTS(
    syn_dc_motor_output(&motor), encoder_read, NULL, 1000, 1000
);
cfg.position_min  = 5000;     // 0.5m from home end (10000 cts/m)
cfg.position_max  = 495000;   // 0.5m from far end
syn_motor_ctrl_init(&ctrl, &cfg);
```



**Step 3: Start auto-tune.** 
```C++
SYN_AutoTune at;
SYN_AutoTune_Limits limits = {
    .position_min = 5000,
    .position_max = 495000,
    .watchdog_ms  = 1000,    // abort if update() stops for 1s
};
syn_autotune_start(&at, &ctrl, &limits, SYN_ATUNE_FLAG_ALL);
```



**Step 4: Poll from your main loop.** 
```C++
while (true) {
    SYN_AutoTune_State st = syn_autotune_update(&at);
    if (st == SYN_ATUNE_DONE) {
        syn_autotune_apply(&at); // Copy gains to controller
        break;
    }
    if (st == SYN_ATUNE_ABORTED) {
        // Error handling...
        break;
    }
    syn_port_sleep_ms(1); // or run from 1kHz interrupt
}
```
 


    
## Public Types Documentation




### enum SYN\_AutoTune\_AbortReason 

_Reason for auto-tune abort._ 
```C++
enum SYN_AutoTune_AbortReason {
    SYN_ATUNE_OK = 0,
    SYN_ATUNE_ABORT_POSITION = 1,
    SYN_ATUNE_ABORT_VELOCITY = 2,
    SYN_ATUNE_ABORT_SOFT_LIMIT = 3,
    SYN_ATUNE_ABORT_WATCHDOG = 4,
    SYN_ATUNE_ABORT_USER = 5,
    SYN_ATUNE_ABORT_STALL = 6,
    SYN_ATUNE_ABORT_NO_MOTION = 7
};
```




<hr>



### enum SYN\_AutoTune\_Flags 

_Auto-tuner feature flags._ 
```C++
enum SYN_AutoTune_Flags {
    SYN_ATUNE_FLAG_NONE = 0,
    SYN_ATUNE_FLAG_IDENT_KV = (1 << 0),
    SYN_ATUNE_FLAG_IDENT_KA = (1 << 1),
    SYN_ATUNE_FLAG_TUNE_PID = (1 << 2),
    SYN_ATUNE_FLAG_ALL = (SYN_ATUNE_FLAG_IDENT_KV | 
                          SYN_ATUNE_FLAG_IDENT_KA | 
                          SYN_ATUNE_FLAG_TUNE_PID)
};
```




<hr>



### enum SYN\_AutoTune\_Method 

_PID gain formula selection for relay auto-tune._ 
```C++
enum SYN_AutoTune_Method {
    SYN_ATUNE_ZN_CLASSIC = 0,
    SYN_ATUNE_ZN_NO_OVERSHOOT = 1,
    SYN_ATUNE_TYREUS_LUYBEN = 2
};
```




<hr>



### enum SYN\_AutoTune\_Mode 

_Auto-tune operating mode._ 
```C++
enum SYN_AutoTune_Mode {
    SYN_ATUNE_MODE_FF_IDENT = 0,
    SYN_ATUNE_MODE_RELAY = 1,
    SYN_ATUNE_MODE_AUTO = 2
};
```




<hr>



### enum SYN\_AutoTune\_State 

_Auto-tuner state machine states._ 
```C++
enum SYN_AutoTune_State {
    SYN_ATUNE_IDLE,
    SYN_ATUNE_PROBE,
    SYN_ATUNE_RAMP_UP,
    SYN_ATUNE_SETTLING,
    SYN_ATUNE_MEASURING,
    SYN_ATUNE_SETTLING_2,
    SYN_ATUNE_MEASURING_2,
    SYN_ATUNE_RELAY,
    SYN_ATUNE_BRAKING,
    SYN_ATUNE_RAMP_DOWN,
    SYN_ATUNE_DONE,
    SYN_ATUNE_ABORTED
};
```




<hr>
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


* `at` Auto-tuner instance. 
* `ctrl` Motor controller to tune. 
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




### function syn\_autotune\_abort\_reason 

_Get the reason for abort (valid when state == ABORTED)._ 
```C++
static inline SYN_AutoTune_AbortReason syn_autotune_abort_reason (
    const SYN_AutoTune * at
) 
```





**Parameters:**


* `at` Autotuner instance. 



**Returns:**

Abort reason code. 





        

<hr>



### function syn\_autotune\_result 

_Get the tuning results (valid when state == DONE)._ 
```C++
static inline const SYN_AutoTune_Result * syn_autotune_result (
    const SYN_AutoTune * at
) 
```





**Parameters:**


* `at` Autotuner instance. 



**Returns:**

Pointer to results struct. 





        

<hr>
## Macro Definition Documentation





### define SYN\_ATUNE\_LOG\_ID 

_Telemetry frame ID for Auto-Tune data._ 
```C++
#define SYN_ATUNE_LOG_ID `0x4154  /* 'AT' */`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_autotune.h`

