

# Struct SYN\_SensorFusion



[**ClassList**](annotated.md) **>** [**SYN\_SensorFusion**](structSYN__SensorFusion.md)



_Mahony 6-DOF IMU Sensor Fusion state structure._ 

* `#include <syn_sensor_fusion.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**Ki**](#variable-ki)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**Kp**](#variable-kp)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**dt**](#variable-dt)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**e\_int**](#variable-e_int)  <br> |
|  [**SYN\_Quaternion**](structSYN__Quaternion.md) | [**q**](#variable-q)  <br> |












































## Public Attributes Documentation




### variable Ki 

```C++
q16_t SYN_SensorFusion::Ki;
```



Integral error feedback gain 
 


        

<hr>



### variable Kp 

```C++
q16_t SYN_SensorFusion::Kp;
```



Proportional feedback gain 
 


        

<hr>



### variable dt 

```C++
q16_t SYN_SensorFusion::dt;
```



Sampling period in seconds (Q16.16) 


        

<hr>



### variable e\_int 

```C++
q16_t SYN_SensorFusion::e_int[3];
```



Integral error vector accumulator 
 


        

<hr>



### variable q 

```C++
SYN_Quaternion SYN_SensorFusion::q;
```



Current estimated quaternion orientation 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sensor/syn_sensor_fusion.h`

