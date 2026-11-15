

# File syn\_interpolator.c



[**FileList**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_interpolator.c**](syn__interpolator_8c.md)

[Go to the source code of this file](syn__interpolator_8c_source.md)

_Coordinated multi-axis linear & circular arc trajectory interpolator implementation._ 

* `#include "../util/syn_assert.h"`
* `#include "syn_interpolator.h"`
* `#include <math.h>`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_interpolator\_eval\_at\_time**](#function-syn_interpolator_eval_at_time) ([**SYN\_Interpolator**](structSYN__Interpolator.md) \* interp, float t\_sec, [**SYN\_Vector3F**](structSYN__Vector3F.md) \* out\_pos, [**SYN\_Vector3F**](structSYN__Vector3F.md) \* out\_vel) <br>_Evaluate the interpolator at a specific time offset_ `t` _seconds._ |
|  void | [**syn\_interpolator\_init**](#function-syn_interpolator_init) ([**SYN\_Interpolator**](structSYN__Interpolator.md) \* interp) <br>_Initialize a Multi-Axis Interpolator._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_interpolator\_plan\_bezier**](#function-syn_interpolator_plan_bezier) ([**SYN\_Interpolator**](structSYN__Interpolator.md) \* interp, [**SYN\_Vector3F**](structSYN__Vector3F.md) p0, [**SYN\_Vector3F**](structSYN__Vector3F.md) p1, [**SYN\_Vector3F**](structSYN__Vector3F.md) p2, [**SYN\_Vector3F**](structSYN__Vector3F.md) p3, float feedrate, float max\_accel, float max\_jerk, float step\_res) <br>_Plan a Cubic 3D Bezier curve trajectory._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_interpolator\_plan\_circular**](#function-syn_interpolator_plan_circular) ([**SYN\_Interpolator**](structSYN__Interpolator.md) \* interp, [**SYN\_Vector3F**](structSYN__Vector3F.md) start, [**SYN\_Vector3F**](structSYN__Vector3F.md) target, [**SYN\_Vector3F**](structSYN__Vector3F.md) center\_offset, [**bool**](syn__defs_8h.md#enum-bool) is\_cw, float feedrate, float max\_accel, float max\_jerk, float step\_res) <br>_Plan a Circular 2D arc move in XY plane._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_interpolator\_plan\_circular\_plane**](#function-syn_interpolator_plan_circular_plane) ([**SYN\_Interpolator**](structSYN__Interpolator.md) \* interp, [**SYN\_Vector3F**](structSYN__Vector3F.md) start, [**SYN\_Vector3F**](structSYN__Vector3F.md) target, [**SYN\_Vector3F**](structSYN__Vector3F.md) center\_offset, uint8\_t plane, [**bool**](syn__defs_8h.md#enum-bool) is\_cw, float feedrate, float max\_accel, float max\_jerk, float step\_res) <br>_Plan a Circular or Helical arc move in a selectable right-handed plane._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_interpolator\_plan\_linear**](#function-syn_interpolator_plan_linear) ([**SYN\_Interpolator**](structSYN__Interpolator.md) \* interp, [**SYN\_Vector3F**](structSYN__Vector3F.md) start, [**SYN\_Vector3F**](structSYN__Vector3F.md) target, float feedrate, float max\_accel, float max\_jerk, float step\_res) <br>_Plan a Linear 2D/3D motion move._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_interpolator\_step**](#function-syn_interpolator_step) ([**SYN\_Interpolator**](structSYN__Interpolator.md) \* interp, [**SYN\_Vector3F**](structSYN__Vector3F.md) \* out\_pos) <br>_Advance the interpolator by one discrete step._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  int32\_t | [**interp\_clamp\_rate\_i32**](#function-interp_clamp_rate_i32) (float val) <br>_Convert a non-negative float rate to int32\_t with saturation to INT32\_MAX._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**interp\_configure\_steps**](#function-interp_configure_steps) ([**SYN\_Interpolator**](structSYN__Interpolator.md) \* interp, float total\_len, float feedrate, float max\_accel, float max\_jerk, float step\_res) <br>_Finalize discrete step count and S-curve profile for a planned trajectory._  |
|  void | [**interp\_eval\_bezier\_at**](#function-interp_eval_bezier_at) (const [**SYN\_Interpolator**](structSYN__Interpolator.md) \* interp, float ratio, float speed, [**SYN\_Vector3F**](structSYN__Vector3F.md) \* out\_pos, [**SYN\_Vector3F**](structSYN__Vector3F.md) \* out\_vel) <br>_Evaluate cubic Bezier curve position and optional velocity at normalized ratio._  |
|  [**SYN\_Vector3F**](structSYN__Vector3F.md) | [**interp\_eval\_bezier\_deriv**](#function-interp_eval_bezier_deriv) ([**SYN\_Vector3F**](structSYN__Vector3F.md) p0, [**SYN\_Vector3F**](structSYN__Vector3F.md) p1, [**SYN\_Vector3F**](structSYN__Vector3F.md) p2, [**SYN\_Vector3F**](structSYN__Vector3F.md) p3, float u) <br>_Evaluate 3D cubic Bezier tangent derivative B'(u)._  |
|  [**SYN\_Vector3F**](structSYN__Vector3F.md) | [**interp\_eval\_bezier\_pos**](#function-interp_eval_bezier_pos) ([**SYN\_Vector3F**](structSYN__Vector3F.md) p0, [**SYN\_Vector3F**](structSYN__Vector3F.md) p1, [**SYN\_Vector3F**](structSYN__Vector3F.md) p2, [**SYN\_Vector3F**](structSYN__Vector3F.md) p3, float u) <br>_Evaluate 3D cubic Bezier position B(u) at parameter u in [0, 1]._  |
|  void | [**interp\_eval\_circular\_at**](#function-interp_eval_circular_at) (const [**SYN\_Interpolator**](structSYN__Interpolator.md) \* interp, float ratio, float speed, [**SYN\_Vector3F**](structSYN__Vector3F.md) \* out\_pos, [**SYN\_Vector3F**](structSYN__Vector3F.md) \* out\_vel) <br>_Evaluate circular arc position and optional velocity at normalized ratio._  |
|  void | [**interp\_eval\_linear\_at**](#function-interp_eval_linear_at) (const [**SYN\_Interpolator**](structSYN__Interpolator.md) \* interp, float ratio, float speed, [**SYN\_Vector3F**](structSYN__Vector3F.md) \* out\_pos, [**SYN\_Vector3F**](structSYN__Vector3F.md) \* out\_vel) <br>_Evaluate linear segment position and optional velocity at normalized ratio._  |
|  [**SYN\_Vector3F**](structSYN__Vector3F.md) | [**interp\_plane\_to\_vec**](#function-interp_plane_to_vec) (float u, float v, float w, uint8\_t plane) <br>_Map right-handed plane coordinates (u, v, w) back to Cartesian (X, Y, Z)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**interp\_scalars\_valid**](#function-interp_scalars_valid) (float feedrate, float max\_accel, float max\_jerk, float step\_res) <br>_Validate common scalar motion parameters for trajectory planning._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**interp\_vec\_is\_finite**](#function-interp_vec_is_finite) ([**SYN\_Vector3F**](structSYN__Vector3F.md) v) <br>_Check that all three components of a 3D vector are finite floats._  |
|  void | [**interp\_vec\_to\_plane**](#function-interp_vec_to_plane) ([**SYN\_Vector3F**](structSYN__Vector3F.md) vec, uint8\_t plane, float \* out\_u, float \* out\_v, float \* out\_w) <br>_Map Cartesian (X, Y, Z) to right-handed plane coordinates (u, v, w)._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_INTERP\_MAX\_I32\_FLOAT**](syn__interpolator_8c.md#define-syn_interp_max_i32_float)  `2147483520.0f`<br>_Largest exact single-precision float &lt;= INT32\_MAX (2147483520.0f)._  |

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



### function syn\_interpolator\_plan\_bezier 

_Plan a Cubic 3D Bezier curve trajectory._ 
```C++
SYN_Status syn_interpolator_plan_bezier (
    SYN_Interpolator * interp,
    SYN_Vector3F p0,
    SYN_Vector3F p1,
    SYN_Vector3F p2,
    SYN_Vector3F p3,
    float feedrate,
    float max_accel,
    float max_jerk,
    float step_res
) 
```





**Parameters:**


* `interp` Pointer to interpolator struct. 
* `p0` Start point P0. 
* `p1` Control point P1. 
* `p2` Control point P2. 
* `p3` End point P3. 
* `feedrate` Desired tangential speed (units/sec). 
* `max_accel` Max acceleration (units/sec^2). 
* `max_jerk` Max jerk (units/sec^3). 
* `step_res` Spatial resolution per step. 



**Returns:**

SYN\_OK on success. 





        

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



### function syn\_interpolator\_plan\_circular\_plane 

_Plan a Circular or Helical arc move in a selectable right-handed plane._ 
```C++
SYN_Status syn_interpolator_plan_circular_plane (
    SYN_Interpolator * interp,
    SYN_Vector3F start,
    SYN_Vector3F target,
    SYN_Vector3F center_offset,
    uint8_t plane,
    bool is_cw,
    float feedrate,
    float max_accel,
    float max_jerk,
    float step_res
) 
```



Plane axis mapping follows NIST RS-274 right-handed cyclic permutations:
* `0` (`G17` XY): primary `(X, Y)` with offsets `(I, J)`, linear axis `Z`.
* `1` (`G18` ZX): primary `( Z , X)` with offsets `(K, I)`, linear axis `Y`.
* `2` (`G19` YZ): primary `(Y,  Z )` with offsets `(J, K)`, linear axis `X`.






**Parameters:**


* `interp` Pointer to interpolator struct. 
* `start` Start position (X, Y, Z). 
* `target` Target position (X, Y, Z). 
* `center_offset` Center offset relative to start (I, J, K). 
* `plane` Working plane selector (0 = XY, 1 = ZX, 2 = YZ). 
* `is_cw` true for Clockwise, false for Counter-Clockwise in the selected plane. 
* `feedrate` Desired tangential speed (units/sec). 
* `max_accel` Max acceleration (units/sec^2). 
* `max_jerk` Max jerk (units/sec^3). 
* `step_res` Spatial resolution per step (units per step). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid arguments. 





        

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
## Public Static Functions Documentation




### function interp\_clamp\_rate\_i32 

_Convert a non-negative float rate to int32\_t with saturation to INT32\_MAX._ 
```C++
static int32_t interp_clamp_rate_i32 (
    float val
) 
```





**Parameters:**


* `val` Non-negative float value (e.g., feedrate / step\_res). 



**Returns:**

Saturated int32\_t value in [0, INT32\_MAX]. 





        

<hr>



### function interp\_configure\_steps 

_Finalize discrete step count and S-curve profile for a planned trajectory._ 
```C++
static SYN_Status interp_configure_steps (
    SYN_Interpolator * interp,
    float total_len,
    float feedrate,
    float max_accel,
    float max_jerk,
    float step_res
) 
```





**Parameters:**


* `interp` Interpolator instance. 
* `total_len` Computed path length in user units. 
* `feedrate` Target feedrate in units/sec. 
* `max_accel` Maximum acceleration in units/sec^2. 
* `max_jerk` Maximum jerk in units/sec^3. 
* `step_res` Spatial resolution per step. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM if step count exceeds INT32\_MAX. 





        

<hr>



### function interp\_eval\_bezier\_at 

_Evaluate cubic Bezier curve position and optional velocity at normalized ratio._ 
```C++
static void interp_eval_bezier_at (
    const SYN_Interpolator * interp,
    float ratio,
    float speed,
    SYN_Vector3F * out_pos,
    SYN_Vector3F * out_vel
) 
```





**Parameters:**


* `interp` Interpolator instance. 
* `ratio` Normalized curve parameter u in [0, 1]. 
* `speed` Instantaneous path speed in units/sec. 
* `out_pos` Output 3D position. 
* `out_vel` Optional output 3D velocity. 




        

<hr>



### function interp\_eval\_bezier\_deriv 

_Evaluate 3D cubic Bezier tangent derivative B'(u)._ 
```C++
static SYN_Vector3F interp_eval_bezier_deriv (
    SYN_Vector3F p0,
    SYN_Vector3F p1,
    SYN_Vector3F p2,
    SYN_Vector3F p3,
    float u
) 
```





**Parameters:**


* `p0` Start control point. 
* `p1` First interior control point. 
* `p2` Second interior control point. 
* `p3` End control point. 
* `u` Curve parameter in [0, 1]. 



**Returns:**

First derivative vector dB/du at u. 





        

<hr>



### function interp\_eval\_bezier\_pos 

_Evaluate 3D cubic Bezier position B(u) at parameter u in [0, 1]._ 
```C++
static SYN_Vector3F interp_eval_bezier_pos (
    SYN_Vector3F p0,
    SYN_Vector3F p1,
    SYN_Vector3F p2,
    SYN_Vector3F p3,
    float u
) 
```





**Parameters:**


* `p0` Start control point. 
* `p1` First interior control point. 
* `p2` Second interior control point. 
* `p3` End control point. 
* `u` Curve parameter in [0, 1]. 



**Returns:**

Evaluated 3D position on the curve. 





        

<hr>



### function interp\_eval\_circular\_at 

_Evaluate circular arc position and optional velocity at normalized ratio._ 
```C++
static void interp_eval_circular_at (
    const SYN_Interpolator * interp,
    float ratio,
    float speed,
    SYN_Vector3F * out_pos,
    SYN_Vector3F * out_vel
) 
```





**Parameters:**


* `interp` Interpolator instance. 
* `ratio` Normalized path ratio in [0, 1]. 
* `speed` Instantaneous XY arc speed in units/sec. 
* `out_pos` Output 3D position. 
* `out_vel` Optional output 3D velocity. 




        

<hr>



### function interp\_eval\_linear\_at 

_Evaluate linear segment position and optional velocity at normalized ratio._ 
```C++
static void interp_eval_linear_at (
    const SYN_Interpolator * interp,
    float ratio,
    float speed,
    SYN_Vector3F * out_pos,
    SYN_Vector3F * out_vel
) 
```





**Parameters:**


* `interp` Interpolator instance. 
* `ratio` Normalized path ratio in [0, 1]. 
* `speed` Instantaneous path speed in units/sec. 
* `out_pos` Output 3D position. 
* `out_vel` Optional output 3D velocity. 




        

<hr>



### function interp\_plane\_to\_vec 

_Map right-handed plane coordinates (u, v, w) back to Cartesian (X, Y, Z)._ 
```C++
static SYN_Vector3F interp_plane_to_vec (
    float u,
    float v,
    float w,
    uint8_t plane
) 
```





**Parameters:**


* `u` Primary in-plane coordinate. 
* `v` Secondary in-plane coordinate. 
* `w` Orthogonal linear coordinate. 
* `plane` Working plane (0 = XY, 1 = ZX, 2 = YZ). 



**Returns:**

Reconstructed Cartesian vector. 





        

<hr>



### function interp\_scalars\_valid 

_Validate common scalar motion parameters for trajectory planning._ 
```C++
static bool interp_scalars_valid (
    float feedrate,
    float max_accel,
    float max_jerk,
    float step_res
) 
```





**Parameters:**


* `feedrate` Target feedrate (&gt; 0, finite). 
* `max_accel` Maximum acceleration (&gt;= 0, finite). 
* `max_jerk` Maximum jerk (&gt;= 0, finite). 
* `step_res` Spatial step resolution (&gt; 0, finite). 



**Returns:**

true if all scalar parameters are valid, false otherwise. 





        

<hr>



### function interp\_vec\_is\_finite 

_Check that all three components of a 3D vector are finite floats._ 
```C++
static bool interp_vec_is_finite (
    SYN_Vector3F v
) 
```





**Parameters:**


* `v` Input 3D vector. 



**Returns:**

true if x, y, and z are all finite, false otherwise. 





        

<hr>



### function interp\_vec\_to\_plane 

_Map Cartesian (X, Y, Z) to right-handed plane coordinates (u, v, w)._ 
```C++
static void interp_vec_to_plane (
    SYN_Vector3F vec,
    uint8_t plane,
    float * out_u,
    float * out_v,
    float * out_w
) 
```





**Parameters:**


* `vec` Input Cartesian vector. 
* `plane` Working plane (0 = XY, 1 = ZX, 2 = YZ). 
* `out_u` Output primary in-plane coordinate. 
* `out_v` Output secondary in-plane coordinate. 
* `out_w` Output orthogonal linear coordinate. 




        

<hr>
## Macro Definition Documentation





### define SYN\_INTERP\_MAX\_I32\_FLOAT 

_Largest exact single-precision float &lt;= INT32\_MAX (2147483520.0f)._ 
```C++
#define SYN_INTERP_MAX_I32_FLOAT `2147483520.0f`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_interpolator.c`

