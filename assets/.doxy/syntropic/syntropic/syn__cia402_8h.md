

# File syn\_cia402.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cia402.h**](syn__cia402_8h.md)

[Go to the source code of this file](syn__cia402_8h_source.md)

_CANopen CiA 402 Device Profile for Drives and Motion Control._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "syn_canopen.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) <br>_CiA 402 Axis Instance._  |
| struct | [**SYN\_CIA402\_Config**](structSYN__CIA402__Config.md) <br>_CiA 402 axis configuration parameters._  |
| struct | [**SYN\_CIA402\_Setpoints**](structSYN__CIA402__Setpoints.md) <br>_CiA 402 motion controller setpoint outputs for motor loop / FOC._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_CIA402\_Mode**](#enum-syn_cia402_mode)  <br>_CiA 402 operation modes._  |
| enum  | [**SYN\_CIA402\_State**](#enum-syn_cia402_state)  <br>_CiA 402 Finite State Machine state enumeration._  |
| typedef [**SYN\_CIA402\_Config**](structSYN__CIA402__Config.md) | [**SYN\_CiA402Config**](#typedef-syn_cia402config)  <br> |
| typedef [**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) | [**SYN\_CiA402Drive**](#typedef-syn_cia402drive)  <br> |
| typedef [**SYN\_CIA402\_State**](syn__cia402_8h.md#enum-syn_cia402_state) | [**SYN\_CiA402State**](#typedef-syn_cia402state)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cia402\_feed\_feedback**](#function-syn_cia402_feed_feedback) ([**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) \* axis, int32\_t pos\_actual, int32\_t vel\_actual, int16\_t torque\_actual) <br>_Feed measured actual values (encoder position, velocity, torque) to axis._  |
|  uint16\_t | [**syn\_cia402\_get\_controlword**](#function-syn_cia402_get_controlword) (const [**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) \* axis) <br>_Read current Controlword (Index 0x6040)._  |
|  int8\_t | [**syn\_cia402\_get\_mode**](#function-syn_cia402_get_mode) (const [**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) \* axis) <br>_Get requested Mode of Operation (Index 0x6060)._  |
|  int8\_t | [**syn\_cia402\_get\_mode\_display**](#function-syn_cia402_get_mode_display) (const [**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) \* axis) <br>_Get active Mode of Operation Display (Index 0x6061)._  |
|  [**SYN\_CIA402\_State**](syn__cia402_8h.md#enum-syn_cia402_state) | [**syn\_cia402\_get\_state**](#function-syn_cia402_get_state) (const [**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) \* axis) <br>_Get current FSM state._  |
|  uint16\_t | [**syn\_cia402\_get\_statusword**](#function-syn_cia402_get_statusword) (const [**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) \* axis) <br>_Read current Statusword (Index 0x6041)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cia402\_init**](#function-syn_cia402_init) ([**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) \* axis, const [**SYN\_CIA402\_Config**](structSYN__CIA402__Config.md) \* cfg) <br>_Initialize CiA 402 Axis controller._  |
|  size\_t | [**syn\_cia402\_populate\_od**](#function-syn_cia402_populate_od) ([**SYN\_CANOpenODEntry**](structSYN__CANOpenODEntry.md) \* od\_table, size\_t max\_entries, [**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) \* axis) <br>_Populate a CANopen Object Dictionary table with standard CiA 402 objects._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cia402\_report\_actuals**](#function-syn_cia402_report_actuals) ([**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) \* axis, int32\_t pos\_actual, int32\_t vel\_actual, int16\_t torque\_actual) <br>_Feed measured actual values alias for compatibility._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cia402\_set\_controlword**](#function-syn_cia402_set_controlword) ([**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) \* axis, uint16\_t cw) <br>_Set Controlword (Index 0x6040) from master / RPDO / SDO._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cia402\_set\_mode**](#function-syn_cia402_set_mode) ([**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) \* axis, int8\_t mode) <br>_Request Mode of Operation (Index 0x6060)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cia402\_set\_target\_position**](#function-syn_cia402_set_target_position) ([**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) \* axis, int32\_t pos) <br>_Set target position (Index 0x607A)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cia402\_set\_target\_torque**](#function-syn_cia402_set_target_torque) ([**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) \* axis, int16\_t torque) <br>_Set target torque (Index 0x6071)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cia402\_set\_target\_velocity**](#function-syn_cia402_set_target_velocity) ([**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) \* axis, int32\_t vel) <br>_Set target velocity (Index 0x60FF)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cia402\_step**](#function-syn_cia402_step) ([**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) \* axis, uint32\_t dt\_us, [**SYN\_CIA402\_Setpoints**](structSYN__CIA402__Setpoints.md) \* out\_setpoints) <br>_Execute periodic CiA 402 control loop step in microseconds._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cia402\_trigger\_fault**](#function-syn_cia402_trigger_fault) ([**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) \* axis, uint16\_t error\_code) <br>_Trigger a drive fault and enter FAULT\_REACTION\_ACTIVE / FAULT state._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cia402\_update**](#function-syn_cia402_update) ([**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) \* axis, uint32\_t dt\_ms) <br>_Periodic motion update in milliseconds._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_CIA402\_CW\_CHANGE\_IMMEDIATELY**](syn__cia402_8h.md#define-syn_cia402_cw_change_immediately)  `(1U &lt;&lt; 5)`<br> |
| define  | [**SYN\_CIA402\_CW\_CHANGE\_ON\_SETPOINT**](syn__cia402_8h.md#define-syn_cia402_cw_change_on_setpoint)  `(1U &lt;&lt; 9)`<br> |
| define  | [**SYN\_CIA402\_CW\_ENABLE\_OPERATION**](syn__cia402_8h.md#define-syn_cia402_cw_enable_operation)  `(1U &lt;&lt; 3)`<br> |
| define  | [**SYN\_CIA402\_CW\_ENABLE\_VOLTAGE**](syn__cia402_8h.md#define-syn_cia402_cw_enable_voltage)  `(1U &lt;&lt; 1)`<br> |
| define  | [**SYN\_CIA402\_CW\_FAULT\_RESET**](syn__cia402_8h.md#define-syn_cia402_cw_fault_reset)  `(1U &lt;&lt; 7)`<br> |
| define  | [**SYN\_CIA402\_CW\_HALT**](syn__cia402_8h.md#define-syn_cia402_cw_halt)  `(1U &lt;&lt; 8)`<br> |
| define  | [**SYN\_CIA402\_CW\_NEW\_SETPOINT**](syn__cia402_8h.md#define-syn_cia402_cw_new_setpoint)  `(1U &lt;&lt; 4)`<br> |
| define  | [**SYN\_CIA402\_CW\_QUICK\_STOP**](syn__cia402_8h.md#define-syn_cia402_cw_quick_stop)  `(1U &lt;&lt; 2)`<br> |
| define  | [**SYN\_CIA402\_CW\_RELATIVE**](syn__cia402_8h.md#define-syn_cia402_cw_relative)  `(1U &lt;&lt; 6)`<br> |
| define  | [**SYN\_CIA402\_CW\_SWITCH\_ON**](syn__cia402_8h.md#define-syn_cia402_cw_switch_on)  `(1U &lt;&lt; 0)`<br> |
| define  | [**SYN\_CIA402\_OD\_CONTROLWORD**](syn__cia402_8h.md#define-syn_cia402_od_controlword)  `0x6040U`<br> |
| define  | [**SYN\_CIA402\_OD\_FOLLOWING\_ERROR\_WINDOW**](syn__cia402_8h.md#define-syn_cia402_od_following_error_window)  `0x6065U`<br> |
| define  | [**SYN\_CIA402\_OD\_HOMING\_ACCELERATION**](syn__cia402_8h.md#define-syn_cia402_od_homing_acceleration)  `0x609BU`<br> |
| define  | [**SYN\_CIA402\_OD\_HOMING\_METHOD**](syn__cia402_8h.md#define-syn_cia402_od_homing_method)  `0x6098U`<br> |
| define  | [**SYN\_CIA402\_OD\_HOMING\_SPEED\_FAST**](syn__cia402_8h.md#define-syn_cia402_od_homing_speed_fast)  `0x6099U`<br> |
| define  | [**SYN\_CIA402\_OD\_HOMING\_SPEED\_SLOW**](syn__cia402_8h.md#define-syn_cia402_od_homing_speed_slow)  `0x609AU`<br> |
| define  | [**SYN\_CIA402\_OD\_MAX\_TORQUE**](syn__cia402_8h.md#define-syn_cia402_od_max_torque)  `0x6072U`<br> |
| define  | [**SYN\_CIA402\_OD\_MODES\_OF\_OPERATION**](syn__cia402_8h.md#define-syn_cia402_od_modes_of_operation)  `0x6060U`<br> |
| define  | [**SYN\_CIA402\_OD\_MODES\_OF\_OPERATION\_DISPLAY**](syn__cia402_8h.md#define-syn_cia402_od_modes_of_operation_display)  `0x6061U`<br> |
| define  | [**SYN\_CIA402\_OD\_POSITION\_ACTUAL\_VALUE**](syn__cia402_8h.md#define-syn_cia402_od_position_actual_value)  `0x6064U`<br> |
| define  | [**SYN\_CIA402\_OD\_POSITION\_DEMAND\_VALUE**](syn__cia402_8h.md#define-syn_cia402_od_position_demand_value)  `0x6062U`<br> |
| define  | [**SYN\_CIA402\_OD\_POSITION\_WINDOW**](syn__cia402_8h.md#define-syn_cia402_od_position_window)  `0x6067U`<br> |
| define  | [**SYN\_CIA402\_OD\_PROFILE\_ACCELERATION**](syn__cia402_8h.md#define-syn_cia402_od_profile_acceleration)  `0x6083U`<br> |
| define  | [**SYN\_CIA402\_OD\_PROFILE\_DECELERATION**](syn__cia402_8h.md#define-syn_cia402_od_profile_deceleration)  `0x6084U`<br> |
| define  | [**SYN\_CIA402\_OD\_PROFILE\_VELOCITY**](syn__cia402_8h.md#define-syn_cia402_od_profile_velocity)  `0x6081U`<br> |
| define  | [**SYN\_CIA402\_OD\_QUICK\_STOP\_DECELERATION**](syn__cia402_8h.md#define-syn_cia402_od_quick_stop_deceleration)  `0x6085U`<br> |
| define  | [**SYN\_CIA402\_OD\_QUICK\_STOP\_OPTION**](syn__cia402_8h.md#define-syn_cia402_od_quick_stop_option)  `0x605AU`<br> |
| define  | [**SYN\_CIA402\_OD\_SOFTWARE\_POS\_LIMIT\_MAX**](syn__cia402_8h.md#define-syn_cia402_od_software_pos_limit_max)  `0x607EU`<br> |
| define  | [**SYN\_CIA402\_OD\_SOFTWARE\_POS\_LIMIT\_MIN**](syn__cia402_8h.md#define-syn_cia402_od_software_pos_limit_min)  `0x607DU`<br> |
| define  | [**SYN\_CIA402\_OD\_STATUSWORD**](syn__cia402_8h.md#define-syn_cia402_od_statusword)  `0x6041U`<br> |
| define  | [**SYN\_CIA402\_OD\_TARGET\_POSITION**](syn__cia402_8h.md#define-syn_cia402_od_target_position)  `0x607AU`<br> |
| define  | [**SYN\_CIA402\_OD\_TARGET\_TORQUE**](syn__cia402_8h.md#define-syn_cia402_od_target_torque)  `0x6071U`<br> |
| define  | [**SYN\_CIA402\_OD\_TARGET\_VELOCITY**](syn__cia402_8h.md#define-syn_cia402_od_target_velocity)  `0x60FFU`<br> |
| define  | [**SYN\_CIA402\_OD\_TORQUE\_ACTUAL\_VALUE**](syn__cia402_8h.md#define-syn_cia402_od_torque_actual_value)  `0x6077U`<br> |
| define  | [**SYN\_CIA402\_OD\_TORQUE\_SLOPE**](syn__cia402_8h.md#define-syn_cia402_od_torque_slope)  `0x6087U`<br> |
| define  | [**SYN\_CIA402\_OD\_VELOCITY\_ACTUAL\_VALUE**](syn__cia402_8h.md#define-syn_cia402_od_velocity_actual_value)  `0x606CU`<br> |
| define  | [**SYN\_CIA402\_SW\_FAULT**](syn__cia402_8h.md#define-syn_cia402_sw_fault)  `(1U &lt;&lt; 3)`<br> |
| define  | [**SYN\_CIA402\_SW\_FOLLOWING\_ERROR**](syn__cia402_8h.md#define-syn_cia402_sw_following_error)  `(1U &lt;&lt; 13)`<br> |
| define  | [**SYN\_CIA402\_SW\_HOMING\_ATTAINED**](syn__cia402_8h.md#define-syn_cia402_sw_homing_attained)  `(1U &lt;&lt; 12)`<br> |
| define  | [**SYN\_CIA402\_SW\_HOMING\_ERROR**](syn__cia402_8h.md#define-syn_cia402_sw_homing_error)  `(1U &lt;&lt; 13)`<br> |
| define  | [**SYN\_CIA402\_SW\_INTERNAL\_LIMIT**](syn__cia402_8h.md#define-syn_cia402_sw_internal_limit)  `(1U &lt;&lt; 11)`<br> |
| define  | [**SYN\_CIA402\_SW\_MANUFACTURER\_8**](syn__cia402_8h.md#define-syn_cia402_sw_manufacturer_8)  `(1U &lt;&lt; 8)`<br> |
| define  | [**SYN\_CIA402\_SW\_OPERATION\_ENABLED**](syn__cia402_8h.md#define-syn_cia402_sw_operation_enabled)  `(1U &lt;&lt; 2)`<br> |
| define  | [**SYN\_CIA402\_SW\_QUICK\_STOP**](syn__cia402_8h.md#define-syn_cia402_sw_quick_stop)  `(1U &lt;&lt; 5)`<br> |
| define  | [**SYN\_CIA402\_SW\_READY\_TO\_SWITCH\_ON**](syn__cia402_8h.md#define-syn_cia402_sw_ready_to_switch_on)  `(1U &lt;&lt; 0)`<br> |
| define  | [**SYN\_CIA402\_SW\_REMOTE**](syn__cia402_8h.md#define-syn_cia402_sw_remote)  `(1U &lt;&lt; 9)`<br> |
| define  | [**SYN\_CIA402\_SW\_SETPOINT\_ACK**](syn__cia402_8h.md#define-syn_cia402_sw_setpoint_ack)  `(1U &lt;&lt; 12)`<br> |
| define  | [**SYN\_CIA402\_SW\_SPEED\_ZERO**](syn__cia402_8h.md#define-syn_cia402_sw_speed_zero)  `(1U &lt;&lt; 12)`<br> |
| define  | [**SYN\_CIA402\_SW\_SWITCHED\_ON**](syn__cia402_8h.md#define-syn_cia402_sw_switched_on)  `(1U &lt;&lt; 1)`<br> |
| define  | [**SYN\_CIA402\_SW\_SWITCH\_ON\_DISABLED**](syn__cia402_8h.md#define-syn_cia402_sw_switch_on_disabled)  `(1U &lt;&lt; 6)`<br> |
| define  | [**SYN\_CIA402\_SW\_TARGET\_REACHED**](syn__cia402_8h.md#define-syn_cia402_sw_target_reached)  `(1U &lt;&lt; 10)`<br> |
| define  | [**SYN\_CIA402\_SW\_VOLTAGE\_ENABLED**](syn__cia402_8h.md#define-syn_cia402_sw_voltage_enabled)  `(1U &lt;&lt; 4)`<br> |
| define  | [**SYN\_CIA402\_SW\_WARNING**](syn__cia402_8h.md#define-syn_cia402_sw_warning)  `(1U &lt;&lt; 7)`<br> |

## Detailed Description


Implements a zero-heap, deterministic CiA 402 Drive Finite State Machine (Controlword 0x6040 / Statusword 0x6041), Motion Modes of Operation (PP, PV, PT, HM, CSP, CSV, CST), trapezoidal trajectory generation, following error supervision, and standard CANopen Object Dictionary binding. 


    
## Public Types Documentation




### enum SYN\_CIA402\_Mode 

_CiA 402 operation modes._ 
```C++
enum SYN_CIA402_Mode {
    SYN_CIA402_MODE_NO_MODE = 0,
    SYN_CIA402_MODE_PROFILE_POSITION = 1,
    SYN_CIA402_MODE_PP = 1,
    SYN_CIA402_MODE_PROFILE_VELOCITY = 3,
    SYN_CIA402_MODE_PV = 3,
    SYN_CIA402_MODE_PROFILE_TORQUE = 4,
    SYN_CIA402_MODE_PT = 4,
    SYN_CIA402_MODE_HOMING = 6,
    SYN_CIA402_MODE_HM = 6,
    SYN_CIA402_MODE_CYCLIC_SYNC_POSITION = 8,
    SYN_CIA402_MODE_CSP = 8,
    SYN_CIA402_MODE_CYCLIC_SYNC_VELOCITY = 9,
    SYN_CIA402_MODE_CSV = 9,
    SYN_CIA402_MODE_CYCLIC_SYNC_TORQUE = 10,
    SYN_CIA402_MODE_CST = 10
};
```




<hr>



### enum SYN\_CIA402\_State 

_CiA 402 Finite State Machine state enumeration._ 
```C++
enum SYN_CIA402_State {
    SYN_CIA402_STATE_NOT_READY_TO_SWITCH_ON = 0,
    SYN_CIA402_STATE_NOT_READY = 0,
    SYN_CIA402_STATE_SWITCH_ON_DISABLED = 1,
    SYN_CIA402_STATE_READY_TO_SWITCH_ON = 2,
    SYN_CIA402_STATE_SWITCHED_ON = 3,
    SYN_CIA402_STATE_OPERATION_ENABLED = 4,
    SYN_CIA402_STATE_QUICK_STOP_ACTIVE = 5,
    SYN_CIA402_STATE_FAULT_REACTION_ACTIVE = 6,
    SYN_CIA402_STATE_FAULT = 7
};
```




<hr>



### typedef SYN\_CiA402Config 

```C++
typedef SYN_CIA402_Config SYN_CiA402Config;
```



Alias for [**SYN\_CIA402\_Config**](structSYN__CIA402__Config.md) for backwards-compatibility. 


        

<hr>



### typedef SYN\_CiA402Drive 

```C++
typedef SYN_CIA402_Axis SYN_CiA402Drive;
```



Alias for [**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) for backwards-compatibility. 


        

<hr>



### typedef SYN\_CiA402State 

```C++
typedef SYN_CIA402_State SYN_CiA402State;
```



Alias for SYN\_CIA402\_State for backwards-compatibility. 


        

<hr>
## Public Functions Documentation




### function syn\_cia402\_feed\_feedback 

_Feed measured actual values (encoder position, velocity, torque) to axis._ 
```C++
SYN_Status syn_cia402_feed_feedback (
    SYN_CIA402_Axis * axis,
    int32_t pos_actual,
    int32_t vel_actual,
    int16_t torque_actual
) 
```





**Parameters:**


* `axis` Axis instance. 
* `pos_actual` Actual position (counts). 
* `vel_actual` Actual velocity (counts/s). 
* `torque_actual` Actual torque (0.1% rated). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on NULL. 





        

<hr>



### function syn\_cia402\_get\_controlword 

_Read current Controlword (Index 0x6040)._ 
```C++
uint16_t syn_cia402_get_controlword (
    const SYN_CIA402_Axis * axis
) 
```





**Parameters:**


* `axis` Axis instance. 



**Returns:**

Controlword value (0 on NULL). 





        

<hr>



### function syn\_cia402\_get\_mode 

_Get requested Mode of Operation (Index 0x6060)._ 
```C++
int8_t syn_cia402_get_mode (
    const SYN_CIA402_Axis * axis
) 
```





**Parameters:**


* `axis` Axis instance. 



**Returns:**

Mode of operation. 





        

<hr>



### function syn\_cia402\_get\_mode\_display 

_Get active Mode of Operation Display (Index 0x6061)._ 
```C++
int8_t syn_cia402_get_mode_display (
    const SYN_CIA402_Axis * axis
) 
```





**Parameters:**


* `axis` Axis instance. 



**Returns:**

Mode of operation display. 





        

<hr>



### function syn\_cia402\_get\_state 

_Get current FSM state._ 
```C++
SYN_CIA402_State syn_cia402_get_state (
    const SYN_CIA402_Axis * axis
) 
```





**Parameters:**


* `axis` Axis instance. 



**Returns:**

Current FSM state. 





        

<hr>



### function syn\_cia402\_get\_statusword 

_Read current Statusword (Index 0x6041)._ 
```C++
uint16_t syn_cia402_get_statusword (
    const SYN_CIA402_Axis * axis
) 
```





**Parameters:**


* `axis` Axis instance. 



**Returns:**

Statusword value (0 on NULL). 





        

<hr>



### function syn\_cia402\_init 

_Initialize CiA 402 Axis controller._ 
```C++
SYN_Status syn_cia402_init (
    SYN_CIA402_Axis * axis,
    const SYN_CIA402_Config * cfg
) 
```





**Parameters:**


* `axis` Axis instance. 
* `cfg` Initial configuration parameters. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on NULL parameters. 





        

<hr>



### function syn\_cia402\_populate\_od 

_Populate a CANopen Object Dictionary table with standard CiA 402 objects._ 
```C++
size_t syn_cia402_populate_od (
    SYN_CANOpenODEntry * od_table,
    size_t max_entries,
    SYN_CIA402_Axis * axis
) 
```



Binds 0x6040 (Controlword), 0x6041 (Statusword), 0x6060..0x60FF objects directly to the live fields of the given axis instance.




**Parameters:**


* `od_table` Destination array of Object Dictionary entries. 
* `max_entries` Size of od\_table. 
* `axis` Axis instance to bind. 



**Returns:**

Number of entries populated (0 on error or insufficient capacity). 





        

<hr>



### function syn\_cia402\_report\_actuals 

_Feed measured actual values alias for compatibility._ 
```C++
SYN_Status syn_cia402_report_actuals (
    SYN_CIA402_Axis * axis,
    int32_t pos_actual,
    int32_t vel_actual,
    int16_t torque_actual
) 
```





**Parameters:**


* `axis` Axis instance. 
* `pos_actual` Actual position (counts). 
* `vel_actual` Actual velocity (counts/s). 
* `torque_actual` Actual torque (0.1% rated). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on NULL. 





        

<hr>



### function syn\_cia402\_set\_controlword 

_Set Controlword (Index 0x6040) from master / RPDO / SDO._ 
```C++
SYN_Status syn_cia402_set_controlword (
    SYN_CIA402_Axis * axis,
    uint16_t cw
) 
```





**Parameters:**


* `axis` Axis instance. 
* `cw` Controlword value. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on NULL. 





        

<hr>



### function syn\_cia402\_set\_mode 

_Request Mode of Operation (Index 0x6060)._ 
```C++
SYN_Status syn_cia402_set_mode (
    SYN_CIA402_Axis * axis,
    int8_t mode
) 
```





**Parameters:**


* `axis` Axis instance. 
* `mode` Requested mode (SYN\_CIA402\_MODE\_\*). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid mode. 





        

<hr>



### function syn\_cia402\_set\_target\_position 

_Set target position (Index 0x607A)._ 
```C++
SYN_Status syn_cia402_set_target_position (
    SYN_CIA402_Axis * axis,
    int32_t pos
) 
```





**Parameters:**


* `axis` Axis instance. 
* `pos` Target position in counts. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on NULL. 





        

<hr>



### function syn\_cia402\_set\_target\_torque 

_Set target torque (Index 0x6071)._ 
```C++
SYN_Status syn_cia402_set_target_torque (
    SYN_CIA402_Axis * axis,
    int16_t torque
) 
```





**Parameters:**


* `axis` Axis instance. 
* `torque` Target torque in 0.1% rated torque. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on NULL. 





        

<hr>



### function syn\_cia402\_set\_target\_velocity 

_Set target velocity (Index 0x60FF)._ 
```C++
SYN_Status syn_cia402_set_target_velocity (
    SYN_CIA402_Axis * axis,
    int32_t vel
) 
```





**Parameters:**


* `axis` Axis instance. 
* `vel` Target velocity in counts/s. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on NULL. 





        

<hr>



### function syn\_cia402\_step 

_Execute periodic CiA 402 control loop step in microseconds._ 
```C++
SYN_Status syn_cia402_step (
    SYN_CIA402_Axis * axis,
    uint32_t dt_us,
    SYN_CIA402_Setpoints * out_setpoints
) 
```



Evaluates state machine transitions, computes motion ramps, performs following error supervision, updates Statusword, and outputs motor setpoints.




**Parameters:**


* `axis` Axis instance. 
* `dt_us` Time elapsed since last step in microseconds. 
* `out_setpoints` Output setpoint commands (may be NULL if not needed). 



**Returns:**

SYN\_OK on success, error code on failure. 





        

<hr>



### function syn\_cia402\_trigger\_fault 

_Trigger a drive fault and enter FAULT\_REACTION\_ACTIVE / FAULT state._ 
```C++
SYN_Status syn_cia402_trigger_fault (
    SYN_CIA402_Axis * axis,
    uint16_t error_code
) 
```





**Parameters:**


* `axis` Axis instance. 
* `error_code` Fault error code (e.g. overcurrent, limit tripped, tracking error). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on NULL. 





        

<hr>



### function syn\_cia402\_update 

_Periodic motion update in milliseconds._ 
```C++
SYN_Status syn_cia402_update (
    SYN_CIA402_Axis * axis,
    uint32_t dt_ms
) 
```





**Parameters:**


* `axis` Axis instance. 
* `dt_ms` Time delta in milliseconds. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on NULL. 





        

<hr>
## Macro Definition Documentation





### define SYN\_CIA402\_CW\_CHANGE\_IMMEDIATELY 

```C++
#define SYN_CIA402_CW_CHANGE_IMMEDIATELY `(1U << 5)`
```



Bit 5: Change Set-point Immediately (PP) \ 


        

<hr>



### define SYN\_CIA402\_CW\_CHANGE\_ON\_SETPOINT 

```C++
#define SYN_CIA402_CW_CHANGE_ON_SETPOINT `(1U << 9)`
```



Bit 9: Change on Set-point (PP) 


        

<hr>



### define SYN\_CIA402\_CW\_ENABLE\_OPERATION 

```C++
#define SYN_CIA402_CW_ENABLE_OPERATION `(1U << 3)`
```



Bit 3: Enable Operation 


        

<hr>



### define SYN\_CIA402\_CW\_ENABLE\_VOLTAGE 

```C++
#define SYN_CIA402_CW_ENABLE_VOLTAGE `(1U << 1)`
```



Bit 1: Enable Voltage 


        

<hr>



### define SYN\_CIA402\_CW\_FAULT\_RESET 

```C++
#define SYN_CIA402_CW_FAULT_RESET `(1U << 7)`
```



Bit 7: Fault Reset (0 -&gt; 1 rising edge) 


        

<hr>



### define SYN\_CIA402\_CW\_HALT 

```C++
#define SYN_CIA402_CW_HALT `(1U << 8)`
```



Bit 8: Halt axis 


        

<hr>



### define SYN\_CIA402\_CW\_NEW\_SETPOINT 

```C++
#define SYN_CIA402_CW_NEW_SETPOINT `(1U << 4)`
```



Bit 4: New Set-point (PP) / Start Homing (HM) 


        

<hr>



### define SYN\_CIA402\_CW\_QUICK\_STOP 

```C++
#define SYN_CIA402_CW_QUICK_STOP `(1U << 2)`
```



Bit 2: Quick Stop (active low: 0=stop) 


        

<hr>



### define SYN\_CIA402\_CW\_RELATIVE 

```C++
#define SYN_CIA402_CW_RELATIVE `(1U << 6)`
```



Bit 6: Relative Positioning (PP: 0=Abs, 1=Rel) 


        

<hr>



### define SYN\_CIA402\_CW\_SWITCH\_ON 

```C++
#define SYN_CIA402_CW_SWITCH_ON `(1U << 0)`
```



Bit 0: Switch On 


        

<hr>



### define SYN\_CIA402\_OD\_CONTROLWORD 

```C++
#define SYN_CIA402_OD_CONTROLWORD `0x6040U`
```



Controlword (U16) 


        

<hr>



### define SYN\_CIA402\_OD\_FOLLOWING\_ERROR\_WINDOW 

```C++
#define SYN_CIA402_OD_FOLLOWING_ERROR_WINDOW `0x6065U`
```



Following Error Window (U32) 


        

<hr>



### define SYN\_CIA402\_OD\_HOMING\_ACCELERATION 

```C++
#define SYN_CIA402_OD_HOMING_ACCELERATION `0x609BU`
```



Homing Acceleration (U32) 


        

<hr>



### define SYN\_CIA402\_OD\_HOMING\_METHOD 

```C++
#define SYN_CIA402_OD_HOMING_METHOD `0x6098U`
```



Homing Method (I8) 


        

<hr>



### define SYN\_CIA402\_OD\_HOMING\_SPEED\_FAST 

```C++
#define SYN_CIA402_OD_HOMING_SPEED_FAST `0x6099U`
```



Homing Speed Switch (U32) 


        

<hr>



### define SYN\_CIA402\_OD\_HOMING\_SPEED\_SLOW 

```C++
#define SYN_CIA402_OD_HOMING_SPEED_SLOW `0x609AU`
```



Homing Speed Zero (U32) 


        

<hr>



### define SYN\_CIA402\_OD\_MAX\_TORQUE 

```C++
#define SYN_CIA402_OD_MAX_TORQUE `0x6072U`
```



Max Torque (U16, 0.1% rated) 


        

<hr>



### define SYN\_CIA402\_OD\_MODES\_OF\_OPERATION 

```C++
#define SYN_CIA402_OD_MODES_OF_OPERATION `0x6060U`
```



Modes of Operation (I8) 


        

<hr>



### define SYN\_CIA402\_OD\_MODES\_OF\_OPERATION\_DISPLAY 

```C++
#define SYN_CIA402_OD_MODES_OF_OPERATION_DISPLAY `0x6061U`
```



Modes of Operation Display (I8) 


        

<hr>



### define SYN\_CIA402\_OD\_POSITION\_ACTUAL\_VALUE 

```C++
#define SYN_CIA402_OD_POSITION_ACTUAL_VALUE `0x6064U`
```



Position Actual Value (I32) 


        

<hr>



### define SYN\_CIA402\_OD\_POSITION\_DEMAND\_VALUE 

```C++
#define SYN_CIA402_OD_POSITION_DEMAND_VALUE `0x6062U`
```



Position Demand Value (I32) 


        

<hr>



### define SYN\_CIA402\_OD\_POSITION\_WINDOW 

```C++
#define SYN_CIA402_OD_POSITION_WINDOW `0x6067U`
```



Position Window (U32) 


        

<hr>



### define SYN\_CIA402\_OD\_PROFILE\_ACCELERATION 

```C++
#define SYN_CIA402_OD_PROFILE_ACCELERATION `0x6083U`
```



Profile Acceleration (U32) 


        

<hr>



### define SYN\_CIA402\_OD\_PROFILE\_DECELERATION 

```C++
#define SYN_CIA402_OD_PROFILE_DECELERATION `0x6084U`
```



Profile Deceleration (U32) 


        

<hr>



### define SYN\_CIA402\_OD\_PROFILE\_VELOCITY 

```C++
#define SYN_CIA402_OD_PROFILE_VELOCITY `0x6081U`
```



Profile Velocity (U32) 


        

<hr>



### define SYN\_CIA402\_OD\_QUICK\_STOP\_DECELERATION 

```C++
#define SYN_CIA402_OD_QUICK_STOP_DECELERATION `0x6085U`
```



Quick Stop Deceleration (U32) 


        

<hr>



### define SYN\_CIA402\_OD\_QUICK\_STOP\_OPTION 

```C++
#define SYN_CIA402_OD_QUICK_STOP_OPTION `0x605AU`
```



Quick Stop Option Code (I16) 


        

<hr>



### define SYN\_CIA402\_OD\_SOFTWARE\_POS\_LIMIT\_MAX 

```C++
#define SYN_CIA402_OD_SOFTWARE_POS_LIMIT_MAX `0x607EU`
```



Software Position Limit Max (I32) 


        

<hr>



### define SYN\_CIA402\_OD\_SOFTWARE\_POS\_LIMIT\_MIN 

```C++
#define SYN_CIA402_OD_SOFTWARE_POS_LIMIT_MIN `0x607DU`
```



Software Position Limit Min (I32) 


        

<hr>



### define SYN\_CIA402\_OD\_STATUSWORD 

```C++
#define SYN_CIA402_OD_STATUSWORD `0x6041U`
```



Statusword (U16) 


        

<hr>



### define SYN\_CIA402\_OD\_TARGET\_POSITION 

```C++
#define SYN_CIA402_OD_TARGET_POSITION `0x607AU`
```



Target Position (I32) 


        

<hr>



### define SYN\_CIA402\_OD\_TARGET\_TORQUE 

```C++
#define SYN_CIA402_OD_TARGET_TORQUE `0x6071U`
```



Target Torque (I16, 0.1% rated) 


        

<hr>



### define SYN\_CIA402\_OD\_TARGET\_VELOCITY 

```C++
#define SYN_CIA402_OD_TARGET_VELOCITY `0x60FFU`
```



Target Velocity (I32) 


        

<hr>



### define SYN\_CIA402\_OD\_TORQUE\_ACTUAL\_VALUE 

```C++
#define SYN_CIA402_OD_TORQUE_ACTUAL_VALUE `0x6077U`
```



Torque Actual Value (I16) 


        

<hr>



### define SYN\_CIA402\_OD\_TORQUE\_SLOPE 

```C++
#define SYN_CIA402_OD_TORQUE_SLOPE `0x6087U`
```



Torque Slope (U32, units/s) 


        

<hr>



### define SYN\_CIA402\_OD\_VELOCITY\_ACTUAL\_VALUE 

```C++
#define SYN_CIA402_OD_VELOCITY_ACTUAL_VALUE `0x606CU`
```



Velocity Actual Value (I32) 


        

<hr>



### define SYN\_CIA402\_SW\_FAULT 

```C++
#define SYN_CIA402_SW_FAULT `(1U << 3)`
```



Bit 3: Fault Active 


        

<hr>



### define SYN\_CIA402\_SW\_FOLLOWING\_ERROR 

```C++
#define SYN_CIA402_SW_FOLLOWING_ERROR `(1U << 13)`
```



Bit 13: Following Error Active (PP) 


        

<hr>



### define SYN\_CIA402\_SW\_HOMING\_ATTAINED 

```C++
#define SYN_CIA402_SW_HOMING_ATTAINED `(1U << 12)`
```



Bit 12: Homing Attained (HM) 


        

<hr>



### define SYN\_CIA402\_SW\_HOMING\_ERROR 

```C++
#define SYN_CIA402_SW_HOMING_ERROR `(1U << 13)`
```



Bit 13: Homing Error (HM) 


        

<hr>



### define SYN\_CIA402\_SW\_INTERNAL\_LIMIT 

```C++
#define SYN_CIA402_SW_INTERNAL_LIMIT `(1U << 11)`
```



Bit 11: Internal / Software Limit Active 


        

<hr>



### define SYN\_CIA402\_SW\_MANUFACTURER\_8 

```C++
#define SYN_CIA402_SW_MANUFACTURER_8 `(1U << 8)`
```



Bit 8: Manufacturer Specific 


        

<hr>



### define SYN\_CIA402\_SW\_OPERATION\_ENABLED 

```C++
#define SYN_CIA402_SW_OPERATION_ENABLED `(1U << 2)`
```



Bit 2: Operation Enabled 


        

<hr>



### define SYN\_CIA402\_SW\_QUICK\_STOP 

```C++
#define SYN_CIA402_SW_QUICK_STOP `(1U << 5)`
```



Bit 5: Quick Stop Inactive (1=normal, 0=quick stop) 


        

<hr>



### define SYN\_CIA402\_SW\_READY\_TO\_SWITCH\_ON 

```C++
#define SYN_CIA402_SW_READY_TO_SWITCH_ON `(1U << 0)`
```



Bit 0: Ready to Switch On 


        

<hr>



### define SYN\_CIA402\_SW\_REMOTE 

```C++
#define SYN_CIA402_SW_REMOTE `(1U << 9)`
```



Bit 9: Remote Control Active 


        

<hr>



### define SYN\_CIA402\_SW\_SETPOINT\_ACK 

```C++
#define SYN_CIA402_SW_SETPOINT_ACK `(1U << 12)`
```



Bit 12: Setpoint Acknowledge (PP) 


        

<hr>



### define SYN\_CIA402\_SW\_SPEED\_ZERO 

```C++
#define SYN_CIA402_SW_SPEED_ZERO `(1U << 12)`
```



Bit 12: Speed 0 (PV) 


        

<hr>



### define SYN\_CIA402\_SW\_SWITCHED\_ON 

```C++
#define SYN_CIA402_SW_SWITCHED_ON `(1U << 1)`
```



Bit 1: Switched On 


        

<hr>



### define SYN\_CIA402\_SW\_SWITCH\_ON\_DISABLED 

```C++
#define SYN_CIA402_SW_SWITCH_ON_DISABLED `(1U << 6)`
```



Bit 6: Switch On Disabled 


        

<hr>



### define SYN\_CIA402\_SW\_TARGET\_REACHED 

```C++
#define SYN_CIA402_SW_TARGET_REACHED `(1U << 10)`
```



Bit 10: Target Position / Velocity Reached 


        

<hr>



### define SYN\_CIA402\_SW\_VOLTAGE\_ENABLED 

```C++
#define SYN_CIA402_SW_VOLTAGE_ENABLED `(1U << 4)`
```



Bit 4: Power Stage Voltage Enabled 


        

<hr>



### define SYN\_CIA402\_SW\_WARNING 

```C++
#define SYN_CIA402_SW_WARNING `(1U << 7)`
```



Bit 7: Drive Warning 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cia402.h`

