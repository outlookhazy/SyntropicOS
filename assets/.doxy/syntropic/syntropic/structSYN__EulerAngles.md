

# Struct SYN\_EulerAngles



[**ClassList**](annotated.md) **>** [**SYN\_EulerAngles**](structSYN__EulerAngles.md)



_3D Euler attitude angles (in radians, Q16.16)._ 

* `#include <syn_sensor_fusion.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**pitch\_rad**](#variable-pitch_rad)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**roll\_rad**](#variable-roll_rad)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**yaw\_rad**](#variable-yaw_rad)  <br> |












































## Public Attributes Documentation




### variable pitch\_rad 

```C++
q16_t SYN_EulerAngles::pitch_rad;
```



Pitch angle around Y axis (-pi/2 to +pi/2) 


        

<hr>



### variable roll\_rad 

```C++
q16_t SYN_EulerAngles::roll_rad;
```



Roll angle around X axis (-pi to +pi) 
 


        

<hr>



### variable yaw\_rad 

```C++
q16_t SYN_EulerAngles::yaw_rad;
```



Yaw angle around Z axis (-pi to +pi) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sensor/syn_sensor_fusion.h`

