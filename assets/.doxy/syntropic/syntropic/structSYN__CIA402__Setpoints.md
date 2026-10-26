

# Struct SYN\_CIA402\_Setpoints



[**ClassList**](annotated.md) **>** [**SYN\_CIA402\_Setpoints**](structSYN__CIA402__Setpoints.md)



_CiA 402 motion controller setpoint outputs for motor loop / FOC._ 

* `#include <syn_cia402.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**brake\_released**](#variable-brake_released)  <br> |
|  [**SYN\_CIA402\_Mode**](syn__cia402_8h.md#enum-syn_cia402_mode) | [**mode**](#variable-mode)  <br> |
|  int32\_t | [**position\_cmd**](#variable-position_cmd)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**power\_enabled**](#variable-power_enabled)  <br> |
|  int16\_t | [**torque\_cmd**](#variable-torque_cmd)  <br> |
|  int32\_t | [**velocity\_cmd**](#variable-velocity_cmd)  <br> |












































## Public Attributes Documentation




### variable brake\_released 

```C++
bool SYN_CIA402_Setpoints::brake_released;
```



True when holding brake should be released 


        

<hr>



### variable mode 

```C++
SYN_CIA402_Mode SYN_CIA402_Setpoints::mode;
```



Active control mode 


        

<hr>



### variable position\_cmd 

```C++
int32_t SYN_CIA402_Setpoints::position_cmd;
```



Demanded position setpoint (counts) 


        

<hr>



### variable power\_enabled 

```C++
bool SYN_CIA402_Setpoints::power_enabled;
```



True when operation enabled / bridge active 


        

<hr>



### variable torque\_cmd 

```C++
int16_t SYN_CIA402_Setpoints::torque_cmd;
```



Demanded torque setpoint (0.1% rated) 


        

<hr>



### variable velocity\_cmd 

```C++
int32_t SYN_CIA402_Setpoints::velocity_cmd;
```



Demanded velocity setpoint (counts/s) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cia402.h`

