

# Struct SYN\_CIA402\_Config



[**ClassList**](annotated.md) **>** [**SYN\_CIA402\_Config**](structSYN__CIA402__Config.md)



_CiA 402 axis configuration parameters._ 

* `#include <syn_cia402.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**following\_err\_window**](#variable-following_err_window)  <br> |
|  uint32\_t | [**max\_profile\_velocity**](#variable-max_profile_velocity)  <br> |
|  uint16\_t | [**max\_torque**](#variable-max_torque)  <br> |
|  uint32\_t | [**position\_window**](#variable-position_window)  <br> |
|  uint32\_t | [**profile\_acceleration**](#variable-profile_acceleration)  <br> |
|  uint32\_t | [**profile\_deceleration**](#variable-profile_deceleration)  <br> |
|  uint32\_t | [**quick\_stop\_deceleration**](#variable-quick_stop_deceleration)  <br> |
|  int32\_t | [**sw\_limit\_max**](#variable-sw_limit_max)  <br> |
|  int32\_t | [**sw\_limit\_min**](#variable-sw_limit_min)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**sw\_limits\_enabled**](#variable-sw_limits_enabled)  <br> |
|  uint32\_t | [**torque\_slope**](#variable-torque_slope)  <br> |












































## Public Attributes Documentation




### variable following\_err\_window 

```C++
uint32_t SYN_CIA402_Config::following_err_window;
```



Following error threshold (0=disabled) 


        

<hr>



### variable max\_profile\_velocity 

```C++
uint32_t SYN_CIA402_Config::max_profile_velocity;
```



Max / default profile velocity (counts/s) 


        

<hr>



### variable max\_torque 

```C++
uint16_t SYN_CIA402_Config::max_torque;
```



Max torque limit (0.1% rated) 


        

<hr>



### variable position\_window 

```C++
uint32_t SYN_CIA402_Config::position_window;
```



Target reached deadband (counts) 


        

<hr>



### variable profile\_acceleration 

```C++
uint32_t SYN_CIA402_Config::profile_acceleration;
```



Default profile acceleration (counts/s^2) 


        

<hr>



### variable profile\_deceleration 

```C++
uint32_t SYN_CIA402_Config::profile_deceleration;
```



Default profile deceleration (counts/s^2) 


        

<hr>



### variable quick\_stop\_deceleration 

```C++
uint32_t SYN_CIA402_Config::quick_stop_deceleration;
```



Quick stop deceleration rate (counts/s^2) 


        

<hr>



### variable sw\_limit\_max 

```C++
int32_t SYN_CIA402_Config::sw_limit_max;
```



Software position maximum limit (counts) 


        

<hr>



### variable sw\_limit\_min 

```C++
int32_t SYN_CIA402_Config::sw_limit_min;
```



Software position minimum limit (counts) 


        

<hr>



### variable sw\_limits\_enabled 

```C++
bool SYN_CIA402_Config::sw_limits_enabled;
```



Software limit check active 


        

<hr>



### variable torque\_slope 

```C++
uint32_t SYN_CIA402_Config::torque_slope;
```



Torque ramp rate (0.1%/s) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cia402.h`

