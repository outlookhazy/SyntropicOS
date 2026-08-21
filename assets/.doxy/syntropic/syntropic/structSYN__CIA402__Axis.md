

# Struct SYN\_CIA402\_Axis



[**ClassList**](annotated.md) **>** [**SYN\_CIA402\_Axis**](structSYN__CIA402__Axis.md)



_CiA 402 Axis Instance._ 

* `#include <syn_cia402.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int32\_t | [**actual\_position**](#variable-actual_position)  <br> |
|  int16\_t | [**actual\_torque**](#variable-actual_torque)  <br> |
|  int32\_t | [**actual\_velocity**](#variable-actual_velocity)  <br> |
|  uint16\_t | [**controlword**](#variable-controlword)  <br> |
|  int64\_t | [**current\_pos\_fxp**](#variable-current_pos_fxp)  <br> |
|  float | [**current\_speed**](#variable-current_speed)  <br> |
|  int32\_t | [**current\_torque\_fxp**](#variable-current_torque_fxp)  <br> |
|  int64\_t | [**current\_vel\_fxp**](#variable-current_vel_fxp)  <br> |
|  uint16\_t | [**error\_code**](#variable-error_code)  <br> |
|  uint32\_t | [**following\_err\_win**](#variable-following_err_win)  <br> |
|  uint32\_t | [**homing\_accel**](#variable-homing_accel)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**homing\_attained**](#variable-homing_attained)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**homing\_error**](#variable-homing_error)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**homing\_in\_progress**](#variable-homing_in_progress)  <br> |
|  int8\_t | [**homing\_method**](#variable-homing_method)  <br> |
|  uint32\_t | [**homing\_speed\_fast**](#variable-homing_speed_fast)  <br> |
|  uint32\_t | [**homing\_speed\_slow**](#variable-homing_speed_slow)  <br> |
|  uint16\_t | [**max\_torque**](#variable-max_torque)  <br> |
|  int8\_t | [**mode\_of\_operation**](#variable-mode_of_operation)  <br> |
|  int8\_t | [**mode\_of\_operation\_disp**](#variable-mode_of_operation_disp)  <br> |
|  int32\_t | [**position\_demand**](#variable-position_demand)  <br> |
|  uint32\_t | [**position\_window**](#variable-position_window)  <br> |
|  uint16\_t | [**prev\_controlword**](#variable-prev_controlword)  <br> |
|  uint32\_t | [**profile\_acc**](#variable-profile_acc)  <br> |
|  uint32\_t | [**profile\_dec**](#variable-profile_dec)  <br> |
|  uint32\_t | [**profile\_vel**](#variable-profile_vel)  <br> |
|  uint32\_t | [**quick\_stop\_dec**](#variable-quick_stop_dec)  <br> |
|  int16\_t | [**quick\_stop\_option**](#variable-quick_stop_option)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**setpoint\_ack**](#variable-setpoint_ack)  <br> |
|  [**SYN\_CIA402\_State**](syn__cia402_8h.md#enum-syn_cia402_state) | [**state**](#variable-state)  <br> |
|  uint16\_t | [**statusword**](#variable-statusword)  <br> |
|  int32\_t | [**sw\_limit\_max**](#variable-sw_limit_max)  <br> |
|  int32\_t | [**sw\_limit\_min**](#variable-sw_limit_min)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**sw\_limits\_enabled**](#variable-sw_limits_enabled)  <br> |
|  int32\_t | [**target\_position**](#variable-target_position)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**target\_reached**](#variable-target_reached)  <br> |
|  int16\_t | [**target\_torque**](#variable-target_torque)  <br> |
|  int32\_t | [**target\_velocity**](#variable-target_velocity)  <br> |
|  uint32\_t | [**torque\_slope**](#variable-torque_slope)  <br> |












































## Public Attributes Documentation




### variable actual\_position 

```C++
int32_t SYN_CIA402_Axis::actual_position;
```



0x6064 Position Actual Value 


        

<hr>



### variable actual\_torque 

```C++
int16_t SYN_CIA402_Axis::actual_torque;
```



0x6077 Torque Actual Value 


        

<hr>



### variable actual\_velocity 

```C++
int32_t SYN_CIA402_Axis::actual_velocity;
```



0x606C Velocity Actual Value 


        

<hr>



### variable controlword 

```C++
uint16_t SYN_CIA402_Axis::controlword;
```



Active Controlword (0x6040) 


        

<hr>



### variable current\_pos\_fxp 

```C++
int64_t SYN_CIA402_Axis::current_pos_fxp;
```



Internal high-precision position (Q32.32) 


        

<hr>



### variable current\_speed 

```C++
float SYN_CIA402_Axis::current_speed;
```



Current internal floating speed for tests 


        

<hr>



### variable current\_torque\_fxp 

```C++
int32_t SYN_CIA402_Axis::current_torque_fxp;
```



Internal high-precision torque (Q16.16) 


        

<hr>



### variable current\_vel\_fxp 

```C++
int64_t SYN_CIA402_Axis::current_vel_fxp;
```



Internal high-precision velocity (Q32.32) 


        

<hr>



### variable error\_code 

```C++
uint16_t SYN_CIA402_Axis::error_code;
```



Last triggered error code 


        

<hr>



### variable following\_err\_win 

```C++
uint32_t SYN_CIA402_Axis::following_err_win;
```



0x6065 Following Error Window 


        

<hr>



### variable homing\_accel 

```C++
uint32_t SYN_CIA402_Axis::homing_accel;
```



0x609B Homing Acceleration 


        

<hr>



### variable homing\_attained 

```C++
bool SYN_CIA402_Axis::homing_attained;
```



Homing completed successfully 


        

<hr>



### variable homing\_error 

```C++
bool SYN_CIA402_Axis::homing_error;
```



Homing fault occurred 


        

<hr>



### variable homing\_in\_progress 

```C++
bool SYN_CIA402_Axis::homing_in_progress;
```



Active homing sequence 


        

<hr>



### variable homing\_method 

```C++
int8_t SYN_CIA402_Axis::homing_method;
```



0x6098 Homing Method 


        

<hr>



### variable homing\_speed\_fast 

```C++
uint32_t SYN_CIA402_Axis::homing_speed_fast;
```



0x6099:1 Homing Speed Fast 


        

<hr>



### variable homing\_speed\_slow 

```C++
uint32_t SYN_CIA402_Axis::homing_speed_slow;
```



0x6099:2 Homing Speed Slow 


        

<hr>



### variable max\_torque 

```C++
uint16_t SYN_CIA402_Axis::max_torque;
```



0x6072 Max Torque 


        

<hr>



### variable mode\_of\_operation 

```C++
int8_t SYN_CIA402_Axis::mode_of_operation;
```



Mode of operation requested (0x6060) 


        

<hr>



### variable mode\_of\_operation\_disp 

```C++
int8_t SYN_CIA402_Axis::mode_of_operation_disp;
```



Mode of operation display (0x6061) 


        

<hr>



### variable position\_demand 

```C++
int32_t SYN_CIA402_Axis::position_demand;
```



0x6062 Position Demand Value 


        

<hr>



### variable position\_window 

```C++
uint32_t SYN_CIA402_Axis::position_window;
```



0x6067 Position Window 


        

<hr>



### variable prev\_controlword 

```C++
uint16_t SYN_CIA402_Axis::prev_controlword;
```



Previous Controlword for edge detection 


        

<hr>



### variable profile\_acc 

```C++
uint32_t SYN_CIA402_Axis::profile_acc;
```



0x6083 Profile Acceleration 


        

<hr>



### variable profile\_dec 

```C++
uint32_t SYN_CIA402_Axis::profile_dec;
```



0x6084 Profile Deceleration 


        

<hr>



### variable profile\_vel 

```C++
uint32_t SYN_CIA402_Axis::profile_vel;
```



0x6081 Profile Velocity 


        

<hr>



### variable quick\_stop\_dec 

```C++
uint32_t SYN_CIA402_Axis::quick_stop_dec;
```



0x6085 Quick Stop Deceleration 


        

<hr>



### variable quick\_stop\_option 

```C++
int16_t SYN_CIA402_Axis::quick_stop_option;
```



Quick stop option code (0x605A) 


        

<hr>



### variable setpoint\_ack 

```C++
bool SYN_CIA402_Axis::setpoint_ack;
```



Setpoint acknowledge handshake flag 


        

<hr>



### variable state 

```C++
SYN_CIA402_State SYN_CIA402_Axis::state;
```



Current FSM state 


        

<hr>



### variable statusword 

```C++
uint16_t SYN_CIA402_Axis::statusword;
```



Active Statusword (0x6041) 


        

<hr>



### variable sw\_limit\_max 

```C++
int32_t SYN_CIA402_Axis::sw_limit_max;
```



0x607E Software Limit Max 


        

<hr>



### variable sw\_limit\_min 

```C++
int32_t SYN_CIA402_Axis::sw_limit_min;
```



0x607D Software Limit Min 


        

<hr>



### variable sw\_limits\_enabled 

```C++
bool SYN_CIA402_Axis::sw_limits_enabled;
```



Software limit check active 


        

<hr>



### variable target\_position 

```C++
int32_t SYN_CIA402_Axis::target_position;
```



0x607A Target Position 


        

<hr>



### variable target\_reached 

```C++
bool SYN_CIA402_Axis::target_reached;
```



Target reached flag 


        

<hr>



### variable target\_torque 

```C++
int16_t SYN_CIA402_Axis::target_torque;
```



0x6071 Target Torque 


        

<hr>



### variable target\_velocity 

```C++
int32_t SYN_CIA402_Axis::target_velocity;
```



0x60FF Target Velocity 


        

<hr>



### variable torque\_slope 

```C++
uint32_t SYN_CIA402_Axis::torque_slope;
```



0x6087 Torque Slope 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cia402.h`

