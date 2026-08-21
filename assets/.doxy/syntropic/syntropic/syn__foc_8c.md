

# File syn\_foc.c



[**FileList**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_foc.c**](syn__foc_8c.md)

[Go to the source code of this file](syn__foc_8c_source.md)

_Fixed-point Field-Oriented Control transforms._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../util/syn_assert.h"`
* `#include "syn_foc.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_foc\_clarke**](#function-syn_foc_clarke) (const [**SYN\_FOC\_ABC**](structSYN__FOC__ABC.md) \* abc, [**SYN\_FOC\_AB**](structSYN__FOC__AB.md) \* ab) <br>_Clarke transform: 3-phase (a,b,c) → 2-phase (α,β)._  |
|  void | [**syn\_foc\_deadtime\_comp**](#function-syn_foc_deadtime_comp) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* duty\_a, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* duty\_b, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* duty\_c, const [**SYN\_FOC\_ABC**](structSYN__FOC__ABC.md) \* i\_abc, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) dt\_comp) <br>_3-Phase Inverter Dead-Time Voltage Compensation._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_foc\_field\_weakening**](#function-syn_foc_field_weakening) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) v\_d, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) v\_q, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) v\_max, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* id\_cmd) <br>_Automatic Field-Weakening Controller._  |
|  void | [**syn\_foc\_inv\_clarke**](#function-syn_foc_inv_clarke) (const [**SYN\_FOC\_AB**](structSYN__FOC__AB.md) \* ab, [**SYN\_FOC\_ABC**](structSYN__FOC__ABC.md) \* abc) <br>_Inverse Clarke: 2-phase (α,β) → 3-phase (a,b,c)._  |
|  void | [**syn\_foc\_inv\_park**](#function-syn_foc_inv_park) (const [**SYN\_FOC\_DQ**](structSYN__FOC__DQ.md) \* dq, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) theta, [**SYN\_FOC\_AB**](structSYN__FOC__AB.md) \* ab) <br>_Inverse Park: rotating (d,q) → stationary (α,β)._  |
|  void | [**syn\_foc\_inv\_park\_fast**](#function-syn_foc_inv_park_fast) (const [**SYN\_FOC\_DQ**](structSYN__FOC__DQ.md) \* dq, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) theta, [**SYN\_FOC\_AB**](structSYN__FOC__AB.md) \* ab) <br>_Inverse Park: rotating (d,q) -&gt; stationary (alpha,beta) using ultra-fast sincos._  |
|  void | [**syn\_foc\_park**](#function-syn_foc_park) (const [**SYN\_FOC\_AB**](structSYN__FOC__AB.md) \* ab, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) theta, [**SYN\_FOC\_DQ**](structSYN__FOC__DQ.md) \* dq) <br>_Park transform: stationary (α,β) → rotating (d,q)._  |
|  void | [**syn\_foc\_park\_fast**](#function-syn_foc_park_fast) (const [**SYN\_FOC\_AB**](structSYN__FOC__AB.md) \* ab, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) theta, [**SYN\_FOC\_DQ**](structSYN__FOC__DQ.md) \* dq) <br>_Park transform: stationary (alpha,beta) -&gt; rotating (d,q) using ultra-fast sincos._  |
|  void | [**syn\_foc\_svpwm**](#function-syn_foc_svpwm) (const [**SYN\_FOC\_AB**](structSYN__FOC__AB.md) \* ab, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) v\_bus, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* duty\_a, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* duty\_b, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* duty\_c) <br>_Compute Space Vector PWM duty cycles from (α, β)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**dt\_sign\_scale**](#function-dt_sign_scale) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) current, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) dt\_comp) <br>_Compute sign-adjusted dead-time duty cycle offset with zero-crossing interpolation._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**Q16\_2\_INV\_SQRT3**](syn__foc_8c.md#define-q16_2_inv_sqrt3)  `75674`<br>_Precomputed 2/√3 in Q16.16 (≈ 1.15470)._  |
| define  | [**Q16\_INV\_SQRT3**](syn__foc_8c.md#define-q16_inv_sqrt3)  `37837`<br>_Precomputed 1/√3 in Q16.16 (≈ 0.57735)._  |
| define  | [**Q16\_SQRT3**](syn__foc_8c.md#define-q16_sqrt3)  `113512`<br>_Precomputed √3 in Q16.16 (≈ 1.73205)._  |
| define  | [**Q16\_SQRT3\_OVER\_2**](syn__foc_8c.md#define-q16_sqrt3_over_2)  `56756`<br>_Precomputed √3/2 in Q16.16 (≈ 0.86603)._  |

## Detailed Description


All transforms use Q16.16 arithmetic with int64\_t intermediates. No floating point, no heap allocation. 


    
## Public Functions Documentation




### function syn\_foc\_clarke 

_Clarke transform: 3-phase (a,b,c) → 2-phase (α,β)._ 
```C++
void syn_foc_clarke (
    const SYN_FOC_ABC * abc,
    SYN_FOC_AB * ab
) 
```



Power-invariant form: α = a β = (a + 2b) / √3


Assumes balanced 3-phase: a + b + c = 0.




**Parameters:**


* `abc` Input 3-phase values. 
* `ab` Output (α, β). 




        

<hr>



### function syn\_foc\_deadtime\_comp 

_3-Phase Inverter Dead-Time Voltage Compensation._ 
```C++
void syn_foc_deadtime_comp (
    q16_t * duty_a,
    q16_t * duty_b,
    q16_t * duty_c,
    const SYN_FOC_ABC * i_abc,
    q16_t dt_comp
) 
```



Adjusts phase duty cycles to compensate for MOSFET/IGBT bridge dead-time distortion based on measured phase current polarities.




**Parameters:**


* `duty_a` [in/out] Phase A duty cycle [0, Q16\_ONE]. 
* `duty_b` [in/out] Phase B duty cycle [0, Q16\_ONE]. 
* `duty_c` [in/out] Phase C duty cycle [0, Q16\_ONE]. 
* `i_abc` Measured 3-phase currents (Q16.16). 
* `dt_comp` Dead-time compensation duty offset (Q16.16). 




        

<hr>



### function syn\_foc\_field\_weakening 

_Automatic Field-Weakening Controller._ 
```C++
bool syn_foc_field_weakening (
    q16_t v_d,
    q16_t v_q,
    q16_t v_max,
    q16_t * id_cmd
) 
```



Injects negative d-axis current (Id &lt; 0) when commanded voltage magnitude exceeds maximum available bus voltage limit (V\_max = V\_bus / sqrt(3)), allowing high-speed motor operation above rated base speed.




**Parameters:**


* `v_d` Current D-axis voltage command (Q16). 
* `v_q` Current Q-axis voltage command (Q16). 
* `v_max` Maximum available phase voltage magnitude (Q16). 
* `id_cmd` [in/out] Current D-axis command (Q16). Adjusted if over-voltage occurs. 



**Returns:**

true if field weakening active, false if operating normally. 





        

<hr>



### function syn\_foc\_inv\_clarke 

_Inverse Clarke: 2-phase (α,β) → 3-phase (a,b,c)._ 
```C++
void syn_foc_inv_clarke (
    const SYN_FOC_AB * ab,
    SYN_FOC_ABC * abc
) 
```





**Parameters:**


* `ab` Input (α, β). 
* `abc` Output 3-phase values. 




        

<hr>



### function syn\_foc\_inv\_park 

_Inverse Park: rotating (d,q) → stationary (α,β)._ 
```C++
void syn_foc_inv_park (
    const SYN_FOC_DQ * dq,
    q16_t theta,
    SYN_FOC_AB * ab
) 
```



α = d·cos(θ) − q·sin(θ) β = d·sin(θ) + q·cos(θ)




**Parameters:**


* `dq` Input rotating-frame values. 
* `theta` Electrical rotor angle in Q16 radians. 
* `ab` Output stationary-frame values. 




        

<hr>



### function syn\_foc\_inv\_park\_fast 

_Inverse Park: rotating (d,q) -&gt; stationary (alpha,beta) using ultra-fast sincos._ 
```C++
void syn_foc_inv_park_fast (
    const SYN_FOC_DQ * dq,
    q16_t theta,
    SYN_FOC_AB * ab
) 
```





**Parameters:**


* `dq` Input rotating-frame values. 
* `theta` Electrical angle in radians (Q16.16). 
* `ab` Output stationary-frame values. 




        

<hr>



### function syn\_foc\_park 

_Park transform: stationary (α,β) → rotating (d,q)._ 
```C++
void syn_foc_park (
    const SYN_FOC_AB * ab,
    q16_t theta,
    SYN_FOC_DQ * dq
) 
```



d = α·cos(θ) + β·sin(θ) q = −α·sin(θ) + β·cos(θ)




**Parameters:**


* `ab` Input stationary-frame values. 
* `theta` Electrical rotor angle in Q16 radians. 
* `dq` Output rotating-frame values. 




        

<hr>



### function syn\_foc\_park\_fast 

_Park transform: stationary (alpha,beta) -&gt; rotating (d,q) using ultra-fast sincos._ 
```C++
void syn_foc_park_fast (
    const SYN_FOC_AB * ab,
    q16_t theta,
    SYN_FOC_DQ * dq
) 
```





**Parameters:**


* `ab` Input stationary-frame values. 
* `theta` Electrical angle in radians (Q16.16). 
* `dq` Output rotating-frame values. 




        

<hr>



### function syn\_foc\_svpwm 

_Compute Space Vector PWM duty cycles from (α, β)._ 
```C++
void syn_foc_svpwm (
    const SYN_FOC_AB * ab,
    q16_t v_bus,
    q16_t * duty_a,
    q16_t * duty_b,
    q16_t * duty_c
) 
```



Maps (α, β) voltages to three-phase PWM duty cycles in the range [0, Q16\_ONE]. Uses standard 7-segment SVPWM pattern.




**Parameters:**


* `ab` Input (α, β) voltage commands (Q16). 
* `v_bus` DC bus voltage in Q16 (for normalization). 
* `duty_a` Output duty cycle for phase A [0, Q16\_ONE]. 
* `duty_b` Output duty cycle for phase B [0, Q16\_ONE]. 
* `duty_c` Output duty cycle for phase C [0, Q16\_ONE]. 




        

<hr>
## Public Static Functions Documentation




### function dt\_sign\_scale 

_Compute sign-adjusted dead-time duty cycle offset with zero-crossing interpolation._ 
```C++
static inline q16_t dt_sign_scale (
    q16_t current,
    q16_t dt_comp
) 
```





**Parameters:**


* `current` Measured phase current (Q16.16). 
* `dt_comp` Base dead-time compensation offset (Q16.16). 



**Returns:**

Scaled compensation offset (Q16.16). 





        

<hr>
## Macro Definition Documentation





### define Q16\_2\_INV\_SQRT3 

_Precomputed 2/√3 in Q16.16 (≈ 1.15470)._ 
```C++
#define Q16_2_INV_SQRT3 `75674`
```




<hr>



### define Q16\_INV\_SQRT3 

_Precomputed 1/√3 in Q16.16 (≈ 0.57735)._ 
```C++
#define Q16_INV_SQRT3 `37837`
```




<hr>



### define Q16\_SQRT3 

_Precomputed √3 in Q16.16 (≈ 1.73205)._ 
```C++
#define Q16_SQRT3 `113512`
```




<hr>



### define Q16\_SQRT3\_OVER\_2 

_Precomputed √3/2 in Q16.16 (≈ 0.86603)._ 
```C++
#define Q16_SQRT3_OVER_2 `56756`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_foc.c`

