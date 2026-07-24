

# File syn\_cia402.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cia402.c**](syn__cia402_8c.md)

[Go to the source code of this file](syn__cia402_8c_source.md)

_CANopen CiA 402 Servo Drive Profile Engine implementation._ 

* `#include "syntropic/proto/syn_cia402.h"`
* `#include <string.h>`
* `#include <math.h>`





































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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cia402.c`

