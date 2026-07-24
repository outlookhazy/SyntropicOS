

# File syn\_interpolator.c



[**FileList**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_interpolator.c**](syn__interpolator_8c.md)

[Go to the source code of this file](syn__interpolator_8c_source.md)

_Coordinated multi-axis linear & circular arc trajectory interpolator implementation._ 

* `#include "syn_interpolator.h"`
* `#include "../util/syn_assert.h"`
* `#include <math.h>`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_interpolator\_eval\_at\_time**](#function-syn_interpolator_eval_at_time) ([**SYN\_Interpolator**](structSYN__Interpolator.md) \* interp, float t\_sec, [**SYN\_Vector3F**](structSYN__Vector3F.md) \* out\_pos, [**SYN\_Vector3F**](structSYN__Vector3F.md) \* out\_vel) <br>_Evaluate the interpolator at a specific time offset_ `t` _seconds._ |
|  void | [**syn\_interpolator\_init**](#function-syn_interpolator_init) ([**SYN\_Interpolator**](structSYN__Interpolator.md) \* interp) <br>_Initialize a Multi-Axis Interpolator._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_interpolator\_plan\_circular**](#function-syn_interpolator_plan_circular) ([**SYN\_Interpolator**](structSYN__Interpolator.md) \* interp, [**SYN\_Vector3F**](structSYN__Vector3F.md) start, [**SYN\_Vector3F**](structSYN__Vector3F.md) target, [**SYN\_Vector3F**](structSYN__Vector3F.md) center\_offset, [**bool**](syn__defs_8h.md#enum-bool) is\_cw, float feedrate, float max\_accel, float max\_jerk, float step\_res) <br>_Plan a Circular 2D arc move in XY plane._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_interpolator\_plan\_linear**](#function-syn_interpolator_plan_linear) ([**SYN\_Interpolator**](structSYN__Interpolator.md) \* interp, [**SYN\_Vector3F**](structSYN__Vector3F.md) start, [**SYN\_Vector3F**](structSYN__Vector3F.md) target, float feedrate, float max\_accel, float max\_jerk, float step\_res) <br>_Plan a Linear 2D/3D motion move._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_interpolator\_step**](#function-syn_interpolator_step) ([**SYN\_Interpolator**](structSYN__Interpolator.md) \* interp, [**SYN\_Vector3F**](structSYN__Vector3F.md) \* out\_pos) <br>_Advance the interpolator by one discrete step._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**M\_PI**](syn__interpolator_8c.md#define-m_pi)  `3.14159265358979323846`<br> |

## Public Functions Documentation




### function syn\_interpolator\_eval\_at\_time 

_Evaluate the interpolator at a specific time offset_ `t` _seconds._
```C++
bool syn_interpolator_eval_at_time (
    SYN_Interpolator * interp,
    float t_sec,
    SYN_Vector3F * out_pos,
    SYN_Vector3F * out_vel
) 
```





**Parameters:**


* `interp` Pointer to interpolator struct. 
* `t_sec` Time offset from start of move in seconds. 
* `out_pos` Out: Position vector at time t. 
* `out_vel` Out: Velocity vector at time t (optional, can be NULL). 



**Returns:**

true if t &lt;= move total time. 





        

<hr>



### function syn\_interpolator\_init 

_Initialize a Multi-Axis Interpolator._ 
```C++
void syn_interpolator_init (
    SYN_Interpolator * interp
) 
```





**Parameters:**


* `interp` Pointer to interpolator struct. 




        

<hr>



### function syn\_interpolator\_plan\_circular 

_Plan a Circular 2D arc move in XY plane._ 
```C++
SYN_Status syn_interpolator_plan_circular (
    SYN_Interpolator * interp,
    SYN_Vector3F start,
    SYN_Vector3F target,
    SYN_Vector3F center_offset,
    bool is_cw,
    float feedrate,
    float max_accel,
    float max_jerk,
    float step_res
) 
```





**Parameters:**


* `interp` Pointer to interpolator struct. 
* `start` Start position (X, Y). 
* `target` Target position (X, Y). 
* `center_offset` Center offset relative to start (I = center.x - start.x, J = center.y - start.y). 
* `is_cw` true for Clockwise, false for Counter-Clockwise. 
* `feedrate` Desired tangential speed (units/sec). 
* `max_accel` Max acceleration (units/sec^2). 
* `max_jerk` Max jerk (units/sec^3). 
* `step_res` Spatial resolution per step (units per step). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_interpolator\_plan\_linear 

_Plan a Linear 2D/3D motion move._ 
```C++
SYN_Status syn_interpolator_plan_linear (
    SYN_Interpolator * interp,
    SYN_Vector3F start,
    SYN_Vector3F target,
    float feedrate,
    float max_accel,
    float max_jerk,
    float step_res
) 
```





**Parameters:**


* `interp` Pointer to interpolator struct. 
* `start` Start position (X, Y, Z). 
* `target` Target position (X, Y, Z). 
* `feedrate` Desired vector velocity (units/sec). 
* `max_accel` Max acceleration (units/sec^2). 
* `max_jerk` Max jerk (units/sec^3). 
* `step_res` Spatial resolution per step (units per step, e.g. 0.001mm). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_interpolator\_step 

_Advance the interpolator by one discrete step._ 
```C++
bool syn_interpolator_step (
    SYN_Interpolator * interp,
    SYN_Vector3F * out_pos
) 
```





**Parameters:**


* `interp` Pointer to interpolator struct. 
* `out_pos` Out: Calculated position after step. 



**Returns:**

true if move active and advanced, false if move complete. 





        

<hr>
## Macro Definition Documentation





### define M\_PI 

```C++
#define M_PI `3.14159265358979323846`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_interpolator.c`

