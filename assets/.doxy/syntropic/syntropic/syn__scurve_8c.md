

# File syn\_scurve.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_scurve.c**](syn__scurve_8c.md)

[Go to the source code of this file](syn__scurve_8c_source.md)



* `#include "syn_scurve.h"`
* `#include "syn_assert.h"`
* `#include <string.h>`
* `#include "syn_qmath.h"`





































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
|  uint32\_t | [**syn\_isqrt**](#function-syn_isqrt) (uint32\_t n) <br> |


























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




### function syn\_isqrt 

```C++
static inline uint32_t syn_isqrt (
    uint32_t n
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_scurve.c`

