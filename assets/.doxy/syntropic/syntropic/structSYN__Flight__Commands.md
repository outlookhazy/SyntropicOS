

# Struct SYN\_Flight\_Commands



[**ClassList**](annotated.md) **>** [**SYN\_Flight\_Commands**](structSYN__Flight__Commands.md)



[More...](#detailed-description)

* `#include <syn_flight_pid.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**angle\_mode**](#variable-angle_mode)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**pitch\_target**](#variable-pitch_target)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**roll\_target**](#variable-roll_target)  <br> |
|  uint16\_t | [**throttle\_us**](#variable-throttle_us)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**yaw\_target**](#variable-yaw_target)  <br> |












































## Detailed Description


Pilot Command Inputs in Q16.16. 


    
## Public Attributes Documentation




### variable angle\_mode 

```C++
bool SYN_Flight_Commands::angle_mode;
```



True = Angle self-leveling mode, False = Acro rate mode. 


        

<hr>



### variable pitch\_target 

```C++
q16_t SYN_Flight_Commands::pitch_target;
```



Desired Pitch angle or rate (Q16.16). 


        

<hr>



### variable roll\_target 

```C++
q16_t SYN_Flight_Commands::roll_target;
```



Desired Roll angle or rate (Q16.16). 


        

<hr>



### variable throttle\_us 

```C++
uint16_t SYN_Flight_Commands::throttle_us;
```



Base throttle in microseconds (1000..2000 us). 


        

<hr>



### variable yaw\_target 

```C++
q16_t SYN_Flight_Commands::yaw_target;
```



Desired Yaw rate (Q16.16). 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_flight_pid.h`

