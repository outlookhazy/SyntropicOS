

# File syn\_qmath.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_qmath.c**](syn__qmath_8c.md)

[Go to the source code of this file](syn__qmath_8c_source.md)

_Q16.16 fixed-point transcendental functions and string I/O._ [More...](#detailed-description)

* `#include "syn_qmath.h"`
* `#include "../common/syn_defs.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_acos**](#function-q16_acos) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x) <br>_Arc-cosine: π/2 − asin(x)._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_asin**](#function-q16_asin) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x) <br>_Arc-sine (5th-order Chebyshev polynomial)._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_atan2**](#function-q16_atan2) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) y, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x) <br>_Four-quadrant arctangent (minimax polynomial)._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_ceil**](#function-q16_ceil) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x) <br>_Fixed-point ceil: smallest integer &gt;= x._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_cos**](#function-q16_cos) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x) <br>_Cosine approximation via sin(x + π/2)._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_exp**](#function-q16_exp) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x) <br>_Fixed-point exponential e^x (range reduction + minimax polynomial)._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_floor**](#function-q16_floor) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x) <br>_Fixed-point floor: largest integer &lt;= x._  |
|  size\_t | [**q16\_from\_str**](#function-q16_from_str) (const char \* str, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out) <br>_Parse a decimal string (e.g. "-3.1415") into Q16._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_hypot**](#function-q16_hypot) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) y) <br>_Overflow-safe hypotenuse: √(x² + y²)._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_log**](#function-q16_log) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x) <br>_Fixed-point natural logarithm ln(x) (CLZ + minimax polynomial)._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_poly\_eval**](#function-q16_poly_eval) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* coeffs, uint8\_t n, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x) <br>_Evaluate polynomial P(x) = c0 + c1\*x + c2\*x^2 + ... + cn\*x^n via Horner's method._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_pow**](#function-q16_pow) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) base, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) exp) <br>_Fixed-point power: base^exp = exp(exp \* log(base))._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_round**](#function-q16_round) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x) <br>_Fixed-point round: nearest integer (half rounded up)._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_sin**](#function-q16_sin) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x) <br>_Sine approximation (5th-order Taylor series)._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_sqrt**](#function-q16_sqrt) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x) <br>_Fixed-point square root (binary restoring algorithm)._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_tan**](#function-q16_tan) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x) <br>_Tangent: sin(x) / cos(x)._  |
|  size\_t | [**q16\_to\_str**](#function-q16_to_str) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) val, char \* buf, size\_t buf\_len, uint8\_t decimals) <br>_Format a Q16 value as a decimal string (e.g. "-12.345")._  |
|  uint32\_t | [**syn\_isqrt32**](#function-syn_isqrt32) (uint32\_t n) <br>_Compute floor of 32-bit unsigned integer square root: ⌊√n⌋._  |
|  uint64\_t | [**syn\_isqrt64**](#function-syn_isqrt64) (uint64\_t n) <br>_Compute floor of 64-bit unsigned integer square root: ⌊√n⌋._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**atan\_core**](#function-atan_core) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x) <br>_Core atan approximation for x in [0, 1] (Q16.16)._  |


























## Detailed Description


All functions use 64-bit intermediates for multiply chains. No floating point, no heap, no division by zero. Optimized for 32-bit ARM Cortex-M with hardware multiply; functional on 8-bit AVR. 


    
## Public Functions Documentation




### function q16\_acos 

_Arc-cosine: π/2 − asin(x)._ 
```C++
q16_t q16_acos (
    q16_t x
) 
```





**Parameters:**


* `x` Input in Q16, must be in [−1.0, 1.0]. 



**Returns:**

Angle in Q16 radians [0, π]. 





        

<hr>



### function q16\_asin 

_Arc-sine (5th-order Chebyshev polynomial)._ 
```C++
q16_t q16_asin (
    q16_t x
) 
```





**Parameters:**


* `x` Input in Q16, must be in [−1.0, 1.0]. 



**Returns:**

Angle in Q16 radians [−π/2, π/2]. 





        

<hr>



### function q16\_atan2 

_Four-quadrant arctangent (minimax polynomial)._ 
```C++
q16_t q16_atan2 (
    q16_t y,
    q16_t x
) 
```





**Parameters:**


* `y` Y coordinate in Q16. 
* `x` X coordinate in Q16. 



**Returns:**

Angle in Q16 radians [−π, π]. 





        

<hr>



### function q16\_ceil 

_Fixed-point ceil: smallest integer &gt;= x._ 
```C++
q16_t q16_ceil (
    q16_t x
) 
```





**Parameters:**


* `x` Input in Q16. 



**Returns:**

Ceil value in Q16. 





        

<hr>



### function q16\_cos 

_Cosine approximation via sin(x + π/2)._ 
```C++
q16_t q16_cos (
    q16_t x
) 
```





**Parameters:**


* `x` Angle in Q16 radians. 



**Returns:**

cos(x) in Q16. 





        

<hr>



### function q16\_exp 

_Fixed-point exponential e^x (range reduction + minimax polynomial)._ 
```C++
q16_t q16_exp (
    q16_t x
) 
```





**Parameters:**


* `x` Exponent in Q16. 



**Returns:**

e^x in Q16. Returns INT32\_MAX on overflow. 





        

<hr>



### function q16\_floor 

_Fixed-point floor: largest integer &lt;= x._ 
```C++
q16_t q16_floor (
    q16_t x
) 
```





**Parameters:**


* `x` Input in Q16. 



**Returns:**

Floor value in Q16. 





        

<hr>



### function q16\_from\_str 

_Parse a decimal string (e.g. "-3.1415") into Q16._ 
```C++
size_t q16_from_str (
    const char * str,
    q16_t * out
) 
```



Handles optional sign, integer part, optional decimal point and fractional digits. Stops at first non-numeric character.




**Parameters:**


* `str` Input string (NUL-terminated). 
* `out` Output Q16 value. 



**Returns:**

Number of characters consumed, or 0 on parse error. 





        

<hr>



### function q16\_hypot 

_Overflow-safe hypotenuse: √(x² + y²)._ 
```C++
q16_t q16_hypot (
    q16_t x,
    q16_t y
) 
```





**Parameters:**


* `x` First coordinate in Q16. 
* `y` Second coordinate in Q16. 



**Returns:**

Magnitude in Q16. 





        

<hr>



### function q16\_log 

_Fixed-point natural logarithm ln(x) (CLZ + minimax polynomial)._ 
```C++
q16_t q16_log (
    q16_t x
) 
```





**Parameters:**


* `x` Input in Q16 (must be &gt; 0). 



**Returns:**

ln(x) in Q16. 





        

<hr>



### function q16\_poly\_eval 

_Evaluate polynomial P(x) = c0 + c1\*x + c2\*x^2 + ... + cn\*x^n via Horner's method._ 
```C++
q16_t q16_poly_eval (
    const q16_t * coeffs,
    uint8_t n,
    q16_t x
) 
```





**Parameters:**


* `coeffs` Array of polynomial coefficients [c0, c1, ..., cn]. 
* `n` Number of coefficients (order = n - 1). 
* `x` Evaluation point in Q16. 



**Returns:**

P(x) in Q16. 





        

<hr>



### function q16\_pow 

_Fixed-point power: base^exp = exp(exp \* log(base))._ 
```C++
q16_t q16_pow (
    q16_t base,
    q16_t exp
) 
```





**Parameters:**


* `base` Base in Q16 (must be &gt; 0). 
* `exp` Exponent in Q16. 



**Returns:**

base^exp in Q16. 





        

<hr>



### function q16\_round 

_Fixed-point round: nearest integer (half rounded up)._ 
```C++
q16_t q16_round (
    q16_t x
) 
```





**Parameters:**


* `x` Input in Q16. 



**Returns:**

Rounded value in Q16. 





        

<hr>



### function q16\_sin 

_Sine approximation (5th-order Taylor series)._ 
```C++
q16_t q16_sin (
    q16_t x
) 
```





**Parameters:**


* `x` Angle in Q16 radians. 



**Returns:**

sin(x) in Q16. 





        

<hr>



### function q16\_sqrt 

_Fixed-point square root (binary restoring algorithm)._ 
```C++
q16_t q16_sqrt (
    q16_t x
) 
```





**Parameters:**


* `x` Input in Q16 (must be ≥ 0). 



**Returns:**

√x in Q16. 





        

<hr>



### function q16\_tan 

_Tangent: sin(x) / cos(x)._ 
```C++
q16_t q16_tan (
    q16_t x
) 
```





**Parameters:**


* `x` Angle in Q16 radians. Must not be near ±π/2. 



**Returns:**

tan(x) in Q16. 





        

<hr>



### function q16\_to\_str 

_Format a Q16 value as a decimal string (e.g. "-12.345")._ 
```C++
size_t q16_to_str (
    q16_t val,
    char * buf,
    size_t buf_len,
    uint8_t decimals
) 
```



No heap allocation. The caller provides the output buffer.




**Parameters:**


* `val` Value to format. 
* `buf` Output buffer (must be large enough). 
* `buf_len` Size of output buffer in bytes. 
* `decimals` Number of fractional digits (0–4). 



**Returns:**

Number of characters written (excluding NUL), or 0 on error. 





        

<hr>



### function syn\_isqrt32 

_Compute floor of 32-bit unsigned integer square root: ⌊√n⌋._ 
```C++
uint32_t syn_isqrt32 (
    uint32_t n
) 
```





**Parameters:**


* `n` Unsigned 32-bit integer. 



**Returns:**

⌊√n⌋. 





        

<hr>



### function syn\_isqrt64 

_Compute floor of 64-bit unsigned integer square root: ⌊√n⌋._ 
```C++
uint64_t syn_isqrt64 (
    uint64_t n
) 
```





**Parameters:**


* `n` Unsigned 64-bit integer. 



**Returns:**

⌊√n⌋. 





        

<hr>
## Public Static Functions Documentation




### function atan\_core 

_Core atan approximation for x in [0, 1] (Q16.16)._ 
```C++
static q16_t atan_core (
    q16_t x
) 
```



Uses a 7th-order minimax polynomial: atan(x) ≈ c1\*x + c3\*x³ + c5\*x⁵ + c7\*x⁷


Max error &lt; 0.0003 rad over [0, 1]. High precision for Q16.16.




**Parameters:**


* `x` Input value in Q16.16 (must be in [0, Q16\_ONE]). 



**Returns:**

Arctangent in Q16.16. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_qmath.c`

