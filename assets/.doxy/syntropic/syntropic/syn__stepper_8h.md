

# File syn\_stepper.h



[**FileList**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_stepper.h**](syn__stepper_8h.md)

[Go to the source code of this file](syn__stepper_8h_source.md)

_Stepper motor driver — step/direction with acceleration ramps._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../drivers/syn_gpio.h"`
* `#include "../motor/syn_motor_output.h"`
* `#include "../port/syn_port_system.h"`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Stepper**](structSYN__Stepper.md) <br>_Stepper motor controller context._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_StepperDir**](#enum-syn_stepperdir)  <br>_Stepper motor direction._  |
| enum  | [**SYN\_StepperState**](#enum-syn_stepperstate)  <br>_Stepper motor operational states for the ramp generator._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_stepper\_enable**](#function-syn_stepper_enable) (const [**SYN\_Stepper**](structSYN__Stepper.md) \* s, [**bool**](syn__defs_8h.md#enum-bool) enable) <br>_Enable or disable the motor driver._  |
|  void | [**syn\_stepper\_init**](#function-syn_stepper_init) ([**SYN\_Stepper**](structSYN__Stepper.md) \* s, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) step\_pin, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) dir\_pin) <br>_Initialize stepper motor driver._  |
|  void | [**syn\_stepper\_move**](#function-syn_stepper_move) ([**SYN\_Stepper**](structSYN__Stepper.md) \* s, int32\_t steps) <br>_Start a relative move._  |
|  void | [**syn\_stepper\_move\_to**](#function-syn_stepper_move_to) ([**SYN\_Stepper**](structSYN__Stepper.md) \* s, int32\_t position) <br>_Start a move to an absolute position._  |
|  [**SYN\_MotorOutput**](structSYN__MotorOutput.md) | [**syn\_stepper\_output**](#function-syn_stepper_output) ([**SYN\_Stepper**](structSYN__Stepper.md) \* stepper) <br>_Create a_ [_**SYN\_MotorOutput**_](structSYN__MotorOutput.md) _interface for this stepper._ |
|  void | [**syn\_stepper\_set\_enable\_pin**](#function-syn_stepper_set_enable_pin) ([**SYN\_Stepper**](structSYN__Stepper.md) \* s, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin, [**bool**](syn__defs_8h.md#enum-bool) active\_low) <br>_Set optional enable pin._  |
|  void | [**syn\_stepper\_set\_speed**](#function-syn_stepper_set_speed) ([**SYN\_Stepper**](structSYN__Stepper.md) \* s, uint32\_t max\_sps, uint32\_t accel\_sps2) <br>_Set maximum speed and acceleration._  |
|  void | [**syn\_stepper\_stop**](#function-syn_stepper_stop) ([**SYN\_Stepper**](structSYN__Stepper.md) \* s) <br>_Emergency stop — immediately halt with no deceleration._  |
|  void | [**syn\_stepper\_tick**](#function-syn_stepper_tick) ([**SYN\_Stepper**](structSYN__Stepper.md) \* s) <br>_Advance the stepper state machine by one tick._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_stepper\_is\_moving**](#function-syn_stepper_is_moving) (const [**SYN\_Stepper**](structSYN__Stepper.md) \* s) <br>_Is a move in progress?_  |
|  int32\_t | [**syn\_stepper\_position**](#function-syn_stepper_position) (const [**SYN\_Stepper**](structSYN__Stepper.md) \* s) <br>_Get current position (steps)._  |
|  void | [**syn\_stepper\_set\_position**](#function-syn_stepper_set_position) ([**SYN\_Stepper**](structSYN__Stepper.md) \* s, int32\_t pos) <br>_Set current position without moving._  |


























## Detailed Description


Generates step pulses for step/dir drivers (A4988, DRV8825, TMC2209, etc.). Supports trapezoidal acceleration profiles and non-blocking operation.




**
**


```C++
static SYN_Stepper stepper;
syn_stepper_init(&stepper, STEP_PIN, DIR_PIN);
syn_stepper_set_speed(&stepper, 1000, 500);   // max 1000 sps, accel 500 sps²
syn_stepper_move(&stepper, 2000);               // move 2000 steps forward

// Call periodically (from timer ISR or fast loop):
syn_stepper_tick(&stepper);
```
 





    
## Public Types Documentation




### enum SYN\_StepperDir 

_Stepper motor direction._ 
```C++
enum SYN_StepperDir {
    SYN_STEPPER_CW = 0,
    SYN_STEPPER_CCW = 1
};
```




<hr>



### enum SYN\_StepperState 

_Stepper motor operational states for the ramp generator._ 
```C++
enum SYN_StepperState {
    SYN_STEPPER_IDLE = 0,
    SYN_STEPPER_ACCEL = 1,
    SYN_STEPPER_CRUISE = 2,
    SYN_STEPPER_DECEL = 3
};
```




<hr>
## Public Functions Documentation




### function syn\_stepper\_enable 

_Enable or disable the motor driver._ 
```C++
void syn_stepper_enable (
    const SYN_Stepper * s,
    bool enable
) 
```





**Parameters:**


* `s` Stepper instance. 
* `enable` true to enable, false to disable. 




        

<hr>



### function syn\_stepper\_init 

_Initialize stepper motor driver._ 
```C++
void syn_stepper_init (
    SYN_Stepper * s,
    SYN_GPIO_Pin step_pin,
    SYN_GPIO_Pin dir_pin
) 
```





**Parameters:**


* `s` Stepper instance. 
* `step_pin` Step pulse GPIO. 
* `dir_pin` Direction GPIO. 




        

<hr>



### function syn\_stepper\_move 

_Start a relative move._ 
```C++
void syn_stepper_move (
    SYN_Stepper * s,
    int32_t steps
) 
```





**Parameters:**


* `s` Stepper handle. 
* `steps` Number of steps (positive = forward, negative = reverse). 




        

<hr>



### function syn\_stepper\_move\_to 

_Start a move to an absolute position._ 
```C++
void syn_stepper_move_to (
    SYN_Stepper * s,
    int32_t position
) 
```





**Parameters:**


* `s` Stepper. 
* `position` Target absolute position in steps. 




        

<hr>



### function syn\_stepper\_output 

_Create a_ [_**SYN\_MotorOutput**_](structSYN__MotorOutput.md) _interface for this stepper._
```C++
SYN_MotorOutput syn_stepper_output (
    SYN_Stepper * stepper
) 
```



The set\_output callback calls [**syn\_stepper\_tick()**](syn__stepper_8h.md#function-syn_stepper_tick), coast/brake both call [**syn\_stepper\_stop()**](syn__stepper_8h.md#function-syn_stepper_stop). The motor controller's output value is not used directly — stepper motion is driven by the stepper's own move/move\_to commands.




**Parameters:**


* `stepper` Stepper instance (must outlive the returned output). 



**Returns:**

Motor output interface. 





        

<hr>



### function syn\_stepper\_set\_enable\_pin 

_Set optional enable pin._ 
```C++
void syn_stepper_set_enable_pin (
    SYN_Stepper * s,
    SYN_GPIO_Pin pin,
    bool active_low
) 
```





**Parameters:**


* `s` Stepper instance. 
* `pin` Enable signal GPIO pin. 
* `active_low` True if enabling requires driving pin LOW. 




        

<hr>



### function syn\_stepper\_set\_speed 

_Set maximum speed and acceleration._ 
```C++
void syn_stepper_set_speed (
    SYN_Stepper * s,
    uint32_t max_sps,
    uint32_t accel_sps2
) 
```





**Parameters:**


* `s` Stepper. 
* `max_sps` Maximum speed in steps per second. 
* `accel_sps2` Acceleration in steps per second². 




        

<hr>



### function syn\_stepper\_stop 

_Emergency stop — immediately halt with no deceleration._ 
```C++
void syn_stepper_stop (
    SYN_Stepper * s
) 
```





**Parameters:**


* `s` Stepper instance. 




        

<hr>



### function syn\_stepper\_tick 

_Advance the stepper state machine by one tick._ 
```C++
void syn_stepper_tick (
    SYN_Stepper * s
) 
```



Call from a timer ISR or high-frequency loop. The tick rate should be at least 2× the maximum step rate for proper pulse generation.




**Parameters:**


* `s` Stepper instance. 




        

<hr>
## Public Static Functions Documentation




### function syn\_stepper\_is\_moving 

_Is a move in progress?_ 
```C++
static inline bool syn_stepper_is_moving (
    const SYN_Stepper * s
) 
```





**Parameters:**


* `s` Stepper. 



**Returns:**

true if moving. 





        

<hr>



### function syn\_stepper\_position 

_Get current position (steps)._ 
```C++
static inline int32_t syn_stepper_position (
    const SYN_Stepper * s
) 
```





**Parameters:**


* `s` Stepper. 



**Returns:**

Position in steps. 





        

<hr>



### function syn\_stepper\_set\_position 

_Set current position without moving._ 
```C++
static inline void syn_stepper_set_position (
    SYN_Stepper * s,
    int32_t pos
) 
```





**Parameters:**


* `s` Stepper. 
* `pos` New position. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_stepper.h`

