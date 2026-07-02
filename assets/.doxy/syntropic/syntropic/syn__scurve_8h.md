

# File syn\_scurve.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_scurve.h**](syn__scurve_8h.md)

[Go to the source code of this file](syn__scurve_8h_source.md)

_Jerk-limited S-curve trajectory generator._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_SCurve**](structSYN__SCurve.md) <br>_S-curve trajectory generator state._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_scurve\_init**](#function-syn_scurve_init) ([**SYN\_SCurve**](structSYN__SCurve.md) \* sc, int32\_t initial\_p) <br>_Initialize the S-Curve generator._  |
|  void | [**syn\_scurve\_set\_constraints**](#function-syn_scurve_set_constraints) ([**SYN\_SCurve**](structSYN__SCurve.md) \* sc, int32\_t v\_max, int32\_t a\_max, int32\_t j\_max) <br>_Configure the kinematic constraints._  |
|  void | [**syn\_scurve\_set\_target**](#function-syn_scurve_set_target) ([**SYN\_SCurve**](structSYN__SCurve.md) \* sc, int32\_t target) <br>_Set a new target position._  |
|  int32\_t | [**syn\_scurve\_update**](#function-syn_scurve_update) ([**SYN\_SCurve**](structSYN__SCurve.md) \* sc) <br>_Update the trajectory generator by one time step (tick)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  int32\_t | [**syn\_scurve\_acceleration**](#function-syn_scurve_acceleration) (const [**SYN\_SCurve**](structSYN__SCurve.md) \* sc) <br>_Get current acceleration._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_scurve\_done**](#function-syn_scurve_done) (const [**SYN\_SCurve**](structSYN__SCurve.md) \* sc) <br>_Check if trajectory is complete._  |
|  int32\_t | [**syn\_scurve\_position**](#function-syn_scurve_position) (const [**SYN\_SCurve**](structSYN__SCurve.md) \* sc) <br>_Get current position._  |
|  int32\_t | [**syn\_scurve\_velocity**](#function-syn_scurve_velocity) (const [**SYN\_SCurve**](structSYN__SCurve.md) \* sc) <br>_Get current velocity._  |


























## Detailed Description


Generates smooth motion profiles by bounding velocity, acceleration, and jerk. Ideal for stepper motors and servo positioning. 


    
## Public Functions Documentation




### function syn\_scurve\_init 

_Initialize the S-Curve generator._ 
```C++
void syn_scurve_init (
    SYN_SCurve * sc,
    int32_t initial_p
) 
```





**Parameters:**


* `sc` Pointer to generator. 
* `initial_p` Initial position. 




        

<hr>



### function syn\_scurve\_set\_constraints 

_Configure the kinematic constraints._ 
```C++
void syn_scurve_set_constraints (
    SYN_SCurve * sc,
    int32_t v_max,
    int32_t a_max,
    int32_t j_max
) 
```





**Parameters:**


* `sc` Pointer to generator. 
* `v_max` Maximum velocity (units/tick). 
* `a_max` Maximum acceleration (units/tick^2). 
* `j_max` Maximum jerk (units/tick^3). 




        

<hr>



### function syn\_scurve\_set\_target 

_Set a new target position._ 
```C++
void syn_scurve_set_target (
    SYN_SCurve * sc,
    int32_t target
) 
```





**Parameters:**


* `sc` Pointer to generator. 
* `target` Target position. 




        

<hr>



### function syn\_scurve\_update 

_Update the trajectory generator by one time step (tick)._ 
```C++
int32_t syn_scurve_update (
    SYN_SCurve * sc
) 
```





**Parameters:**


* `sc` Pointer to generator. 



**Returns:**

Current position. 





        

<hr>
## Public Static Functions Documentation




### function syn\_scurve\_acceleration 

_Get current acceleration._ 
```C++
static inline int32_t syn_scurve_acceleration (
    const SYN_SCurve * sc
) 
```





**Parameters:**


* `sc` S-curve instance. 



**Returns:**

Current acceleration value. 





        

<hr>



### function syn\_scurve\_done 

_Check if trajectory is complete._ 
```C++
static inline bool syn_scurve_done (
    const SYN_SCurve * sc
) 
```





**Parameters:**


* `sc` S-curve instance. 



**Returns:**

true if target reached. 





        

<hr>



### function syn\_scurve\_position 

_Get current position._ 
```C++
static inline int32_t syn_scurve_position (
    const SYN_SCurve * sc
) 
```





**Parameters:**


* `sc` S-curve instance. 



**Returns:**

Current position value. 





        

<hr>



### function syn\_scurve\_velocity 

_Get current velocity._ 
```C++
static inline int32_t syn_scurve_velocity (
    const SYN_SCurve * sc
) 
```





**Parameters:**


* `sc` S-curve instance. 



**Returns:**

Current velocity value. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_scurve.h`

