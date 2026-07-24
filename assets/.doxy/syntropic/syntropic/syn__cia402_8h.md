

# File syn\_cia402.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cia402.h**](syn__cia402_8h.md)

[Go to the source code of this file](syn__cia402_8h_source.md)

_CANopen CiA 402 Servo Drive & Motion Control Profile Engine._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include "../common/syn_defs.h"`
* `#include "../util/syn_scurve.h"`
* `#include "syn_canopen.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_CiA402Config**](structSYN__CiA402Config.md) <br>_Configuration parameters for CiA 402 Drive._  |
| struct | [**SYN\_CiA402Drive**](structSYN__CiA402Drive.md) <br>_CiA 402 Drive Handle._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_CiA402Mode**](#enum-syn_cia402mode)  <br>_CiA 402 Modes of Operation (0x6060 / 0x6061)_  |
| enum  | [**SYN\_CiA402State**](#enum-syn_cia402state)  <br>_CiA 402 Drive State Machine States._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**syn\_cia402\_get\_statusword**](#function-syn_cia402_get_statusword) (const [**SYN\_CiA402Drive**](structSYN__CiA402Drive.md) \* drive) <br>_Retrieve current 16-bit Statusword value._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cia402\_init**](#function-syn_cia402_init) ([**SYN\_CiA402Drive**](structSYN__CiA402Drive.md) \* drive, const [**SYN\_CiA402Config**](structSYN__CiA402Config.md) \* cfg) <br>_Initialize a CiA 402 Drive instance._  |
|  size\_t | [**syn\_cia402\_populate\_od**](#function-syn_cia402_populate_od) ([**SYN\_CANOpenODEntry**](structSYN__CANOpenODEntry.md) \* entries, size\_t max\_entries, [**SYN\_CiA402Drive**](structSYN__CiA402Drive.md) \* drive) <br>_Populate a standard CANopen Object Dictionary array with CiA 402 drive objects._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cia402\_report\_actuals**](#function-syn_cia402_report_actuals) ([**SYN\_CiA402Drive**](structSYN__CiA402Drive.md) \* drive, int32\_t actual\_pos, int32\_t actual\_vel, int16\_t actual\_torque) <br>_Report actual motor feedback (position, velocity, torque)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cia402\_set\_controlword**](#function-syn_cia402_set_controlword) ([**SYN\_CiA402Drive**](structSYN__CiA402Drive.md) \* drive, uint16\_t controlword) <br>_Process new Controlword command and update drive state machine._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cia402\_set\_mode**](#function-syn_cia402_set_mode) ([**SYN\_CiA402Drive**](structSYN__CiA402Drive.md) \* drive, int8\_t mode) <br>_Set requested Mode of Operation._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cia402\_set\_target\_position**](#function-syn_cia402_set_target_position) ([**SYN\_CiA402Drive**](structSYN__CiA402Drive.md) \* drive, int32\_t target\_pos) <br>_Set target position for PP or CSP mode._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cia402\_set\_target\_torque**](#function-syn_cia402_set_target_torque) ([**SYN\_CiA402Drive**](structSYN__CiA402Drive.md) \* drive, int16\_t target\_torque) <br>_Set target torque for PT or CST mode._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cia402\_set\_target\_velocity**](#function-syn_cia402_set_target_velocity) ([**SYN\_CiA402Drive**](structSYN__CiA402Drive.md) \* drive, int32\_t target\_vel) <br>_Set target velocity for PV or CSV mode._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cia402\_trigger\_fault**](#function-syn_cia402_trigger_fault) ([**SYN\_CiA402Drive**](structSYN__CiA402Drive.md) \* drive, uint16\_t error\_code) <br>_Trigger a fault state in the drive._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_cia402\_update**](#function-syn_cia402_update) ([**SYN\_CiA402Drive**](structSYN__CiA402Drive.md) \* drive, uint32\_t dt\_ms) <br>_Periodic motion control update step._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_CIA402\_CW\_ABS\_REL**](syn__cia402_8h.md#define-syn_cia402_cw_abs_rel)  `(1U &lt;&lt; 6)`<br> |
| define  | [**SYN\_CIA402\_CW\_CHANGE\_IMMED**](syn__cia402_8h.md#define-syn_cia402_cw_change_immed)  `(1U &lt;&lt; 5)`<br> |
| define  | [**SYN\_CIA402\_CW\_ENABLE\_OP**](syn__cia402_8h.md#define-syn_cia402_cw_enable_op)  `(1U &lt;&lt; 3)`<br> |
| define  | [**SYN\_CIA402\_CW\_ENABLE\_VOLTAGE**](syn__cia402_8h.md#define-syn_cia402_cw_enable_voltage)  `(1U &lt;&lt; 1)`<br> |
| define  | [**SYN\_CIA402\_CW\_FAULT\_RESET**](syn__cia402_8h.md#define-syn_cia402_cw_fault_reset)  `(1U &lt;&lt; 7)`<br> |
| define  | [**SYN\_CIA402\_CW\_HALT**](syn__cia402_8h.md#define-syn_cia402_cw_halt)  `(1U &lt;&lt; 8)`<br> |
| define  | [**SYN\_CIA402\_CW\_NEW\_SETPOINT**](syn__cia402_8h.md#define-syn_cia402_cw_new_setpoint)  `(1U &lt;&lt; 4)`<br> |
| define  | [**SYN\_CIA402\_CW\_QUICK\_STOP**](syn__cia402_8h.md#define-syn_cia402_cw_quick_stop)  `(1U &lt;&lt; 2)`<br> |
| define  | [**SYN\_CIA402\_CW\_SWITCH\_ON**](syn__cia402_8h.md#define-syn_cia402_cw_switch_on)  `(1U &lt;&lt; 0)`<br> |
| define  | [**SYN\_CIA402\_SW\_FAULT**](syn__cia402_8h.md#define-syn_cia402_sw_fault)  `(1U &lt;&lt; 3)`<br> |
| define  | [**SYN\_CIA402\_SW\_INTERNAL\_LIMIT**](syn__cia402_8h.md#define-syn_cia402_sw_internal_limit)  `(1U &lt;&lt; 11)`<br> |
| define  | [**SYN\_CIA402\_SW\_OPERATION\_ENABLED**](syn__cia402_8h.md#define-syn_cia402_sw_operation_enabled)  `(1U &lt;&lt; 2)`<br> |
| define  | [**SYN\_CIA402\_SW\_QUICK\_STOP**](syn__cia402_8h.md#define-syn_cia402_sw_quick_stop)  `(1U &lt;&lt; 5)`<br> |
| define  | [**SYN\_CIA402\_SW\_READY\_TO\_SWITCH\_ON**](syn__cia402_8h.md#define-syn_cia402_sw_ready_to_switch_on)  `(1U &lt;&lt; 0)`<br> |
| define  | [**SYN\_CIA402\_SW\_SETPOINT\_ACK**](syn__cia402_8h.md#define-syn_cia402_sw_setpoint_ack)  `(1U &lt;&lt; 12)`<br> |
| define  | [**SYN\_CIA402\_SW\_SWITCHED\_ON**](syn__cia402_8h.md#define-syn_cia402_sw_switched_on)  `(1U &lt;&lt; 1)`<br> |
| define  | [**SYN\_CIA402\_SW\_SWITCH\_ON\_DISABLED**](syn__cia402_8h.md#define-syn_cia402_sw_switch_on_disabled)  `(1U &lt;&lt; 6)`<br> |
| define  | [**SYN\_CIA402\_SW\_TARGET\_REACHED**](syn__cia402_8h.md#define-syn_cia402_sw_target_reached)  `(1U &lt;&lt; 10)`<br> |
| define  | [**SYN\_CIA402\_SW\_VOLTAGE\_ENABLED**](syn__cia402_8h.md#define-syn_cia402_sw_voltage_enabled)  `(1U &lt;&lt; 4)`<br> |
| define  | [**SYN\_CIA402\_SW\_WARNING**](syn__cia402_8h.md#define-syn_cia402_sw_warning)  `(1U &lt;&lt; 7)`<br> |

## Detailed Description


Implements the standard CiA 402 power drive state machine, Controlword (0x6040) processing, Statusword (0x6041) status reporting, and motion modes of operation (Profile Position, Profile Velocity, Profile Torque, Homing, and Cyclic Synchronous Position/Velocity).


Designed to bind directly into syn\_canopen Object Dictionary tables. 


    
## Public Types Documentation




### enum SYN\_CiA402Mode 

_CiA 402 Modes of Operation (0x6060 / 0x6061)_ 
```C++
enum SYN_CiA402Mode {
    SYN_CIA402_MODE_NO_MODE = 0,
    SYN_CIA402_MODE_PP = 1,
    SYN_CIA402_MODE_PV = 3,
    SYN_CIA402_MODE_PT = 4,
    SYN_CIA402_MODE_HM = 6,
    SYN_CIA402_MODE_CSP = 8,
    SYN_CIA402_MODE_CSV = 9,
    SYN_CIA402_MODE_CST = 10
};
```




<hr>



### enum SYN\_CiA402State 

_CiA 402 Drive State Machine States._ 
```C++
enum SYN_CiA402State {
    SYN_CIA402_STATE_NOT_READY = 0,
    SYN_CIA402_STATE_SWITCH_ON_DISABLED = 1,
    SYN_CIA402_STATE_READY_TO_SWITCH_ON = 2,
    SYN_CIA402_STATE_SWITCHED_ON = 3,
    SYN_CIA402_STATE_OPERATION_ENABLED = 4,
    SYN_CIA402_STATE_QUICK_STOP_ACTIVE = 5,
    SYN_CIA402_STATE_FAULT_REACTION = 6,
    SYN_CIA402_STATE_FAULT = 7
};
```




<hr>
## Public Functions Documentation




### function syn\_cia402\_get\_statusword 

_Retrieve current 16-bit Statusword value._ 
```C++
uint16_t syn_cia402_get_statusword (
    const SYN_CiA402Drive * drive
) 
```





**Parameters:**


* `drive` Pointer to drive handle. 



**Returns:**

16-bit Statusword (0x6041), or 0 if drive is NULL. 





        

<hr>



### function syn\_cia402\_init 

_Initialize a CiA 402 Drive instance._ 
```C++
SYN_Status syn_cia402_init (
    SYN_CiA402Drive * drive,
    const SYN_CiA402Config * cfg
) 
```





**Parameters:**


* `drive` Pointer to drive handle. 
* `cfg` Pointer to drive configuration. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if pointers are NULL. 





        

<hr>



### function syn\_cia402\_populate\_od 

_Populate a standard CANopen Object Dictionary array with CiA 402 drive objects._ 
```C++
size_t syn_cia402_populate_od (
    SYN_CANOpenODEntry * entries,
    size_t max_entries,
    SYN_CiA402Drive * drive
) 
```



Creates entries for 0x6040, 0x6041, 0x6060, 0x6061, 0x6064, 0x606C, 0x6071, 0x6077, 0x607A, 0x60FF.




**Parameters:**


* `entries` Output array of [**SYN\_CANOpenODEntry**](structSYN__CANOpenODEntry.md) elements. 
* `max_entries` Maximum capacity of entries array (must be &gt;= 10). 
* `drive` Pointer to drive handle. 



**Returns:**

Number of entries populated, or 0 on error. 





        

<hr>



### function syn\_cia402\_report\_actuals 

_Report actual motor feedback (position, velocity, torque)._ 
```C++
SYN_Status syn_cia402_report_actuals (
    SYN_CiA402Drive * drive,
    int32_t actual_pos,
    int32_t actual_vel,
    int16_t actual_torque
) 
```





**Parameters:**


* `drive` Pointer to drive handle. 
* `actual_pos` Actual measured position (0x6064). 
* `actual_vel` Actual measured velocity (0x606C). 
* `actual_torque` Actual measured torque (0x6077). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if drive is NULL. 





        

<hr>



### function syn\_cia402\_set\_controlword 

_Process new Controlword command and update drive state machine._ 
```C++
SYN_Status syn_cia402_set_controlword (
    SYN_CiA402Drive * drive,
    uint16_t controlword
) 
```





**Parameters:**


* `drive` Pointer to drive handle. 
* `controlword` Received 16-bit Controlword (0x6040). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if drive is NULL. 





        

<hr>



### function syn\_cia402\_set\_mode 

_Set requested Mode of Operation._ 
```C++
SYN_Status syn_cia402_set_mode (
    SYN_CiA402Drive * drive,
    int8_t mode
) 
```





**Parameters:**


* `drive` Pointer to drive handle. 
* `mode` Target mode (SYN\_CIA402\_MODE\_\*). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid inputs. 





        

<hr>



### function syn\_cia402\_set\_target\_position 

_Set target position for PP or CSP mode._ 
```C++
SYN_Status syn_cia402_set_target_position (
    SYN_CiA402Drive * drive,
    int32_t target_pos
) 
```





**Parameters:**


* `drive` Pointer to drive handle. 
* `target_pos` Target position value (0x607A). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if drive is NULL. 





        

<hr>



### function syn\_cia402\_set\_target\_torque 

_Set target torque for PT or CST mode._ 
```C++
SYN_Status syn_cia402_set_target_torque (
    SYN_CiA402Drive * drive,
    int16_t target_torque
) 
```





**Parameters:**


* `drive` Pointer to drive handle. 
* `target_torque` Target torque value (0x6071). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if drive is NULL. 





        

<hr>



### function syn\_cia402\_set\_target\_velocity 

_Set target velocity for PV or CSV mode._ 
```C++
SYN_Status syn_cia402_set_target_velocity (
    SYN_CiA402Drive * drive,
    int32_t target_vel
) 
```





**Parameters:**


* `drive` Pointer to drive handle. 
* `target_vel` Target velocity value (0x60FF). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if drive is NULL. 





        

<hr>



### function syn\_cia402\_trigger\_fault 

_Trigger a fault state in the drive._ 
```C++
SYN_Status syn_cia402_trigger_fault (
    SYN_CiA402Drive * drive,
    uint16_t error_code
) 
```





**Parameters:**


* `drive` Pointer to drive handle. 
* `error_code` 16-bit error code (0x603F). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if drive is NULL. 





        

<hr>



### function syn\_cia402\_update 

_Periodic motion control update step._ 
```C++
SYN_Status syn_cia402_update (
    SYN_CiA402Drive * drive,
    uint32_t dt_ms
) 
```



Executes trajectory planning (S-Curve PP, PV ramps) and state machine updates.




**Parameters:**


* `drive` Pointer to drive handle. 
* `dt_ms` Elapsed time step in milliseconds. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if drive is NULL. 





        

<hr>
## Macro Definition Documentation





### define SYN\_CIA402\_CW\_ABS\_REL 

```C++
#define SYN_CIA402_CW_ABS_REL `(1U << 6)`
```



Bit 6: PP Absolute(0) / Relative(1) 


        

<hr>



### define SYN\_CIA402\_CW\_CHANGE\_IMMED 

```C++
#define SYN_CIA402_CW_CHANGE_IMMED `(1U << 5)`
```



Bit 5: PP Change Immediately 


        

<hr>



### define SYN\_CIA402\_CW\_ENABLE\_OP 

```C++
#define SYN_CIA402_CW_ENABLE_OP `(1U << 3)`
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



