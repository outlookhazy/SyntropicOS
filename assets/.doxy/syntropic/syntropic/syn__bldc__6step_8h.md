

# File syn\_bldc\_6step.h



[**FileList**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_bldc\_6step.h**](syn__bldc__6step_8h.md)

[Go to the source code of this file](syn__bldc__6step_8h_source.md)

_Zero-Heap 6-Step (Trapezoidal) BLDC Motor Commutation Driver._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/control/syn_pid.h"`
* `#include "syntropic/util/syn_qmath.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_BLDC\_6Step**](structSYN__BLDC__6Step.md) <br> |
| struct | [**SYN\_BLDC\_Config**](structSYN__BLDC__Config.md) <br> |
| struct | [**SYN\_BLDC\_PhaseOutputs**](structSYN__BLDC__PhaseOutputs.md) <br> |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_BLDC\_Direction**](#enum-syn_bldc_direction)  <br> |
| enum  | [**SYN\_BLDC\_GateState**](#enum-syn_bldc_gatestate)  <br> |
| enum  | [**SYN\_BLDC\_State**](#enum-syn_bldc_state)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_bldc\_6step\_get\_phase\_outputs**](#function-syn_bldc_6step_get_phase_outputs) (const [**SYN\_BLDC\_6Step**](structSYN__BLDC__6Step.md) \* bldc, [**SYN\_BLDC\_PhaseOutputs**](structSYN__BLDC__PhaseOutputs.md) \* out) <br>_Retrieve current 3-phase gate output states._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_bldc\_6step\_init**](#function-syn_bldc_6step_init) ([**SYN\_BLDC\_6Step**](structSYN__BLDC__6Step.md) \* bldc, const [**SYN\_BLDC\_Config**](structSYN__BLDC__Config.md) \* cfg) <br>_Initialize 6-Step BLDC driver instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_bldc\_6step\_set\_direction**](#function-syn_bldc_6step_set_direction) ([**SYN\_BLDC\_6Step**](structSYN__BLDC__6Step.md) \* bldc, [**SYN\_BLDC\_Direction**](syn__bldc__6step_8h.md#enum-syn_bldc_direction) dir) <br>_Set motor rotation direction._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_bldc\_6step\_set\_duty**](#function-syn_bldc_6step_set_duty) ([**SYN\_BLDC\_6Step**](structSYN__BLDC__6Step.md) \* bldc, uint16\_t duty\_0\_to\_1000) <br>_Set PWM duty cycle for motor speed control._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_bldc\_6step\_set\_hall**](#function-syn_bldc_6step_set_hall) ([**SYN\_BLDC\_6Step**](structSYN__BLDC__6Step.md) \* bldc, uint8\_t hall\_state, [**SYN\_BLDC\_PhaseOutputs**](structSYN__BLDC__PhaseOutputs.md) \* out) <br>_Process 3-bit Hall sensor reading and update 6-step phase commutation gates._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_bldc\_6step\_start**](#function-syn_bldc_6step_start) ([**SYN\_BLDC\_6Step**](structSYN__BLDC__6Step.md) \* bldc) <br>_Start motor driver (enable gate commutation)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_bldc\_6step\_stop**](#function-syn_bldc_6step_stop) ([**SYN\_BLDC\_6Step**](structSYN__BLDC__6Step.md) \* bldc, [**SYN\_BLDC\_PhaseOutputs**](structSYN__BLDC__PhaseOutputs.md) \* out) <br>_Stop motor driver (turn OFF all 6 gates)._  |
|  uint32\_t | [**syn\_bldc\_6step\_update\_speed**](#function-syn_bldc_6step_update_speed) ([**SYN\_BLDC\_6Step**](structSYN__BLDC__6Step.md) \* bldc, uint32\_t now\_ms, uint32\_t target\_rpm) <br>_Update Hall-based speed calculation (RPM) and speed PID loop._  |




























## Detailed Description


Provides 6-step trapezoidal block commutation for 3-phase Brushless DC (BLDC) motors:
* 3-channel Hall sensor state decoding (Hall states 1..6)
* 6-step phase commutation lookup table for 3-phase half-bridge outputs (U, V, W)
* Clockwise (CW) & Counter-Clockwise (CCW) rotation support
* Fault detection for invalid Hall states (0b000 & 0b111)
* PWM duty cycle scaling (0..1000)
* Hall transition edge timestamping for RPM speed calculation 




    
## Public Types Documentation




### enum SYN\_BLDC\_Direction 

```C++
enum SYN_BLDC_Direction {
    SYN_BLDC_DIR_CW = 1,
    SYN_BLDC_DIR_CCW = 2
};
```



Motor Rotation Direction. 


        

<hr>



### enum SYN\_BLDC\_GateState 

```C++
enum SYN_BLDC_GateState {
    SYN_BLDC_GATE_OFF = 0,
    SYN_BLDC_GATE_PWM = 1,
    SYN_BLDC_GATE_LOW = 2
};
```



Phase Output State (High-Side / Low-Side Gate Switches). 


        

<hr>



### enum SYN\_BLDC\_State 

```C++
enum SYN_BLDC_State {
    SYN_BLDC_STATE_STOPPED = 0,
    SYN_BLDC_STATE_RUNNING = 1,
    SYN_BLDC_STATE_FAULT = 2
};
```



Driver State. 


        

<hr>
## Public Functions Documentation




### function syn\_bldc\_6step\_get\_phase\_outputs 

_Retrieve current 3-phase gate output states._ 
```C++
SYN_Status syn_bldc_6step_get_phase_outputs (
    const SYN_BLDC_6Step * bldc,
    SYN_BLDC_PhaseOutputs * out
) 
```





**Parameters:**


* `bldc` Pointer to driver struct. 
* `out` Pointer to destination output struct. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_bldc\_6step\_init 

_Initialize 6-Step BLDC driver instance._ 
```C++
SYN_Status syn_bldc_6step_init (
    SYN_BLDC_6Step * bldc,
    const SYN_BLDC_Config * cfg
) 
```





**Parameters:**


* `bldc` Pointer to driver struct. 
* `cfg` Configuration parameters (or NULL for defaults: 4 pole pairs). 



**Returns:**

SYN\_OK on success, SYN\_ERR\_INVALID\_PARAM if bldc is NULL. 





        

<hr>



### function syn\_bldc\_6step\_set\_direction 

_Set motor rotation direction._ 
```C++
SYN_Status syn_bldc_6step_set_direction (
    SYN_BLDC_6Step * bldc,
    SYN_BLDC_Direction dir
) 
```





**Parameters:**


* `bldc` Pointer to driver struct. 
* `dir` SYN\_BLDC\_DIR\_CW or SYN\_BLDC\_DIR\_CCW. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_bldc\_6step\_set\_duty 

_Set PWM duty cycle for motor speed control._ 
```C++
SYN_Status syn_bldc_6step_set_duty (
    SYN_BLDC_6Step * bldc,
    uint16_t duty_0_to_1000
) 
```





**Parameters:**


* `bldc` Pointer to driver struct. 
* `duty_0_to_1000` PWM duty cycle clamped to range 0..1000. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_bldc\_6step\_set\_hall 

_Process 3-bit Hall sensor reading and update 6-step phase commutation gates._ 
```C++
SYN_Status syn_bldc_6step_set_hall (
    SYN_BLDC_6Step * bldc,
    uint8_t hall_state,
    SYN_BLDC_PhaseOutputs * out
) 
```



Call this from Hall sensor EXTI GPIO interrupts or input sampling loops.




**Parameters:**


* `bldc` Pointer to driver struct. 
* `hall_state` 3-bit Hall sensor state (bits [2:0] = H3, H2, H1). Valid: 1..6. 
* `out` Pointer to output struct to receive new 3-phase gate states. 



**Returns:**

SYN\_OK on valid commutation step, SYN\_ERR\_FAULT if hall\_state is 0b000 or 0b111. 





        

<hr>



### function syn\_bldc\_6step\_start 

_Start motor driver (enable gate commutation)._ 
```C++
SYN_Status syn_bldc_6step_start (
    SYN_BLDC_6Step * bldc
) 
```





**Parameters:**


* `bldc` Pointer to driver struct. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_bldc\_6step\_stop 

_Stop motor driver (turn OFF all 6 gates)._ 
```C++
SYN_Status syn_bldc_6step_stop (
    SYN_BLDC_6Step * bldc,
    SYN_BLDC_PhaseOutputs * out
) 
```





**Parameters:**


* `bldc` Pointer to driver struct. 
* `out` Optional pointer to receive updated OFF gate states. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_bldc\_6step\_update\_speed 

_Update Hall-based speed calculation (RPM) and speed PID loop._ 
```C++
uint32_t syn_bldc_6step_update_speed (
    SYN_BLDC_6Step * bldc,
    uint32_t now_ms,
    uint32_t target_rpm
) 
```



Call periodically (e.g. every 10ms or 100ms).




**Parameters:**


* `bldc` Pointer to driver struct. 
* `now_ms` Current system tick in milliseconds. 
* `target_rpm` Target motor speed in RPM (used if speed PID is active). 



**Returns:**

Calculated current RPM. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_bldc_6step.h`

