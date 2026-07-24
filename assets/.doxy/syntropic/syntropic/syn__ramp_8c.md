

# File syn\_ramp.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_ramp.c**](syn__ramp_8c.md)

[Go to the source code of this file](syn__ramp_8c_source.md)

_Ramp / motion profile generator implementation._ 

* `#include "syn_ramp.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































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
|  int32\_t | [**update\_linear**](#function-update_linear) ([**SYN\_Ramp**](structSYN__Ramp.md) \* ramp) <br>_Linear ramp step toward target._  |
|  int32\_t | [**update\_trapezoid**](#function-update_trapezoid) ([**SYN\_Ramp**](structSYN__Ramp.md) \* ramp) <br>_Trapezoid ramp step toward target._  |


























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




### function update\_linear 

_Linear ramp step toward target._ 
```C++
static int32_t update_linear (
    SYN_Ramp * ramp
) 
```





**Parameters:**


* `ramp` Ramp instance. 



**Returns:**

Current ramp value. 





        

<hr>



### function update\_trapezoid 

_Trapezoid ramp step toward target._ 
```C++
static int32_t update_trapezoid (
    SYN_Ramp * ramp
) 
```



Supports fixed-point velocity/accel when frac\_bits &gt; 0. Velocity and accel are in Q`frac_bits` format. Position is integer; a fractional accumulator carries sub-unit remainder.




**Parameters:**


* `ramp` Ramp instance. 



**Returns:**

Current ramp value (integer position). 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_ramp.c`

