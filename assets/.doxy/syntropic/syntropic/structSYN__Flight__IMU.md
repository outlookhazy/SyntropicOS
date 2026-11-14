

# Struct SYN\_Flight\_IMU



[**ClassList**](annotated.md) **>** [**SYN\_Flight\_IMU**](structSYN__Flight__IMU.md)



[More...](#detailed-description)

* `#include <syn_flight_pid.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**angle\_pitch**](#variable-angle_pitch)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**angle\_roll**](#variable-angle_roll)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**gyro\_pitch**](#variable-gyro_pitch)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**gyro\_roll**](#variable-gyro_roll)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**gyro\_yaw**](#variable-gyro_yaw)  <br> |












































## Detailed Description


IMU Sensor Measurement Inputs in Q16.16. 


    
## Public Attributes Documentation




### variable angle\_pitch 

```C++
q16_t SYN_Flight_IMU::angle_pitch;
```



Measured Pitch angle in deg (Q16.16). 


        

<hr>



### variable angle\_roll 

```C++
q16_t SYN_Flight_IMU::angle_roll;
```



Measured Roll angle in deg (Q16.16). 


        

<hr>



### variable gyro\_pitch 

```C++
q16_t SYN_Flight_IMU::gyro_pitch;
```



Measured Pitch rate in deg/s (Q16.16). 


        

<hr>



### variable gyro\_roll 

```C++
q16_t SYN_Flight_IMU::gyro_roll;
```



Measured Roll rate in deg/s (Q16.16). 


        

<hr>



### variable gyro\_yaw 

```C++
q16_t SYN_Flight_IMU::gyro_yaw;
```



Measured Yaw rate in deg/s (Q16.16). 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_flight_pid.h`

