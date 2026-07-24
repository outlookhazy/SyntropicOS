

# File syn\_qmath.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_qmath.h**](syn__qmath_8h.md)

[Go to the source code of this file](syn__qmath_8h_source.md)

_Fixed-point Q16.16 arithmetic — no floating point._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stddef.h>`

















## Public Types

| Type | Name |
| ---: | :--- |
| typedef int32\_t | [**q16\_t**](#typedef-q16_t)  <br> |




















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
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_abs**](#function-q16_abs) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a) <br>_Absolute value of a Q16 number._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_add**](#function-q16_add) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b) <br>_Add two Q16 values._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_add\_sat**](#function-q16_add_sat) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b) <br>_Saturating add (clamp to INT32 range)._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_clamp**](#function-q16_clamp) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) val, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) lo, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) hi) <br>_Clamp a Q16 value to [lo, hi]._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_div**](#function-q16_div) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b) <br>_Divide two Q16 values._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_lerp**](#function-q16_lerp) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) t) <br>_Linear interpolation: lerp(a, b, t) where t is Q16 in [0, 1.0]._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_mul**](#function-q16_mul) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b) <br>_Multiply two Q16 values._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_mul\_sat**](#function-q16_mul_sat) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b) <br>_Saturating multiply._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_sub**](#function-q16_sub) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b) <br>_Subtract two Q16 values._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_sub\_sat**](#function-q16_sub_sat) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b) <br>_Saturating subtract (clamp to INT32 range)._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**Q16\_2\_PI**](syn__qmath_8h.md#define-q16_2_pi)  `411775`<br> |
| define  | [**Q16\_E**](syn__qmath_8h.md#define-q16_e)  `178145`<br> |
| define  | [**Q16\_FRAC\_1000**](syn__qmath_8h.md#define-q16_frac_1000) (q) `((int32\_t)((((q) & 0xFFFF) \* 1000L) &gt;&gt; [**Q16\_SHIFT**](syn__qmath_8h.md#define-q16_shift)))`<br> |
| define  | [**Q16\_FRAC\_10000**](syn__qmath_8h.md#define-q16_frac_10000) (q) `((int32\_t)((((q) & 0xFFFF) \* 10000L) &gt;&gt; [**Q16\_SHIFT**](syn__qmath_8h.md#define-q16_shift)))`<br> |
| define  | [**Q16\_FROM\_FLOAT**](syn__qmath_8h.md#define-q16_from_float) (f) `(([**q16\_t**](syn__qmath_8h.md#typedef-q16_t))((f) \* (1L &lt;&lt; [**Q16\_SHIFT**](syn__qmath_8h.md#define-q16_shift))))`<br> |
| define  | [**Q16\_FROM\_FRAC**](syn__qmath_8h.md#define-q16_from_frac) (num, den) `(([**q16\_t**](syn__qmath_8h.md#typedef-q16_t))(((int64\_t)((uint64\_t)(int64\_t)(num) &lt;&lt; [**Q16\_SHIFT**](syn__qmath_8h.md#define-q16_shift))) / (den)))`<br> |
| define  | [**Q16\_FROM\_INT**](syn__qmath_8h.md#define-q16_from_int) (n) `(([**q16\_t**](syn__qmath_8h.md#typedef-q16_t))((int32\_t)(n) \* [**Q16\_ONE**](syn__qmath_8h.md#define-q16_one)))`<br> |
| define  | [**Q16\_HALF**](syn__qmath_8h.md#define-q16_half)  `(([**q16\_t**](syn__qmath_8h.md#typedef-q16_t))(1L &lt;&lt; ([**Q16\_SHIFT**](syn__qmath_8h.md#define-q16_shift) - 1)))`<br> |
| define  | [**Q16\_LN2**](syn__qmath_8h.md#define-q16_ln2)  `45426`<br> |
| define  | [**Q16\_ONE**](syn__qmath_8h.md#define-q16_one)  `(([**q16\_t**](syn__qmath_8h.md#typedef-q16_t))(1L &lt;&lt; [**Q16\_SHIFT**](syn__qmath_8h.md#define-q16_shift)))`<br> |
| define  | [**Q16\_PI**](syn__qmath_8h.md#define-q16_pi)  `205887`<br> |
| define  | [**Q16\_PI\_2**](syn__qmath_8h.md#define-q16_pi_2)  `102944`<br> |
| define  | [**Q16\_SHIFT**](syn__qmath_8h.md#define-q16_shift)  `16`<br> |
| define  | [**Q16\_SQRT2**](syn__qmath_8h.md#define-q16_sqrt2)  `92682`<br> |
| define  | [**Q16\_TO\_INT**](syn__qmath_8h.md#define-q16_to_int) (q) `((int32\_t)((q) &gt;&gt; [**Q16\_SHIFT**](syn__qmath_8h.md#define-q16_shift)))`<br> |
| define  | [**Q16\_TO\_INT\_ROUND**](syn__qmath_8h.md#define-q16_to_int_round) (q) `((int32\_t)(((q) + [**Q16\_HALF**](syn__qmath_8h.md#define-q16_half)) &gt;&gt; [**Q16\_SHIFT**](syn__qmath_8h.md#define-q16_shift)))`<br> |

## Detailed Description


All operations use signed 32-bit values with 16 integer bits and 16 fractional bits. Multiply/divide use 64-bit intermediates to avoid overflow.


Basic arithmetic (add, sub, mul, div, lerp, clamp) is inline in this header. Transcendental functions (sin, cos, sqrt, atan2, exp, log) and string I/O are compiled in [**syn\_qmath.c**](syn__qmath_8c.md).




**
**


```C++
q16_t a = Q16_FROM_INT(3);         // 3.0
q16_t b = Q16_FROM_FRAC(1, 2);     // 0.5
q16_t c = q16_mul(a, b);           // 1.5
int   i = Q16_TO_INT(c);           // 1
int   f = Q16_FRAC_1000(c);        // 500 (fractional part × 1000)
```
 





    
## Public Types Documentation




### typedef q16\_t 

```C++
typedef int32_t q16_t;
```



Fixed-point Q16.16 type: 16 integer bits, 16 fractional bits. 


        

<hr>
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




### function q16\_abs 

_Absolute value of a Q16 number._ 
```C++
static inline q16_t q16_abs (
    q16_t a
) 
```





**Parameters:**


* `a` Input value. 



**Returns:**

\|a\| in Q16. 





        

<hr>



### function q16\_add 

_Add two Q16 values._ 
```C++
static inline q16_t q16_add (
    q16_t a,
    q16_t b
) 
```





**Parameters:**


* `a` First operand. 
* `b` Second operand. 



**Returns:**

Sum in Q16. 





        

<hr>



### function q16\_add\_sat 

_Saturating add (clamp to INT32 range)._ 
```C++
static inline q16_t q16_add_sat (
    q16_t a,
    q16_t b
) 
```





**Parameters:**


* `a` First operand. 
* `b` Second operand. 



**Returns:**

Clamped sum in Q16. 





        

<hr>



### function q16\_clamp 

_Clamp a Q16 value to [lo, hi]._ 
```C++
static inline q16_t q16_clamp (
    q16_t val,
    q16_t lo,
    q16_t hi
) 
```





**Parameters:**


* `val` Input value. 
* `lo` Lower bound. 
* `hi` Upper bound. 



**Returns:**

Clamped value. 





        

<hr>



### function q16\_div 

_Divide two Q16 values._ 
```C++
static inline q16_t q16_div (
    q16_t a,
    q16_t b
) 
```





**Parameters:**


* `a` Dividend. 
* `b` Divisor (must not be zero). 



**Returns:**

Quotient in Q16. 





        

<hr>



### function q16\_lerp 

_Linear interpolation: lerp(a, b, t) where t is Q16 in [0, 1.0]._ 
```C++
static inline q16_t q16_lerp (
    q16_t a,
    q16_t b,
    q16_t t
) 
```





**Parameters:**


* `a` Start value. 
* `b` End value. 
* `t` Interpolation factor (Q16, 0 to Q16\_ONE). 



**Returns:**

Interpolated value in Q16. 





        

<hr>



### function q16\_mul 

_Multiply two Q16 values._ 
```C++
static inline q16_t q16_mul (
    q16_t a,
    q16_t b
) 
```





**Parameters:**


* `a` First operand. 
* `b` Second operand. 



**Returns:**

Product in Q16. 





        

<hr>



### function q16\_mul\_sat 

_Saturating multiply._ 
```C++
static inline q16_t q16_mul_sat (
    q16_t a,
    q16_t b
) 
```





**Parameters:**


* `a` First operand. 
* `b` Second operand. 



**Returns:**

Clamped product in Q16. 





        

<hr>



### function q16\_sub 

_Subtract two Q16 values._ 
```C++
static inline q16_t q16_sub (
    q16_t a,
    q16_t b
) 
```





**Parameters:**


* `a` Minuend. 
* `b` Subtrahend. 



**Returns:**

Difference in Q16. 





        

<hr>



### function q16\_sub\_sat 

_Saturating subtract (clamp to INT32 range)._ 
```C++
static inline q16_t q16_sub_sat (
    q16_t a,
    q16_t b
) 
```





**Parameters:**


* `a` Minuend. 
* `b` Subtrahend. 



**Returns:**

Clamped difference in Q16. 





        

<hr>
## Macro Definition Documentation





### define Q16\_2\_PI 

```C++
#define Q16_2_PI `411775`
```



6.28318531 in Q16.16 


        

<hr>



### define Q16\_E 

```C++
#define Q16_E `178145`
```



2.71828183 in Q16.16 


        

<hr>



### define Q16\_FRAC\_1000 

```C++
#define Q16_FRAC_1000 (
    q
) `((int32_t)((((q) & 0xFFFF) * 1000L) >> Q16_SHIFT ))`
```



Fractional part as 0–999 (for printf: "%d.%03d"). 


        

<hr>



### define Q16\_FRAC\_10000 

```C++
#define Q16_FRAC_10000 (
    q
) `((int32_t)((((q) & 0xFFFF) * 10000L) >> Q16_SHIFT ))`
```



Fractional part as 0–9999 (for printf: "%d.%04d"). 


        

<hr>



### define Q16\_FROM\_FLOAT 

```C++
#define Q16_FROM_FLOAT (
    f
) `(( q16_t )((f) * (1L << Q16_SHIFT )))`
```



Float literal to Q16 (compile-time only, avoid at runtime). 


        

<hr>



### define Q16\_FROM\_FRAC 

```C++
#define Q16_FROM_FRAC (
    num,
    den
) `(( q16_t )(((int64_t)((uint64_t)(int64_t)(num) << Q16_SHIFT )) / (den)))`
```



Fraction to Q16: [**Q16\_FROM\_FRAC(1, 3)**](syn__qmath_8h.md#define-q16_from_frac) ≈ 0.333. 


        

<hr>



### define Q16\_FROM\_INT 

```C++
#define Q16_FROM_INT (
    n
) `(( q16_t )((int32_t)(n) * Q16_ONE ))`
```



Integer to Q16. 


        

<hr>



### define Q16\_HALF 

```C++
#define Q16_HALF `(( q16_t )(1L << ( Q16_SHIFT - 1)))`
```



0.5 in Q16.16 
 


        

<hr>



### define Q16\_LN2 

```C++
#define Q16_LN2 `45426`
```



0.69314718 in Q16.16 


        

<hr>



### define Q16\_ONE 

```C++
#define Q16_ONE `(( q16_t )(1L << Q16_SHIFT ))`
```



1.0 in Q16.16 
 


        

<hr>



### define Q16\_PI 

```C++
#define Q16_PI `205887`
```



3.14159265 in Q16.16 


        

<hr>



### define Q16\_PI\_2 

```C++
#define Q16_PI_2 `102944`
```



1.57079633 in Q16.16 


        

<hr>



### define Q16\_SHIFT 

```C++
#define Q16_SHIFT `16`
```



Fractional bit count 
 


        

<hr>



### define Q16\_SQRT2 

```C++
#define Q16_SQRT2 `92682`
```



1.41421356 in Q16.16 


        

<hr>



### define Q16\_TO\_INT 

```C++
#define Q16_TO_INT (
    q
) `((int32_t)((q) >> Q16_SHIFT ))`
```



Q16 to integer (truncates toward zero). 


        

<hr>



### define Q16\_TO\_INT\_ROUND 

```C++
#define Q16_TO_INT_ROUND (
    q
) `((int32_t)(((q) + Q16_HALF ) >> Q16_SHIFT ))`
```



Q16 to integer (rounded). 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_qmath.h`

