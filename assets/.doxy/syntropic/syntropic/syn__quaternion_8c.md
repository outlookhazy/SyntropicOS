

# File syn\_quaternion.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_quaternion.c**](syn__quaternion_8c.md)

[Go to the source code of this file](syn__quaternion_8c_source.md)

_Fixed-point Q16.16 3D Quaternion operations._ 

* `#include "syn_quaternion.h"`
* `#include "syn_assert.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_quat\_conjugate**](#function-syn_quat_conjugate) (const [**SYN\_Quaternion**](structSYN__Quaternion.md) \* q, [**SYN\_Quaternion**](structSYN__Quaternion.md) \* out) <br>_Calculate quaternion conjugate q\* = (w, -x, -y, -z)._  |
|  void | [**syn\_quat\_from\_euler**](#function-syn_quat_from_euler) ([**SYN\_Quaternion**](structSYN__Quaternion.md) \* q, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) roll, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) pitch, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) yaw) <br>_Create orientation quaternion from Euler angles (roll, pitch, yaw) in Q16 radians (Z-Y-X sequence)._  |
|  void | [**syn\_quat\_identity**](#function-syn_quat_identity) ([**SYN\_Quaternion**](structSYN__Quaternion.md) \* q) <br>_Initialize identity quaternion q = (1, 0, 0, 0)._  |
|  void | [**syn\_quat\_init**](#function-syn_quat_init) ([**SYN\_Quaternion**](structSYN__Quaternion.md) \* q, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) w, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) y, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) z) <br>_Initialize quaternion with given components._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_quat\_inverse**](#function-syn_quat_inverse) (const [**SYN\_Quaternion**](structSYN__Quaternion.md) \* q, [**SYN\_Quaternion**](structSYN__Quaternion.md) \* out) <br>_Calculate quaternion inverse q^-1 = q\* / \|q\|^2._  |
|  void | [**syn\_quat\_mul**](#function-syn_quat_mul) (const [**SYN\_Quaternion**](structSYN__Quaternion.md) \* q1, const [**SYN\_Quaternion**](structSYN__Quaternion.md) \* q2, [**SYN\_Quaternion**](structSYN__Quaternion.md) \* out) <br>_Quaternion multiplication q\_out = q1 \* q2 (Hamilton product)._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**syn\_quat\_norm**](#function-syn_quat_norm) (const [**SYN\_Quaternion**](structSYN__Quaternion.md) \* q) <br>_Calculate quaternion norm (magnitude)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_quat\_normalize**](#function-syn_quat_normalize) ([**SYN\_Quaternion**](structSYN__Quaternion.md) \* q) <br>_Normalize quaternion to unit length._  |
|  void | [**syn\_quat\_rotate\_vec3**](#function-syn_quat_rotate_vec3) (const [**SYN\_Quaternion**](structSYN__Quaternion.md) \* q, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* v, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out) <br>_Rotate a 3D vector v by unit quaternion q: v\_out = q \* v \* q\*._  |
|  void | [**syn\_quat\_slerp**](#function-syn_quat_slerp) (const [**SYN\_Quaternion**](structSYN__Quaternion.md) \* q1, const [**SYN\_Quaternion**](structSYN__Quaternion.md) \* q2, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) t, [**SYN\_Quaternion**](structSYN__Quaternion.md) \* out) <br>_Spherical linear interpolation (SLERP) between q1 and q2 by factor t (0.0 to 1.0 in Q16)._  |
|  void | [**syn\_quat\_to\_euler**](#function-syn_quat_to_euler) (const [**SYN\_Quaternion**](structSYN__Quaternion.md) \* q, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* roll, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* pitch, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* yaw) <br>_Extract Euler angles (roll, pitch, yaw) in Q16 radians from unit quaternion._  |
|  void | [**syn\_quat\_to\_mat3x3**](#function-syn_quat_to_mat3x3) (const [**SYN\_Quaternion**](structSYN__Quaternion.md) \* q, [**SYN\_Matrix**](structSYN__Matrix.md) \* out) <br>_Convert unit quaternion to 3x3 rotation matrix._  |




























## Public Functions Documentation




### function syn\_quat\_conjugate 

_Calculate quaternion conjugate q\* = (w, -x, -y, -z)._ 
```C++
void syn_quat_conjugate (
    const SYN_Quaternion * q,
    SYN_Quaternion * out
) 
```





**Parameters:**


* `q` Pointer to input quaternion. 
* `out` Destination conjugate quaternion. 




        

<hr>



### function syn\_quat\_from\_euler 

_Create orientation quaternion from Euler angles (roll, pitch, yaw) in Q16 radians (Z-Y-X sequence)._ 
```C++
void syn_quat_from_euler (
    SYN_Quaternion * q,
    q16_t roll,
    q16_t pitch,
    q16_t yaw
) 
```





**Parameters:**


* `q` Destination quaternion. 
* `roll` Roll angle in Q16 radians. 
* `pitch` Pitch angle in Q16 radians. 
* `yaw` Yaw angle in Q16 radians. 




        

<hr>



### function syn\_quat\_identity 

_Initialize identity quaternion q = (1, 0, 0, 0)._ 
```C++
void syn_quat_identity (
    SYN_Quaternion * q
) 
```





**Parameters:**


* `q` Pointer to quaternion instance. 




        

<hr>



### function syn\_quat\_init 

_Initialize quaternion with given components._ 
```C++
void syn_quat_init (
    SYN_Quaternion * q,
    q16_t w,
    q16_t x,
    q16_t y,
    q16_t z
) 
```





**Parameters:**


* `q` Pointer to quaternion instance. 
* `w` Scalar (real) component. 
* `x` Vector i component. 
* `y` Vector j component. 
* `z` Vector k component. 




        

<hr>



### function syn\_quat\_inverse 

_Calculate quaternion inverse q^-1 = q\* / \|q\|^2._ 
```C++
SYN_Status syn_quat_inverse (
    const SYN_Quaternion * q,
    SYN_Quaternion * out
) 
```





**Parameters:**


* `q` Pointer to input quaternion. 
* `out` Destination inverse quaternion. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if zero magnitude. 





        

<hr>



### function syn\_quat\_mul 

_Quaternion multiplication q\_out = q1 \* q2 (Hamilton product)._ 
```C++
void syn_quat_mul (
    const SYN_Quaternion * q1,
    const SYN_Quaternion * q2,
    SYN_Quaternion * out
) 
```





**Parameters:**


* `q1` First quaternion operand. 
* `q2` Second quaternion operand. 
* `out` Output product quaternion. May alias q1 or q2. 




        

<hr>



### function syn\_quat\_norm 

_Calculate quaternion norm (magnitude)._ 
```C++
q16_t syn_quat_norm (
    const SYN_Quaternion * q
) 
```





**Parameters:**


* `q` Pointer to input quaternion. 



**Returns:**

Magnitude \|q\| in Q16.16 format. 





        

<hr>



### function syn\_quat\_normalize 

_Normalize quaternion to unit length._ 
```C++
SYN_Status syn_quat_normalize (
    SYN_Quaternion * q
) 
```





**Parameters:**


* `q` Pointer to quaternion to normalize. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if zero magnitude. 





        

<hr>



### function syn\_quat\_rotate\_vec3 

_Rotate a 3D vector v by unit quaternion q: v\_out = q \* v \* q\*._ 
```C++
void syn_quat_rotate_vec3 (
    const SYN_Quaternion * q,
    const q16_t * v,
    q16_t * out
) 
```





**Parameters:**


* `q` Unit quaternion rotation. 
* `v` Input 3D vector (3 elements). 
* `out` Output rotated 3D vector (3 elements). May alias v. 




        

<hr>



### function syn\_quat\_slerp 

_Spherical linear interpolation (SLERP) between q1 and q2 by factor t (0.0 to 1.0 in Q16)._ 
```C++
void syn_quat_slerp (
    const SYN_Quaternion * q1,
    const SYN_Quaternion * q2,
    q16_t t,
    SYN_Quaternion * out
) 
```





**Parameters:**


* `q1` Start unit quaternion (t = 0). 
* `q2` End unit quaternion (t = 1). 
* `t` Interpolation parameter [0, Q16\_ONE]. 
* `out` Output interpolated unit quaternion. 




        

<hr>



### function syn\_quat\_to\_euler 

_Extract Euler angles (roll, pitch, yaw) in Q16 radians from unit quaternion._ 
```C++
void syn_quat_to_euler (
    const SYN_Quaternion * q,
    q16_t * roll,
    q16_t * pitch,
    q16_t * yaw
) 
```





**Parameters:**


* `q` Input unit quaternion. 
* `roll` Output pointer for roll angle in Q16 radians. 
* `pitch` Output pointer for pitch angle in Q16 radians. 
* `yaw` Output pointer for yaw angle in Q16 radians. 




        

<hr>



### function syn\_quat\_to\_mat3x3 

_Convert unit quaternion to 3x3 rotation matrix._ 
```C++
void syn_quat_to_mat3x3 (
    const SYN_Quaternion * q,
    SYN_Matrix * out
) 
```





**Parameters:**


* `q` Unit quaternion. 
* `out` Output 3x3 rotation matrix. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_quaternion.c`

