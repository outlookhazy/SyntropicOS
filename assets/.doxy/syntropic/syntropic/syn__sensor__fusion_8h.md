

# File syn\_sensor\_fusion.h



[**FileList**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_sensor\_fusion.h**](syn__sensor__fusion_8h.md)

[Go to the source code of this file](syn__sensor__fusion_8h_source.md)

_6-DOF IMU Sensor Fusion & AHRS Complementary / Mahony Filter (Q16.16 fixed-point)._ 

* `#include "../common/syn_defs.h"`
* `#include "../util/syn_qmath.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_EulerAngles**](structSYN__EulerAngles.md) <br>_3D Euler attitude angles (in radians, Q16.16)._  |
| struct | [**SYN\_Quaternion**](structSYN__Quaternion.md) <br>_3D Quaternion representation (w + xi + yj + zk)._  |
| struct | [**SYN\_SensorFusion**](structSYN__SensorFusion.md) <br>_Mahony 6-DOF IMU Sensor Fusion state structure._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sensor\_fusion\_get\_euler**](#function-syn_sensor_fusion_get_euler) (const [**SYN\_SensorFusion**](structSYN__SensorFusion.md) \* f, [**SYN\_EulerAngles**](structSYN__EulerAngles.md) \* euler) <br>_Retrieve current Euler attitude angles (Roll, Pitch, Yaw)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sensor\_fusion\_get\_quaternion**](#function-syn_sensor_fusion_get_quaternion) (const [**SYN\_SensorFusion**](structSYN__SensorFusion.md) \* f, [**SYN\_Quaternion**](structSYN__Quaternion.md) \* q) <br>_Retrieve current quaternion orientation estimate._  |
|  void | [**syn\_sensor\_fusion\_init**](#function-syn_sensor_fusion_init) ([**SYN\_SensorFusion**](structSYN__SensorFusion.md) \* f, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) Kp, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) Ki, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) dt) <br>_Initialize the IMU sensor fusion filter._  |
|  void | [**syn\_sensor\_fusion\_reset**](#function-syn_sensor_fusion_reset) ([**SYN\_SensorFusion**](structSYN__SensorFusion.md) \* f) <br>_Reset filter state to initial identity orientation (q = [1, 0, 0, 0])._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sensor\_fusion\_update**](#function-syn_sensor_fusion_update) ([**SYN\_SensorFusion**](structSYN__SensorFusion.md) \* f, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) gx, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) gy, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) gz, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) ax, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) ay, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) az) <br>_Update orientation estimate using 6-axis IMU sample._  |




























## Public Functions Documentation




### function syn\_sensor\_fusion\_get\_euler 

_Retrieve current Euler attitude angles (Roll, Pitch, Yaw)._ 
```C++
SYN_Status syn_sensor_fusion_get_euler (
    const SYN_SensorFusion * f,
    SYN_EulerAngles * euler
) 
```





**Parameters:**


* `f` Filter instance. 
* `euler` Output Euler angles pointer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL. 





        

<hr>



### function syn\_sensor\_fusion\_get\_quaternion 

_Retrieve current quaternion orientation estimate._ 
```C++
SYN_Status syn_sensor_fusion_get_quaternion (
    const SYN_SensorFusion * f,
    SYN_Quaternion * q
) 
```





**Parameters:**


* `f` Filter instance. 
* `q` Output quaternion pointer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL. 





        

<hr>



### function syn\_sensor\_fusion\_init 

_Initialize the IMU sensor fusion filter._ 
```C++
void syn_sensor_fusion_init (
    SYN_SensorFusion * f,
    q16_t Kp,
    q16_t Ki,
    q16_t dt
) 
```





**Parameters:**


* `f` Filter instance. 
* `Kp` Proportional error gain (e.g., Q16\_FROM\_FLOAT(2.0)). 
* `Ki` Integral error gain (e.g., Q16\_FROM\_FLOAT(0.005)). 
* `dt` Sampling interval in seconds (e.g., Q16\_FROM\_FLOAT(0.01) for 100 Hz). 




        

<hr>



### function syn\_sensor\_fusion\_reset 

_Reset filter state to initial identity orientation (q = [1, 0, 0, 0])._ 
```C++
void syn_sensor_fusion_reset (
    SYN_SensorFusion * f
) 
```





**Parameters:**


* `f` Filter instance. 




        

<hr>



### function syn\_sensor\_fusion\_update 

_Update orientation estimate using 6-axis IMU sample._ 
```C++
SYN_Status syn_sensor_fusion_update (
    SYN_SensorFusion * f,
    q16_t gx,
    q16_t gy,
    q16_t gz,
    q16_t ax,
    q16_t ay,
    q16_t az
) 
```





**Parameters:**


* `f` Filter instance. 
* `gx` Gyroscope X rate in rad/s (Q16.16). 
* `gy` Gyroscope Y rate in rad/s (Q16.16). 
* `gz` Gyroscope Z rate in rad/s (Q16.16). 
* `ax` Accelerometer X acceleration in g or m/s² (Q16.16). 
* `ay` Accelerometer Y acceleration in g or m/s² (Q16.16). 
* `az` Accelerometer Z acceleration in g or m/s² (Q16.16). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sensor/syn_sensor_fusion.h`

