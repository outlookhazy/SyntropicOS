

# File syn\_servo.h



[**FileList**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_servo.h**](syn__servo_8h.md)

[Go to the source code of this file](syn__servo_8h_source.md)

_Hobby servo controller — pulse-width positioning._ [More...](#detailed-description)

* `#include "../port/syn_port_system.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Servo**](structSYN__Servo.md) <br>_Hobby servo instance — pulse range, position, smooth movement._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_servo\_init**](#function-syn_servo_init) ([**SYN\_Servo**](structSYN__Servo.md) \* servo, uint16\_t pulse\_min, uint16\_t pulse\_max, uint16\_t angle\_range) <br>_Initialize a servo._  |
|  void | [**syn\_servo\_move\_to**](#function-syn_servo_move_to) ([**SYN\_Servo**](structSYN__Servo.md) \* servo, uint16\_t angle, uint16\_t duration) <br>_Start a smooth move to an angle._  |
|  void | [**syn\_servo\_set\_angle**](#function-syn_servo_set_angle) ([**SYN\_Servo**](structSYN__Servo.md) \* servo, uint16\_t angle) <br>_Set servo position by angle (immediate)._  |
|  void | [**syn\_servo\_set\_pulse**](#function-syn_servo_set_pulse) ([**SYN\_Servo**](structSYN__Servo.md) \* servo, uint16\_t us) <br>_Set servo position by raw pulse width (immediate)._  |
|  void | [**syn\_servo\_update**](#function-syn_servo_update) ([**SYN\_Servo**](structSYN__Servo.md) \* servo) <br>_Update servo position for smooth movement._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_servo\_at\_target**](#function-syn_servo_at_target) (const [**SYN\_Servo**](structSYN__Servo.md) \* servo) <br>_Is the servo at its target position?_  |
|  uint16\_t | [**syn\_servo\_get\_angle**](#function-syn_servo_get_angle) (const [**SYN\_Servo**](structSYN__Servo.md) \* servo) <br>_Get the current angle._  |
|  uint16\_t | [**syn\_servo\_get\_pulse\_us**](#function-syn_servo_get_pulse_us) (const [**SYN\_Servo**](structSYN__Servo.md) \* servo) <br>_Get the current pulse width in microseconds._  |


























## Detailed Description


Standard hobby servos expect a 50 Hz PWM signal with a pulse width of 1000–2000 µs (center = 1500 µs). This module provides angle-based and microsecond-based positioning with optional smooth movement.


Requires a hardware timer or syn\_soft\_pwm for pulse generation. The module computes pulse width; you connect it to your PWM output.




**
**


```C++
SYN_Servo servo;
syn_servo_init(&servo, 1000, 2000, 180); // 1000–2000µs, 180° range

syn_servo_set_angle(&servo, 90);   // center
uint16_t pw = syn_servo_get_pulse_us(&servo);  // → 1500µs

// Smooth move:
syn_servo_move_to(&servo, 0, 500);  // move to 0° over 500ms
while (!syn_servo_at_target(&servo)) {
    syn_servo_update(&servo);       // call periodically
    set_pwm_us(syn_servo_get_pulse_us(&servo));
}
```
 





    
## Public Functions Documentation




### function syn\_servo\_init 

_Initialize a servo._ 
```C++
void syn_servo_init (
    SYN_Servo * servo,
    uint16_t pulse_min,
    uint16_t pulse_max,
    uint16_t angle_range
) 
```





**Parameters:**


* `servo` Servo instance. 
* `pulse_min` Minimum pulse width in µs (0° position). 
* `pulse_max` Maximum pulse width in µs (max angle position). 
* `angle_range` Full angular range in degrees. 




        

<hr>



### function syn\_servo\_move\_to 

_Start a smooth move to an angle._ 
```C++
void syn_servo_move_to (
    SYN_Servo * servo,
    uint16_t angle,
    uint16_t duration
) 
```





**Parameters:**


* `servo` Servo. 
* `angle` Target angle. 
* `duration` Movement duration in milliseconds. 




        

<hr>



### function syn\_servo\_set\_angle 

_Set servo position by angle (immediate)._ 
```C++
void syn_servo_set_angle (
    SYN_Servo * servo,
    uint16_t angle
) 
```





**Parameters:**


* `servo` Servo handle. 
* `angle` Angle in degrees (0 to angle\_range). 




        

<hr>



### function syn\_servo\_set\_pulse 

_Set servo position by raw pulse width (immediate)._ 
```C++
void syn_servo_set_pulse (
    SYN_Servo * servo,
    uint16_t us
) 
```





**Parameters:**


* `servo` Servo handle. 
* `us` Pulse width in microseconds. 




        

<hr>



### function syn\_servo\_update 

_Update servo position for smooth movement._ 
```C++
void syn_servo_update (
    SYN_Servo * servo
) 
```



Call periodically (e.g., every 10–20ms) while a smooth move is active.




**Parameters:**


* `servo` Servo instance. 




        

<hr>
## Public Static Functions Documentation




### function syn\_servo\_at\_target 

_Is the servo at its target position?_ 
```C++
static inline bool syn_servo_at_target (
    const SYN_Servo * servo
) 
```





**Parameters:**


* `servo` Servo. 



**Returns:**

true if at target. 





        

<hr>



### function syn\_servo\_get\_angle 

_Get the current angle._ 
```C++
static inline uint16_t syn_servo_get_angle (
    const SYN_Servo * servo
) 
```





**Parameters:**


* `servo` Servo. 



**Returns:**

Angle in degrees. 





        

<hr>



### function syn\_servo\_get\_pulse\_us 

_Get the current pulse width in microseconds._ 
```C++
static inline uint16_t syn_servo_get_pulse_us (
    const SYN_Servo * servo
) 
```





**Parameters:**


* `servo` Servo. 



**Returns:**

Pulse width in µs. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_servo.h`

