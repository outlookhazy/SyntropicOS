

# File syn\_vector.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_vector.h**](syn__vector_8h.md)

[Go to the source code of this file](syn__vector_8h_source.md)

_Fixed-point Q16.16 Vector operations and signal statistics — zero heap allocation._ 

* `#include "../common/syn_defs.h"`
* `#include "syn_qmath.h"`
* `#include <stdbool.h>`
* `#include <stdint.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_vec\_add**](#function-syn_vec_add) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* a, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* b, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out, uint16\_t n) <br>_Elementwise vector addition out = a + b._  |
|  void | [**syn\_vec\_clamp**](#function-syn_vec_clamp) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* v, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) min\_val, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) max\_val, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out, uint16\_t n) <br>_Elementwise vector clamping: min\_val &lt;= out[i] &lt;= max\_val._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**syn\_vec\_max**](#function-syn_vec_max) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* v, uint16\_t n) <br>_Find maximum value in vector._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**syn\_vec\_mean**](#function-syn_vec_mean) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* v, uint16\_t n) <br>_Calculate arithmetic mean (average) of vector._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**syn\_vec\_min**](#function-syn_vec_min) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* v, uint16\_t n) <br>_Find minimum value in vector._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**syn\_vec\_rms**](#function-syn_vec_rms) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* v, uint16\_t n) <br>_Calculate Root Mean Square (RMS) of vector._  |
|  void | [**syn\_vec\_scale**](#function-syn_vec_scale) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* v, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) scale, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out, uint16\_t n) <br>_Elementwise vector scalar multiplication out = v \* scale._  |
|  void | [**syn\_vec\_sub**](#function-syn_vec_sub) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* a, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* b, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out, uint16\_t n) <br>_Elementwise vector subtraction out = a - b._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**syn\_vec\_variance**](#function-syn_vec_variance) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* v, uint16\_t n) <br>_Calculate variance of vector elements._  |




























## Public Functions Documentation




### function syn\_vec\_add 

_Elementwise vector addition out = a + b._ 
```C++
void syn_vec_add (
    const q16_t * a,
    const q16_t * b,
    q16_t * out,
    uint16_t n
) 
```





**Parameters:**


* `a` Pointer to input vector A. 
* `b` Pointer to input vector B. 
* `out` Pointer to destination output vector. 
* `n` Number of elements in vectors. 




        

<hr>



### function syn\_vec\_clamp 

_Elementwise vector clamping: min\_val &lt;= out[i] &lt;= max\_val._ 
```C++
void syn_vec_clamp (
    const q16_t * v,
    q16_t min_val,
    q16_t max_val,
    q16_t * out,
    uint16_t n
) 
```





**Parameters:**


* `v` Pointer to input vector. 
* `min_val` Minimum lower limit threshold. 
* `max_val` Maximum upper limit threshold. 
* `out` Pointer to destination output vector. 
* `n` Number of elements in vector. 




        

<hr>



### function syn\_vec\_max 

_Find maximum value in vector._ 
```C++
q16_t syn_vec_max (
    const q16_t * v,
    uint16_t n
) 
```





**Parameters:**


* `v` Pointer to input vector. 
* `n` Number of elements. 



**Returns:**

Maximum value in vector, or 0 if n == 0. 





        

<hr>



### function syn\_vec\_mean 

_Calculate arithmetic mean (average) of vector._ 
```C++
q16_t syn_vec_mean (
    const q16_t * v,
    uint16_t n
) 
```





**Parameters:**


* `v` Pointer to input vector. 
* `n` Number of elements. 



**Returns:**

Mean value in Q16.16, or 0 if n == 0. 





        

<hr>



### function syn\_vec\_min 

_Find minimum value in vector._ 
```C++
q16_t syn_vec_min (
    const q16_t * v,
    uint16_t n
) 
```





**Parameters:**


* `v` Pointer to input vector. 
* `n` Number of elements. 



**Returns:**

Minimum value in vector, or 0 if n == 0. 





        

<hr>



### function syn\_vec\_rms 

_Calculate Root Mean Square (RMS) of vector._ 
```C++
q16_t syn_vec_rms (
    const q16_t * v,
    uint16_t n
) 
```





**Parameters:**


* `v` Pointer to input vector. 
* `n` Number of elements. 



**Returns:**

RMS value in Q16.16 format. 





        

<hr>



### function syn\_vec\_scale 

_Elementwise vector scalar multiplication out = v \* scale._ 
```C++
void syn_vec_scale (
    const q16_t * v,
    q16_t scale,
    q16_t * out,
    uint16_t n
) 
```





**Parameters:**


* `v` Pointer to input vector. 
* `scale` Scalar multiplier in Q16.16. 
* `out` Pointer to destination output vector. 
* `n` Number of elements in vector. 




        

<hr>



### function syn\_vec\_sub 

_Elementwise vector subtraction out = a - b._ 
```C++
void syn_vec_sub (
    const q16_t * a,
    const q16_t * b,
    q16_t * out,
    uint16_t n
) 
```





**Parameters:**


* `a` Pointer to input vector A. 
* `b` Pointer to input vector B. 
* `out` Pointer to destination output vector. 
* `n` Number of elements in vectors. 




        

<hr>



### function syn\_vec\_variance 

_Calculate variance of vector elements._ 
```C++
q16_t syn_vec_variance (
    const q16_t * v,
    uint16_t n
) 
```





**Parameters:**


* `v` Pointer to input vector. 
* `n` Number of elements. 



**Returns:**

Variance in Q16.16 format. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_vector.h`

