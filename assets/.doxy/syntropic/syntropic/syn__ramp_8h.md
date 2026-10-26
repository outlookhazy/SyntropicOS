

# File syn\_ramp.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_ramp.h**](syn__ramp_8h.md)

[Go to the source code of this file](syn__ramp_8h_source.md)

_Ramp / motion profile generator._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Ramp**](structSYN__Ramp.md) <br>_Ramp generator instance — current value, target, rate._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_RampMode**](#enum-syn_rampmode)  <br>_Ramp profile mode._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_ramp\_init**](#function-syn_ramp_init) ([**SYN\_Ramp**](structSYN__Ramp.md) \* ramp, int32\_t initial) <br>_Initialize ramp generator._  |
|  void | [**syn\_ramp\_jump**](#function-syn_ramp_jump) ([**SYN\_Ramp**](structSYN__Ramp.md) \* ramp, int32\_t value) <br>_Jump immediately to a value (no ramp)._  |
|  void | [**syn\_ramp\_set\_target**](#function-syn_ramp_set_target) ([**SYN\_Ramp**](structSYN__Ramp.md) \* ramp, int32\_t target, int32\_t rate) <br>_Set new target with linear ramp._  |
|  void | [**syn\_ramp\_set\_target\_trapezoid**](#function-syn_ramp_set_target_trapezoid) ([**SYN\_Ramp**](structSYN__Ramp.md) \* ramp, int32\_t target, int32\_t max\_rate, int32\_t accel) <br>_Set new target with trapezoidal ramp (acceleration/deceleration)._  |
|  void | [**syn\_ramp\_set\_target\_trapezoid\_fp**](#function-syn_ramp_set_target_trapezoid_fp) ([**SYN\_Ramp**](structSYN__Ramp.md) \* ramp, int32\_t target, int32\_t max\_rate, int32\_t accel, uint8\_t frac\_bits) <br>_Set new target with fixed-point trapezoidal ramp._  |
|  int32\_t | [**syn\_ramp\_update**](#function-syn_ramp_update) ([**SYN\_Ramp**](structSYN__Ramp.md) \* ramp) <br>_Update the ramp — call once per tick._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ramp\_done**](#function-syn_ramp_done) (const [**SYN\_Ramp**](structSYN__Ramp.md) \* ramp) <br>_Check if ramp has reached its target._  |
|  int32\_t | [**syn\_ramp\_value**](#function-syn_ramp_value) (const [**SYN\_Ramp**](structSYN__Ramp.md) \* ramp) <br>_Get current value without updating._  |


























## Detailed Description


Generates smooth setpoint transitions for any controlled variable (valves, lights, temperature, motor speed, etc.).


Supports two modes:
* LINEAR: constant slew rate
* TRAPEZOID: acceleration/deceleration (trapezoidal velocity profile)




The trapezoid mode supports optional fixed-point velocity/acceleration via `syn_ramp_set_target_trapezoid_fp()`. This allows sub-unit-per-tick resolution — critical when converting from per-second to per-tick at high update rates where integer truncation would lose precision.


Usage: 
```C++
static SYN_Ramp ramp;
syn_ramp_init(&ramp, 0);
syn_ramp_set_target(&ramp, 1000, 10);  // ramp to 1000 at 10/tick

// In your update loop:
int32_t output = syn_ramp_update(&ramp);
```
 


    
## Public Types Documentation




### enum SYN\_RampMode 

_Ramp profile mode._ 
```C++
enum SYN_RampMode {
    SYN_RAMP_LINEAR = 0,
    SYN_RAMP_TRAPEZOID = 1
};
```




<hr>
## Public Functions Documentation




### function syn\_ramp\_init 

_Initialize ramp generator._ 
```C++
void syn_ramp_init (
    SYN_Ramp * ramp,
    int32_t initial
) 
```





**Parameters:**


* `ramp` Ramp instance. 
* `initial` Starting value. 




        

<hr>



### function syn\_ramp\_jump 

_Jump immediately to a value (no ramp)._ 
```C++
void syn_ramp_jump (
    SYN_Ramp * ramp,
    int32_t value
) 
```





**Parameters:**


* `ramp` Ramp instance. 
* `value` Value to jump to. 




        

<hr>



### function syn\_ramp\_set\_target 

_Set new target with linear ramp._ 
```C++
void syn_ramp_set_target (
    SYN_Ramp * ramp,
    int32_t target,
    int32_t rate
) 
```





**Parameters:**


* `ramp` Ramp instance. 
* `target` Desired final value. 
* `rate` Max change per update call (always positive). 




        

<hr>



### function syn\_ramp\_set\_target\_trapezoid 

_Set new target with trapezoidal ramp (acceleration/deceleration)._ 
```C++
void syn_ramp_set_target_trapezoid (
    SYN_Ramp * ramp,
    int32_t target,
    int32_t max_rate,
    int32_t accel
) 
```





**Parameters:**


* `ramp` Ramp instance. 
* `target` Desired final value. 
* `max_rate` Max velocity (units/tick). 
* `accel` Acceleration (units/tick²). Controls smoothness. 




        

<hr>



### function syn\_ramp\_set\_target\_trapezoid\_fp 

_Set new target with fixed-point trapezoidal ramp._ 
```C++
void syn_ramp_set_target_trapezoid_fp (
    SYN_Ramp * ramp,
    int32_t target,
    int32_t max_rate,
    int32_t accel,
    uint8_t frac_bits
) 
```



Like [**syn\_ramp\_set\_target\_trapezoid()**](syn__ramp_8h.md#function-syn_ramp_set_target_trapezoid), but velocity and acceleration are in Q`frac_bits` fixed-point. This allows sub-unit-per-tick resolution for smooth motion at high update rates.




**Parameters:**


* `ramp` Ramp instance. 
* `target` Desired final value (integer, not fixed-point). 
* `max_rate` Max velocity in Q`frac_bits` (e.g., Q8). 
* `accel` Acceleration in Q`frac_bits`. 
* `frac_bits` Number of fractional bits (e.g., 8 for Q8). 




        

<hr>



### function syn\_ramp\_update 

_Update the ramp — call once per tick._ 
```C++
int32_t syn_ramp_update (
    SYN_Ramp * ramp
) 
```





**Parameters:**


* `ramp` Ramp instance. 



**Returns:**

Current output value. 





        

<hr>
## Public Static Functions Documentation




### function syn\_ramp\_done 

_Check if ramp has reached its target._ 
```C++
static inline bool syn_ramp_done (
    const SYN_Ramp * ramp
) 
```





**Parameters:**


* `ramp` Ramp instance. 



**Returns:**

true if done. 





        

<hr>



### function syn\_ramp\_value 

_Get current value without updating._ 
```C++
static inline int32_t syn_ramp_value (
    const SYN_Ramp * ramp
) 
```





**Parameters:**


* `ramp` Ramp instance. 



**Returns:**

Current value. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_ramp.h`

