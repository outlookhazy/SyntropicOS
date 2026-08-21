

# File syn\_cia402.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cia402.c**](syn__cia402_8c.md)

[Go to the source code of this file](syn__cia402_8c_source.md)

_CANopen CiA 402 Device Profile for Drives and Motion Control implementation._ 

* `#include "../util/syn_assert.h"`
* `#include "syn_cia402.h"`
* `#include <stdlib.h>`
* `#include <string.h>`





































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


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_cia402\_eval\_fsm**](#function-syn_cia402_eval_fsm) ([**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) \* axis) <br>_Evaluate CiA 402 Device Control State Machine transitions._  |
|  void | [**syn\_cia402\_update\_statusword**](#function-syn_cia402_update_statusword) ([**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md) \* axis) <br>_Construct CANopen Statusword (0x6041) from current drive state._  |


























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
## Public Static Functions Documentation




### function syn\_cia402\_eval\_fsm 

_Evaluate CiA 402 Device Control State Machine transitions._ 
```C++
static void syn_cia402_eval_fsm (
    SYN_CIA402_Axis * axis
) 
```





**Parameters:**


* `axis` CiA 402 axis instance. 




        

<hr>



### function syn\_cia402\_update\_statusword 

_Construct CANopen Statusword (0x6041) from current drive state._ 
```C++
static void syn_cia402_update_statusword (
    SYN_CIA402_Axis * axis
) 
```





**Parameters:**


* `axis` CiA 402 axis instance. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cia402.c`

