

# Struct SYN\_CiA402Drive



[**ClassList**](annotated.md) **>** [**SYN\_CiA402Drive**](structSYN__CiA402Drive.md)



_CiA 402 Drive Handle._ 

* `#include <syn_cia402.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int32\_t | [**actual\_position**](#variable-actual_position)  <br> |
|  int16\_t | [**actual\_torque**](#variable-actual_torque)  <br> |
|  int32\_t | [**actual\_velocity**](#variable-actual_velocity)  <br> |
|  [**SYN\_CiA402Config**](structSYN__CiA402Config.md) | [**cfg**](#variable-cfg)  <br> |
|  uint16\_t | [**controlword**](#variable-controlword)  <br> |
|  float | [**current\_pos**](#variable-current_pos)  <br> |
|  float | [**current\_speed**](#variable-current_speed)  <br> |
|  uint16\_t | [**error\_code**](#variable-error_code)  <br> |
|  int8\_t | [**mode\_display**](#variable-mode_display)  <br> |
|  int8\_t | [**mode\_of\_operation**](#variable-mode_of_operation)  <br> |
|  uint16\_t | [**prev\_controlword**](#variable-prev_controlword)  <br> |
|  [**SYN\_SCurve**](structSYN__SCurve.md) | [**scurve**](#variable-scurve)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**setpoint\_ack**](#variable-setpoint_ack)  <br> |
|  [**SYN\_CiA402State**](syn__cia402_8h.md#enum-syn_cia402state) | [**state**](#variable-state)  <br> |
|  uint16\_t | [**statusword**](#variable-statusword)  <br> |
|  int32\_t | [**target\_position**](#variable-target_position)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**target\_reached**](#variable-target_reached)  <br> |
|  int16\_t | [**target\_torque**](#variable-target_torque)  <br> |
|  int32\_t | [**target\_velocity**](#variable-target_velocity)  <br> |












































## Public Attributes Documentation




### variable actual\_position 

```C++
int32_t SYN_CiA402Drive::actual_position;
```



Index 0x6064 Position Actual Value 


        

<hr>



### variable actual\_torque 

```C++
int16_t SYN_CiA402Drive::actual_torque;
```



Index 0x6077 Torque Actual Value 


        

<hr>



### variable actual\_velocity 

```C++
int32_t SYN_CiA402Drive::actual_velocity;
```



Index 0x606C Velocity Actual Value 


        

<hr>



### variable cfg 

```C++
SYN_CiA402Config SYN_CiA402Drive::cfg;
```



Drive configuration parameters 


        

<hr>



### variable controlword 

```C++
uint16_t SYN_CiA402Drive::controlword;
```



Index 0x6040 Controlword 


        

<hr>



### variable current\_pos 

```C++
float SYN_CiA402Drive::current_pos;
```



Current internal continuous position 


        

<hr>



### variable current\_speed 

```C++
float SYN_CiA402Drive::current_speed;
```



Current internal continuous speed 


        

<hr>



### variable error\_code 

```C++
uint16_t SYN_CiA402Drive::error_code;
```



Index 0x603F Error Code 


        

<hr>



### variable mode\_display 

```C++
int8_t SYN_CiA402Drive::mode_display;
```



Index 0x6061 Modes of Operation Display 


        

<hr>



### variable mode\_of\_operation 

```C++
int8_t SYN_CiA402Drive::mode_of_operation;
```



Index 0x6060 Modes of Operation 


        

<hr>



### variable prev\_controlword 

```C++
uint16_t SYN_CiA402Drive::prev_controlword;
```



Previous controlword for edge detection 


        

<hr>



### variable scurve 

```C++
SYN_SCurve SYN_CiA402Drive::scurve;
```



Internal S-Curve trajectory generator for PP mode 


        

<hr>



### variable setpoint\_ack 

```C++
bool SYN_CiA402Drive::setpoint_ack;
```



Setpoint acknowledge bit indicator 


        

<hr>



### variable state 

```C++
SYN_CiA402State SYN_CiA402Drive::state;
```



Current drive state machine state 


        

<hr>



### variable statusword 

```C++
uint16_t SYN_CiA402Drive::statusword;
```



Index 0x6041 Statusword 


        

<hr>



### variable target\_position 

```C++
int32_t SYN_CiA402Drive::target_position;
```



Index 0x607A Target Position 


        

<hr>



### variable target\_reached 

```C++
bool SYN_CiA402Drive::target_reached;
```



Target reached flag 


        

<hr>



### variable target\_torque 

```C++
int16_t SYN_CiA402Drive::target_torque;
```



Index 0x6071 Target Torque 


        

<hr>



### variable target\_velocity 

```C++
int32_t SYN_CiA402Drive::target_velocity;
```



Index 0x60FF Target Velocity 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cia402.h`

