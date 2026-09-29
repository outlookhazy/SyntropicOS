

# File syn\_scurve.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_scurve.c**](syn__scurve_8c.md)

[Go to the source code of this file](syn__scurve_8c_source.md)



* `#include "syn_assert.h"`
* `#include "syn_scurve.h"`
* `#include <string.h>`
* `#include "syn_qmath.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_scurve3d\_plan**](#function-syn_scurve3d_plan) ([**SYN\_SCurve3D**](structSYN__SCurve3D.md) \* sc3d, int32\_t start\_x, int32\_t start\_y, int32\_t start\_z, int32\_t target\_x, int32\_t target\_y, int32\_t target\_z, int32\_t v\_max, int32\_t a\_max, int32\_t j\_max) <br>_Initialize and plan a synchronized 3D vector move._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_scurve3d\_update**](#function-syn_scurve3d_update) ([**SYN\_SCurve3D**](structSYN__SCurve3D.md) \* sc3d, int32\_t \* out\_x, int32\_t \* out\_y, int32\_t \* out\_z) <br>_Step 3D synchronized S-Curve profile by one tick._  |
|  void | [**syn\_scurve\_init**](#function-syn_scurve_init) ([**SYN\_SCurve**](structSYN__SCurve.md) \* sc, int32\_t initial\_p) <br>_Initialize the S-Curve generator._  |
|  void | [**syn\_scurve\_set\_constraints**](#function-syn_scurve_set_constraints) ([**SYN\_SCurve**](structSYN__SCurve.md) \* sc, int32\_t v\_max, int32\_t a\_max, int32\_t j\_max) <br>_Configure the kinematic constraints._  |
|  void | [**syn\_scurve\_set\_target**](#function-syn_scurve_set_target) ([**SYN\_SCurve**](structSYN__SCurve.md) \* sc, int32\_t target) <br>_Set a new target position._  |
|  int32\_t | [**syn\_scurve\_update**](#function-syn_scurve_update) ([**SYN\_SCurve**](structSYN__SCurve.md) \* sc) <br>_Update the trajectory generator by one time step (tick)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**syn\_isqrt**](#function-syn_isqrt) (uint32\_t n) <br> |


























## Public Functions Documentation




### function syn\_scurve3d\_plan 

_Initialize and plan a synchronized 3D vector move._ 
```C++
void syn_scurve3d_plan (
    SYN_SCurve3D * sc3d,
    int32_t start_x,
    int32_t start_y,
    int32_t start_z,
    int32_t target_x,
    int32_t target_y,
    int32_t target_z,
    int32_t v_max,
    int32_t a_max,
    int32_t j_max
) 
```





**Parameters:**


* `sc3d` 3D profile pointer. 
* `start_x` Starting X coordinate. 
* `start_y` Starting Y coordinate. 
* `start_z` Starting Z coordinate. 
* `target_x` Target X coordinate. 
* `target_y` Target Y coordinate. 
* `target_z` Target Z coordinate. 
* `v_max` Vector max velocity. 
* `a_max` Vector max acceleration. 
* `j_max` Vector max jerk. 




        

<hr>



### function syn\_scurve3d\_update 

_Step 3D synchronized S-Curve profile by one tick._ 
```C++
bool syn_scurve3d_update (
    SYN_SCurve3D * sc3d,
    int32_t * out_x,
    int32_t * out_y,
    int32_t * out_z
) 
```





**Parameters:**


* `sc3d` 3D profile pointer. 
* `out_x` Out: Current X coordinate. 
* `out_y` Out: Current Y coordinate. 
* `out_z` Out: Current Z coordinate. 



**Returns:**

true if move active, false if completed. 





        

<hr>



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




### function syn\_isqrt 

```C++
static inline uint32_t syn_isqrt (
    uint32_t n
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_scurve.c`

