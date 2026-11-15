

# File syn\_flight\_pid.c



[**FileList**](files.md) **>** [**control**](dir_d63400c13859c7c99cca12fba8ce6e16.md) **>** [**syn\_flight\_pid.c**](syn__flight__pid_8c.md)

[Go to the source code of this file](syn__flight__pid_8c_source.md)

_Zero-Heap 3-Axis Quadcopter Flight PID Stabilization & Motor Mixer Implementation._ 

* `#include "syntropic/control/syn_flight_pid.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_flight\_init**](#function-syn_flight_init) ([**SYN\_Flight\_Controller**](structSYN__Flight__Controller.md) \* fc) <br>_Initialize 3-axis flight PID controller instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_flight\_update**](#function-syn_flight_update) ([**SYN\_Flight\_Controller**](structSYN__Flight__Controller.md) \* fc, const [**SYN\_Flight\_IMU**](structSYN__Flight__IMU.md) \* imu, const [**SYN\_Flight\_Commands**](structSYN__Flight__Commands.md) \* cmd, uint32\_t dt\_ms, [**SYN\_Flight\_MotorOutputs**](structSYN__Flight__MotorOutputs.md) \* motors) <br>_Step 3-axis flight controller and compute Quad-X motor outputs._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**clamp\_us**](#function-clamp_us) (int32\_t val) <br>_Clamp pulse width value to 1000..2000 us servo range._  |


























## Public Functions Documentation




### function syn\_flight\_init 

_Initialize 3-axis flight PID controller instance._ 
```C++
SYN_Status syn_flight_init (
    SYN_Flight_Controller * fc
) 
```





**Parameters:**


* `fc` Pointer to flight controller struct. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_flight\_update 

_Step 3-axis flight controller and compute Quad-X motor outputs._ 
```C++
SYN_Status syn_flight_update (
    SYN_Flight_Controller * fc,
    const SYN_Flight_IMU * imu,
    const SYN_Flight_Commands * cmd,
    uint32_t dt_ms,
    SYN_Flight_MotorOutputs * motors
) 
```





**Parameters:**


* `fc` Pointer to flight controller struct. 
* `imu` Pointer to IMU measurements. 
* `cmd` Pointer to pilot command inputs. 
* `dt_ms` Time step in milliseconds (e.g. 1ms = 1kHz loop). 
* `motors` Pointer to output motor commands struct. 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Public Static Functions Documentation




### function clamp\_us 

_Clamp pulse width value to 1000..2000 us servo range._ 
```C++
static uint16_t clamp_us (
    int32_t val
) 
```





**Parameters:**


* `val` Input pulse width value. 



**Returns:**

Clamped pulse width in us. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_flight_pid.c`

