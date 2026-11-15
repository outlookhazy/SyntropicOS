

# File syn\_actuator.h



[**FileList**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_actuator.h**](syn__actuator_8h.md)

[Go to the source code of this file](syn__actuator_8h_source.md)

_Linear actuator with potentiometer feedback._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../motor/syn_dc_motor.h"`
* `#include "../motor/syn_motor_ctrl.h"`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Actuator**](structSYN__Actuator.md) <br>_Linear actuator instance — motor controller + stroke mapping._  |
| struct | [**SYN\_Actuator\_Config**](structSYN__Actuator__Config.md) <br>_Actuator configuration._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_actuator\_clear\_stall**](#function-syn_actuator_clear_stall) ([**SYN\_Actuator**](structSYN__Actuator.md) \* act) <br>_Clear stall condition and re-enable._  |
|  void | [**syn\_actuator\_init**](#function-syn_actuator_init) ([**SYN\_Actuator**](structSYN__Actuator.md) \* act, const [**SYN\_Actuator\_Config**](structSYN__Actuator__Config.md) \* cfg) <br>_Initialize linear actuator._  |
|  void | [**syn\_actuator\_set\_position**](#function-syn_actuator_set_position) ([**SYN\_Actuator**](structSYN__Actuator.md) \* act, int16\_t pct\_x10) <br>_Set desired position._  |
|  void | [**syn\_actuator\_stop**](#function-syn_actuator_stop) ([**SYN\_Actuator**](structSYN__Actuator.md) \* act) <br>_Stop the actuator immediately._  |
|  int16\_t | [**syn\_actuator\_update**](#function-syn_actuator_update) ([**SYN\_Actuator**](structSYN__Actuator.md) \* act) <br>_Update — read pot, run PID, drive motor._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_actuator\_at\_target**](#function-syn_actuator_at_target) (const [**SYN\_Actuator**](structSYN__Actuator.md) \* act) <br>_Check if actuator has reached its target._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_actuator\_is\_stalled**](#function-syn_actuator_is_stalled) (const [**SYN\_Actuator**](structSYN__Actuator.md) \* act) <br>_Check if stalled._  |
|  int16\_t | [**syn\_actuator\_position**](#function-syn_actuator_position) (const [**SYN\_Actuator**](structSYN__Actuator.md) \* act) <br>_Get current position (0-1000, i.e. 0.0-100.0%)._  |


























## Detailed Description


Wraps motor\_ctrl + sensor into a turnkey position-controlled linear actuator. The potentiometer reading is mapped to a percentage position (0.0% – 100.0%, stored as 0–1000).


Usage: 
```C++
static SYN_Actuator act;
static SYN_DCMotor motor;
static SYN_ADC pot_adc;

SYN_Actuator_Config cfg = {
    .dc_motor    = &motor,
    .read_pos    = my_pot_read,     // returns ADC value
    .read_ctx    = &pot_adc,
    .stroke_min  = 100,             // ADC at fully retracted
    .stroke_max  = 3900,            // ADC at fully extended
    .update_hz   = 50,
};
syn_actuator_init(&act, &cfg);
syn_actuator_set_position(&act, 500);  // move to 50.0%

// In your loop:
syn_actuator_update(&act);
```
 


    
## Public Functions Documentation




### function syn\_actuator\_clear\_stall 

_Clear stall condition and re-enable._ 
```C++
void syn_actuator_clear_stall (
    SYN_Actuator * act
) 
```





**Parameters:**


* `act` Actuator. 




        

<hr>



### function syn\_actuator\_init 

_Initialize linear actuator._ 
```C++
void syn_actuator_init (
    SYN_Actuator * act,
    const SYN_Actuator_Config * cfg
) 
```





**Parameters:**


* `act` Actuator instance. 
* `cfg` Configuration. 




        

<hr>



### function syn\_actuator\_set\_position 

_Set desired position._ 
```C++
void syn_actuator_set_position (
    SYN_Actuator * act,
    int16_t pct_x10
) 
```





**Parameters:**


* `act` Actuator instance. 
* `pct_x10` Position in 0.1% units (0 = retracted, 1000 = extended). 




        

<hr>



### function syn\_actuator\_stop 

_Stop the actuator immediately._ 
```C++
void syn_actuator_stop (
    SYN_Actuator * act
) 
```





**Parameters:**


* `act` Actuator instance. 




        

<hr>



### function syn\_actuator\_update 

_Update — read pot, run PID, drive motor._ 
```C++
int16_t syn_actuator_update (
    SYN_Actuator * act
) 
```



Call from your scheduler task at the configured update\_hz.




**Parameters:**


* `act` Actuator instance. 



**Returns:**

Current position (0-1000). 





        

<hr>
## Public Static Functions Documentation




### function syn\_actuator\_at\_target 

_Check if actuator has reached its target._ 
```C++
static inline bool syn_actuator_at_target (
    const SYN_Actuator * act
) 
```





**Parameters:**


* `act` Actuator. 



**Returns:**

true if within ±0.5% of target. 





        

<hr>



### function syn\_actuator\_is\_stalled 

_Check if stalled._ 
```C++
static inline bool syn_actuator_is_stalled (
    const SYN_Actuator * act
) 
```





**Parameters:**


* `act` Actuator. 



**Returns:**

true if motor is stalled. 





        

<hr>



### function syn\_actuator\_position 

_Get current position (0-1000, i.e. 0.0-100.0%)._ 
```C++
static inline int16_t syn_actuator_position (
    const SYN_Actuator * act
) 
```





**Parameters:**


* `act` Actuator. 



**Returns:**

Position in 0.1% units. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_actuator.h`