Bit 7: Fault Reset (0-&gt;1 transition) 


        

<hr>



### define SYN\_CIA402\_CW\_HALT 

```C++
#define SYN_CIA402_CW_HALT `(1U << 8)`
```



Bit 8: Halt motion 


        

<hr>



### define SYN\_CIA402\_CW\_NEW\_SETPOINT 

```C++
#define SYN_CIA402_CW_NEW_SETPOINT `(1U << 4)`
```



Bit 4: PP New Setpoint 


        

<hr>



### define SYN\_CIA402\_CW\_QUICK\_STOP 

```C++
#define SYN_CIA402_CW_QUICK_STOP `(1U << 2)`
```



Bit 2: Quick Stop (active low) 


        

<hr>



### define SYN\_CIA402\_CW\_SWITCH\_ON 

```C++
#define SYN_CIA402_CW_SWITCH_ON `(1U << 0)`
```



Bit 0: Switch On 


        

<hr>



### define SYN\_CIA402\_SW\_FAULT 

```C++
#define SYN_CIA402_SW_FAULT `(1U << 3)`
```



Bit 3: Fault 


        

<hr>



### define SYN\_CIA402\_SW\_INTERNAL\_LIMIT 

```C++
#define SYN_CIA402_SW_INTERNAL_LIMIT `(1U << 11)`
```



Bit 11: Internal Limit Active 


        

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



Bit 5: Quick Stop (1 = not active) 


        

<hr>



### define SYN\_CIA402\_SW\_READY\_TO\_SWITCH\_ON 

```C++
#define SYN_CIA402_SW_READY_TO_SWITCH_ON `(1U << 0)`
```



Bit 0: Ready to Switch On 


        

<hr>



### define SYN\_CIA402\_SW\_SETPOINT\_ACK 

```C++
#define SYN_CIA402_SW_SETPOINT_ACK `(1U << 12)`
```



Bit 12: Setpoint Acknowledge 


        

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



Bit 10: Target Reached 


        

<hr>



### define SYN\_CIA402\_SW\_VOLTAGE\_ENABLED 

```C++
#define SYN_CIA402_SW_VOLTAGE_ENABLED `(1U << 4)`
```



Bit 4: Voltage Enabled 


        

<hr>



### define SYN\_CIA402\_SW\_WARNING 

```C++
#define SYN_CIA402_SW_WARNING `(1U << 7)`
```



Bit 7: Warning 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cia402.h`

