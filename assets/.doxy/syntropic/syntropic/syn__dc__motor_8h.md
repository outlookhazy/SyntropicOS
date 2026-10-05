

# File syn\_dc\_motor.h



[**FileList**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_dc\_motor.h**](syn__dc__motor_8h.md)

[Go to the source code of this file](syn__dc__motor_8h_source.md)

_DC motor controller for H-bridge drivers._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../drivers/syn_gpio.h"`
* `#include "../motor/syn_motor_output.h"`
* `#include "../port/syn_port_system.h"`
* `#include "../util/syn_ramp.h"`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_DCMotor**](structSYN__DCMotor.md) <br>_DC motor instance — pins, speed, ramp state._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_DCMotorMode**](#enum-syn_dcmotormode)  <br>_DC motor control wiring mode._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_dc\_motor\_brake**](#function-syn_dc_motor_brake) ([**SYN\_DCMotor**](structSYN__DCMotor.md) \* motor) <br>_Brake the motor (both pins high, if driver supports it)._  |
|  void | [**syn\_dc\_motor\_coast**](#function-syn_dc_motor_coast) ([**SYN\_DCMotor**](structSYN__DCMotor.md) \* motor) <br>_Stop the motor (coast — both pins low)._  |
|  void | [**syn\_dc\_motor\_init**](#function-syn_dc_motor_init) ([**SYN\_DCMotor**](structSYN__DCMotor.md) \* motor, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin\_a, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin\_b, [**SYN\_DCMotorMode**](syn__dc__motor_8h.md#enum-syn_dcmotormode) mode) <br>_Initialize DC motor controller._  |
|  [**SYN\_MotorOutput**](structSYN__MotorOutput.md) | [**syn\_dc\_motor\_output**](#function-syn_dc_motor_output) ([**SYN\_DCMotor**](structSYN__DCMotor.md) \* motor) <br>_Create a_ [_**SYN\_MotorOutput**_](structSYN__MotorOutput.md) _interface for this DC motor._ |
|  void | [**syn\_dc\_motor\_ramp\_to**](#function-syn_dc_motor_ramp_to) ([**SYN\_DCMotor**](structSYN__DCMotor.md) \* motor, int32\_t speed, uint16\_t duration) <br>_Ramp to a target speed over a duration._  |
|  void | [**syn\_dc\_motor\_set\_duty\_callback**](#function-syn_dc_motor_set_duty_callback) ([**SYN\_DCMotor**](structSYN__DCMotor.md) \* motor, void(\*)([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin), uint16\_t, void \*) cb, void \* ctx) <br>_Set the PWM duty callback._  |
|  void | [**syn\_dc\_motor\_set\_duty\_max**](#function-syn_dc_motor_set_duty_max) ([**SYN\_DCMotor**](structSYN__DCMotor.md) \* motor, int32\_t duty\_max) <br>_Set the maximum duty cycle value._  |
|  void | [**syn\_dc\_motor\_set\_speed**](#function-syn_dc_motor_set_speed) ([**SYN\_DCMotor**](structSYN__DCMotor.md) \* motor, int32\_t speed) <br>_Set motor speed immediately._  |
|  void | [**syn\_dc\_motor\_update**](#function-syn_dc_motor_update) ([**SYN\_DCMotor**](structSYN__DCMotor.md) \* motor) <br>_Update motor ramp. Call periodically._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_dc\_motor\_at\_target**](#function-syn_dc_motor_at_target) (const [**SYN\_DCMotor**](structSYN__DCMotor.md) \* motor) <br>_Is the ramp complete?_  |
|  int32\_t | [**syn\_dc\_motor\_get\_speed**](#function-syn_dc_motor_get_speed) (const [**SYN\_DCMotor**](structSYN__DCMotor.md) \* motor) <br>_Get current speed._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_DC\_MOTOR\_DUTY\_MAX\_DEFAULT**](syn__dc__motor_8h.md#define-syn_dc_motor_duty_max_default)  `1000`<br>_Default duty cycle range (0.1% resolution)._  |

## Detailed Description


Controls direction + speed via two GPIO pins (or one PWM + one dir pin). Supports soft start/stop ramps to limit inrush current and mechanical stress. Provides a [**SYN\_MotorOutput**](structSYN__MotorOutput.md) interface for use with syn\_motor\_ctrl.




**
**


```C++
static SYN_DCMotor motor;
syn_dc_motor_init(&motor, PWM_PIN, DIR_PIN, SYN_DC_MODE_PWM_DIR);
syn_dc_motor_set_speed(&motor, 750);   // 75% of duty_max (default 1000)
syn_dc_motor_set_speed(&motor, -500);  // 50% reverse

// With ramp:
syn_dc_motor_ramp_to(&motor, 1000, 500);  // ramp to full over 500ms
while (!syn_dc_motor_at_target(&motor)) {
    syn_dc_motor_update(&motor);
}

// With motor controller:
cfg.motor = syn_dc_motor_output(&motor);
```
 





    
## Public Types Documentation




### enum SYN\_DCMotorMode 

_DC motor control wiring mode._ 
```C++
enum SYN_DCMotorMode {
    SYN_DC_MODE_PWM_DIR = 0,
    SYN_DC_MODE_DUAL_PWM = 1
};
```




<hr>
## Public Functions Documentation




### function syn\_dc\_motor\_brake 

_Brake the motor (both pins high, if driver supports it)._ 
```C++
void syn_dc_motor_brake (
    SYN_DCMotor * motor
) 
```





**Parameters:**


* `motor` Motor instance. 




        

<hr>



### function syn\_dc\_motor\_coast 

_Stop the motor (coast — both pins low)._ 
```C++
void syn_dc_motor_coast (
    SYN_DCMotor * motor
) 
```





**Parameters:**


* `motor` Motor instance. 




        

<hr>



### function syn\_dc\_motor\_init 

_Initialize DC motor controller._ 
```C++
void syn_dc_motor_init (
    SYN_DCMotor * motor,
    SYN_GPIO_Pin pin_a,
    SYN_GPIO_Pin pin_b,
    SYN_DCMotorMode mode
) 
```



Sets duty\_max to SYN\_DC\_MOTOR\_DUTY\_MAX\_DEFAULT (1000).




**Parameters:**


* `motor` Motor instance. 
* `pin_a` PWM pin (or IN\_A). 
* `pin_b` Direction pin (or IN\_B). 
* `mode` Control mode. 




        

<hr>



### function syn\_dc\_motor\_output 

_Create a_ [_**SYN\_MotorOutput**_](structSYN__MotorOutput.md) _interface for this DC motor._
```C++
SYN_MotorOutput syn_dc_motor_output (
    SYN_DCMotor * motor
) 
```



Returns a motor output vtable that maps set\_output() to [**syn\_dc\_motor\_set\_speed()**](syn__dc__motor_8h.md#function-syn_dc_motor_set_speed), coast() to [**syn\_dc\_motor\_coast()**](syn__dc__motor_8h.md#function-syn_dc_motor_coast), etc. The output range maps directly to [-duty\_max, +duty\_max].




**Parameters:**


* `motor` DC motor instance (must outlive the returned output). 



**Returns:**

Motor output interface. 





        

<hr>



### function syn\_dc\_motor\_ramp\_to 

_Ramp to a target speed over a duration._ 
```C++
void syn_dc_motor_ramp_to (
    SYN_DCMotor * motor,
    int32_t speed,
    uint16_t duration
) 
```





**Parameters:**


* `motor` Motor. 
* `speed` Target speed [-duty\_max, +duty\_max]. 
* `duration` Ramp duration in milliseconds. 




        

<hr>



### function syn\_dc\_motor\_set\_duty\_callback 

_Set the PWM duty callback._ 
```C++
void syn_dc_motor_set_duty_callback (
    SYN_DCMotor * motor,
    void(*)( SYN_GPIO_Pin , uint16_t, void *) cb,
    void * ctx
) 
```



The callback is called with the pin and a duty value in [0, duty\_max] whenever the motor speed changes.




**Parameters:**


* `motor` Motor instance. 
* `cb` Duty callback. 
* `ctx` User context. 




        

<hr>



### function syn\_dc\_motor\_set\_duty\_max 

_Set the maximum duty cycle value._ 
```C++
void syn_dc_motor_set_duty_max (
    SYN_DCMotor * motor,
    int32_t duty_max
) 
```



Call after init to change from the default (1000). Resets speed to 0.




**Parameters:**


* `motor` Motor instance. 
* `duty_max` Maximum duty value (e.g., 255, 1000, 4095, 65535). 




        

<hr>



### function syn\_dc\_motor\_set\_speed 

_Set motor speed immediately._ 
```C++
void syn_dc_motor_set_speed (
    SYN_DCMotor * motor,
    int32_t speed
) 
```





**Parameters:**


* `motor` DC motor handle. 
* `speed` Speed in range [-duty\_max, +duty\_max]. Positive = forward, negative = reverse, 0 = stop. Clamped to ±duty\_max. 




        

<hr>



### function syn\_dc\_motor\_update 

_Update motor ramp. Call periodically._ 
```C++
void syn_dc_motor_update (
    SYN_DCMotor * motor
) 
```





**Parameters:**


* `motor` Motor instance. 




        

<hr>
## Public Static Functions Documentation




### function syn\_dc\_motor\_at\_target 

_Is the ramp complete?_ 
```C++
static inline bool syn_dc_motor_at_target (
    const SYN_DCMotor * motor
) 
```





**Parameters:**


* `motor` Motor instance. 



**Returns:**

true if at target speed. 





        

<hr>



### function syn\_dc\_motor\_get\_speed 

_Get current speed._ 
```C++
static inline int32_t syn_dc_motor_get_speed (
    const SYN_DCMotor * motor
) 
```





**Parameters:**


* `motor` Motor instance. 



**Returns:**

Speed in range [-duty\_max, +duty\_max]. 





        

<hr>
## Macro Definition Documentation





### define SYN\_DC\_MOTOR\_DUTY\_MAX\_DEFAULT 

_Default duty cycle range (0.1% resolution)._ 
```C++
#define SYN_DC_MOTOR_DUTY_MAX_DEFAULT `1000`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_dc_motor.h`

