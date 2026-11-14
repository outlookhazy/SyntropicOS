

# File syn\_transform.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_transform.h**](syn__transform_8h.md)

[Go to the source code of this file](syn__transform_8h_source.md)

_Fixed-point Q16.16 Coordinate System Transformations (Polar, Spherical, Cartesian)._ 

* `#include "../common/syn_defs.h"`
* `#include "syn_qmath.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_cart2pol**](#function-syn_cart2pol) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) y, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* r, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* theta) <br>_Convert 2D Cartesian coordinates (x, y) to Polar coordinates (r, theta)._  |
|  void | [**syn\_cart2sph**](#function-syn_cart2sph) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) y, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) z, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* r, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* theta, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* phi) <br>_Convert 3D Cartesian coordinates (x, y, z) to Spherical coordinates (r, theta, phi)._  |
|  void | [**syn\_pol2cart**](#function-syn_pol2cart) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) r, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) theta, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* x, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* y) <br>_Convert 2D Polar coordinates (r, theta) to Cartesian coordinates (x, y)._  |
|  void | [**syn\_sph2cart**](#function-syn_sph2cart) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) r, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) theta, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) phi, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* x, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* y, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* z) <br>_Convert 3D Spherical coordinates (r, theta, phi) to Cartesian coordinates (x, y, z)._  |




























## Public Functions Documentation




### function syn\_cart2pol 

_Convert 2D Cartesian coordinates (x, y) to Polar coordinates (r, theta)._ 
```C++
void syn_cart2pol (
    q16_t x,
    q16_t y,
    q16_t * r,
    q16_t * theta
) 
```





**Parameters:**


* `x` X coordinate in Q16. 
* `y` Y coordinate in Q16. 
* `r` Output radius r in Q16. 
* `theta` Output angle theta in Q16 radians [-pi, pi]. 




        

<hr>



### function syn\_cart2sph 

_Convert 3D Cartesian coordinates (x, y, z) to Spherical coordinates (r, theta, phi)._ 
```C++
void syn_cart2sph (
    q16_t x,
    q16_t y,
    q16_t z,
    q16_t * r,
    q16_t * theta,
    q16_t * phi
) 
```





**Parameters:**


* `x` X coordinate in Q16. 
* `y` Y coordinate in Q16. 
* `z` Z coordinate in Q16. 
* `r` Output radius r in Q16. 
* `theta` Output azimuth angle theta in Q16 radians [-pi, pi]. 
* `phi` Output polar angle (inclination) phi in Q16 radians [0, pi]. 




        

<hr>



### function syn\_pol2cart 

_Convert 2D Polar coordinates (r, theta) to Cartesian coordinates (x, y)._ 
```C++
void syn_pol2cart (
    q16_t r,
    q16_t theta,
    q16_t * x,
    q16_t * y
) 
```





**Parameters:**


* `r` Radius r in Q16. 
* `theta` Angle theta in Q16 radians. 
* `x` Output X coordinate in Q16. 
* `y` Output Y coordinate in Q16. 




        

<hr>



### function syn\_sph2cart 

_Convert 3D Spherical coordinates (r, theta, phi) to Cartesian coordinates (x, y, z)._ 
```C++
void syn_sph2cart (
    q16_t r,
    q16_t theta,
    q16_t phi,
    q16_t * x,
    q16_t * y,
    q16_t * z
) 
```





**Parameters:**


* `r` Radius r in Q16. 
* `theta` Azimuth angle theta in Q16 radians. 
* `phi` Polar angle (inclination) phi in Q16 radians. 
* `x` Output X coordinate in Q16. 
* `y` Output Y coordinate in Q16. 
* `z` Output Z coordinate in Q16. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_transform.h`

