

# File syn\_servo.c



[**FileList**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_servo.c**](syn__servo_8c.md)

[Go to the source code of this file](syn__servo_8c_source.md)

_Servo controller implementation._ 

* `#include "syn_servo.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































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
|  uint16\_t | [**angle\_to\_us**](#function-angle_to_us) (const [**SYN\_Servo**](structSYN__Servo.md) \* s, uint16\_t angle) <br>_Convert angle to pulse width in microseconds._  |
|  uint16\_t | [**clamp\_us**](#function-clamp_us) (const [**SYN\_Servo**](structSYN__Servo.md) \* s, uint16\_t us) <br>_Clamp pulse width to min/max range._  |


























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




### function angle\_to\_us 

_Convert angle to pulse width in microseconds._ 
```C++
static uint16_t angle_to_us (
    const SYN_Servo * s,
    uint16_t angle
) 
```





**Parameters:**


* `s` Servo instance. 
* `angle` Angle (0 to angle\_range). 



**Returns:**

Pulse width in µs. 





        

<hr>



### function clamp\_us 

_Clamp pulse width to min/max range._ 
```C++
static uint16_t clamp_us (
    const SYN_Servo * s,
    uint16_t us
) 
```





**Parameters:**


* `s` Servo instance. 
* `us` Pulse width in µs. 



**Returns:**

Clamped pulse width. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_servo.c`

