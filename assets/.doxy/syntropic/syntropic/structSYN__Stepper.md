

# Struct SYN\_Stepper



[**ClassList**](annotated.md) **>** [**SYN\_Stepper**](structSYN__Stepper.md)



_Stepper motor controller context._ 

* `#include <syn_stepper.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**accel**](#variable-accel)  <br> |
|  int32\_t | [**decel\_start**](#variable-decel_start)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dir\_invert**](#variable-dir_invert)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**dir\_pin**](#variable-dir_pin)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**enable\_invert**](#variable-enable_invert)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**enable\_pin**](#variable-enable_pin)  <br> |
|  uint32\_t | [**last\_step\_tick**](#variable-last_step_tick)  <br> |
|  uint32\_t | [**max\_speed**](#variable-max_speed)  <br> |
|  int32\_t | [**position**](#variable-position)  <br> |
|  uint32\_t | [**speed**](#variable-speed)  <br> |
|  uint8\_t | [**state**](#variable-state)  <br> |
|  uint32\_t | [**step\_interval**](#variable-step_interval)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**step\_pin**](#variable-step_pin)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**step\_state**](#variable-step_state)  <br> |
|  int32\_t | [**steps\_to\_go**](#variable-steps_to_go)  <br> |
|  int32\_t | [**target**](#variable-target)  <br> |












































## Public Attributes Documentation




### variable accel 

```C++
uint32_t SYN_Stepper::accel;
```



Acceleration in steps/sec² 
 


        

<hr>



### variable decel\_start 

```C++
int32_t SYN_Stepper::decel_start;
```



Step count at which to start decelerating 
 


        

<hr>



### variable dir\_invert 

```C++
bool SYN_Stepper::dir_invert;
```



Invert direction logic 
 


        

<hr>



### variable dir\_pin 

```C++
SYN_GPIO_Pin SYN_Stepper::dir_pin;
```



Direction control GPIO pin 


        

<hr>



### variable enable\_invert 

```C++
bool SYN_Stepper::enable_invert;
```



Enable active-low 
 


        

<hr>



### variable enable\_pin 

```C++
SYN_GPIO_Pin SYN_Stepper::enable_pin;
```



Optional enable pin (set to -1 if unused) 


        

<hr>



### variable last\_step\_tick 

```C++
uint32_t SYN_Stepper::last_step_tick;
```



Tick (µs) of last step 
 


        

<hr>



### variable max\_speed 

```C++
uint32_t SYN_Stepper::max_speed;
```



Maximum speed in steps/sec 
 


        

<hr>



### variable position 

```C++
int32_t SYN_Stepper::position;
```



Current position (steps, signed) 
 


        

<hr>



### variable speed 

```C++
uint32_t SYN_Stepper::speed;
```



Current speed (steps/sec, fixed-point Q16) 


        

<hr>



### variable state 

```C++
uint8_t SYN_Stepper::state;
```



SYN\_StepperState 
 


        

<hr>



### variable step\_interval 

```C++
uint32_t SYN_Stepper::step_interval;
```



Current step interval (µs) 
 


        

<hr>



### variable step\_pin 

```C++
SYN_GPIO_Pin SYN_Stepper::step_pin;
```



Step signal GPIO pin 


        

<hr>



### variable step\_state 

```C++
bool SYN_Stepper::step_state;
```



Current step pin level (for pulse gen) 
 


        

<hr>



### variable steps\_to\_go 

```C++
int32_t SYN_Stepper::steps_to_go;
```



Remaining steps in current move 
 


        

<hr>



### variable target 

```C++
int32_t SYN_Stepper::target;
```



Target position 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_stepper.h`

