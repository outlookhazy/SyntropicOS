

# File syn\_qmath.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_qmath.h**](syn__qmath_8h.md)

[Go to the source code of this file](syn__qmath_8h_source.md)

_Fixed-point Q16.16 arithmetic — header-only, no floating point._ [More...](#detailed-description)

* `#include <stdint.h>`

















## Public Types

| Type | Name |
| ---: | :--- |
| typedef int32\_t | [**q16\_t**](#typedef-q16_t)  <br> |






















## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_abs**](#function-q16_abs) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a) <br>_Absolute value of a Q16 number._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_add**](#function-q16_add) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b) <br>_Add two Q16 values._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_add\_sat**](#function-q16_add_sat) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b) <br>_Saturating add (clamp to INT32 range)._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_clamp**](#function-q16_clamp) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) val, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) lo, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) hi) <br>_Clamp a Q16 value to [lo, hi]._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_cos**](#function-q16_cos) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x) <br>_Cosine approximation via sin(x + PI/2)._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_div**](#function-q16_div) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b) <br>_Divide two Q16 values._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_lerp**](#function-q16_lerp) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) t) <br>_Linear interpolation: lerp(a, b, t) where t is Q16 in [0, 1.0]._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_mul**](#function-q16_mul) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b) <br>_Multiply two Q16 values._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_mul\_sat**](#function-q16_mul_sat) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b) <br>_Saturating multiply._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_sin**](#function-q16_sin) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x) <br>_Sine approximation using 5th-order Taylor series._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**q16\_sub**](#function-q16_sub) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b) <br>_Subtract two Q16 values._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**Q16\_2\_PI**](syn__qmath_8h.md#define-q16_2_pi)  `411775`<br> |
| define  | [**Q16\_FRAC\_1000**](syn__qmath_8h.md#define-q16_frac_1000) (q) `((int32\_t)((((q) & 0xFFFF) \* 1000L) &gt;&gt; [**Q16\_SHIFT**](syn__qmath_8h.md#define-q16_shift)))`<br> |
| define  | [**Q16\_FROM\_FLOAT**](syn__qmath_8h.md#define-q16_from_float) (f) `(([**q16\_t**](syn__qmath_8h.md#typedef-q16_t))((f) \* (1L &lt;&lt; [**Q16\_SHIFT**](syn__qmath_8h.md#define-q16_shift))))`<br> |
| define  | [**Q16\_FROM\_FRAC**](syn__qmath_8h.md#define-q16_from_frac) (num, den) `(([**q16\_t**](syn__qmath_8h.md#typedef-q16_t))(((int64\_t)((uint64\_t)(int64\_t)(num) &lt;&lt; [**Q16\_SHIFT**](syn__qmath_8h.md#define-q16_shift))) / (den)))`<br> |
| define  | [**Q16\_FROM\_INT**](syn__qmath_8h.md#define-q16_from_int) (n) `(([**q16\_t**](syn__qmath_8h.md#typedef-q16_t))((int32\_t)(n) \* [**Q16\_ONE**](syn__qmath_8h.md#define-q16_one)))`<br> |
| define  | [**Q16\_HALF**](syn__qmath_8h.md#define-q16_half)  `(([**q16\_t**](syn__qmath_8h.md#typedef-q16_t))(1L &lt;&lt; ([**Q16\_SHIFT**](syn__qmath_8h.md#define-q16_shift) - 1)))`<br> |
| define  | [**Q16\_ONE**](syn__qmath_8h.md#define-q16_one)  `(([**q16\_t**](syn__qmath_8h.md#typedef-q16_t))(1L &lt;&lt; [**Q16\_SHIFT**](syn__qmath_8h.md#define-q16_shift)))`<br> |
| define  | [**Q16\_PI**](syn__qmath_8h.md#define-q16_pi)  `205887`<br> |
| define  | [**Q16\_PI\_2**](syn__qmath_8h.md#define-q16_pi_2)  `102944`<br> |
| define  | [**Q16\_SHIFT**](syn__qmath_8h.md#define-q16_shift)  `16`<br> |
| define  | [**Q16\_TO\_INT**](syn__qmath_8h.md#define-q16_to_int) (q) `((int32\_t)((q) &gt;&gt; [**Q16\_SHIFT**](syn__qmath_8h.md#define-q16_shift)))`<br> |
| define  | [**Q16\_TO\_INT\_ROUND**](syn__qmath_8h.md#define-q16_to_int_round) (q) `((int32\_t)(((q) + [**Q16\_HALF**](syn__qmath_8h.md#define-q16_half)) &gt;&gt; [**Q16\_SHIFT**](syn__qmath_8h.md#define-q16_shift)))`<br> |

## Detailed Description


All operations use signed 32-bit values with 16 integer bits and 16 fractional bits. Multiply/divide use 64-bit intermediates to avoid overflow.




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



### function q16\_cos 

_Cosine approximation via sin(x + PI/2)._ 
```C++
static inline q16_t q16_cos (
    q16_t x
) 
```





**Parameters:**


* `x` Angle in Q16 radians. 



**Returns:**

cos(x) in Q16. 





        

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



### function q16\_sin 

_Sine approximation using 5th-order Taylor series._ 
```C++
static inline q16_t q16_sin (
    q16_t x
) 
```





**Parameters:**


* `x` Angle in Q16 radians. 



**Returns:**

sin(x) in Q16. 





        

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
## Macro Definition Documentation





### define Q16\_2\_PI 

```C++
#define Q16_2_PI `411775`
```



6.28318 in Q16.16 


        

<hr>



### define Q16\_FRAC\_1000 

```C++
#define Q16_FRAC_1000 (
    q
) `((int32_t)((((q) & 0xFFFF) * 1000L) >> Q16_SHIFT ))`
```



Fractional part as 0–999 (for printf: "%d.%03d"). 


        

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



3.14159 in Q16.16 


        

<hr>



### define Q16\_PI\_2 

```C++
#define Q16_PI_2 `102944`
```



1.57079 in Q16.16 


        

<hr>



### define Q16\_SHIFT 

```C++
#define Q16_SHIFT `16`
```



Fractional bit count 
 


        

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

