

# Struct SYN\_CiA402Config



[**ClassList**](annotated.md) **>** [**SYN\_CiA402Config**](structSYN__CiA402Config.md)



_Configuration parameters for CiA 402 Drive._ 

* `#include <syn_cia402.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int32\_t | [**max\_profile\_velocity**](#variable-max_profile_velocity)  <br> |
|  int16\_t | [**max\_torque**](#variable-max_torque)  <br> |
|  int32\_t | [**profile\_acceleration**](#variable-profile_acceleration)  <br> |
|  int32\_t | [**profile\_deceleration**](#variable-profile_deceleration)  <br> |
|  int32\_t | [**quick\_stop\_deceleration**](#variable-quick_stop_deceleration)  <br> |












































## Public Attributes Documentation




### variable max\_profile\_velocity 

```C++
int32_t SYN_CiA402Config::max_profile_velocity;
```



Max profile velocity in units/s (0x607F) 


        

<hr>



### variable max\_torque 

```C++
int16_t SYN_CiA402Config::max_torque;
```



Maximum allowable torque/current setpoint (0x6072) 


        

<hr>



### variable profile\_acceleration 

```C++
int32_t SYN_CiA402Config::profile_acceleration;
```



Profile acceleration in units/s^2 (0x6083) 


        

<hr>



### variable profile\_deceleration 

```C++
int32_t SYN_CiA402Config::profile_deceleration;
```



Profile deceleration in units/s^2 (0x6084) 


        

<hr>



### variable quick\_stop\_deceleration 

```C++
int32_t SYN_CiA402Config::quick_stop_deceleration;
```



Quick stop deceleration in units/s^2 (0x6085) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cia402.h`

