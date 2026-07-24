

# File syn\_foc.h



[**FileList**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_foc.h**](syn__foc_8h.md)

[Go to the source code of this file](syn__foc_8h_source.md)

_Fixed-point Field-Oriented Control transforms (Clarke & Park)._ [More...](#detailed-description)

* `#include "../util/syn_qmath.h"`
* `#include "../common/syn_defs.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_FOC\_AB**](structSYN__FOC__AB.md) <br>_2-phase stationary frame (α, β)._  |
| struct | [**SYN\_FOC\_ABC**](structSYN__FOC__ABC.md) <br>_3-phase values (a, b, c)._  |
| struct | [**SYN\_FOC\_DQ**](structSYN__FOC__DQ.md) <br>_2-phase rotating frame (d, q)._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_foc\_clarke**](#function-syn_foc_clarke) (const [**SYN\_FOC\_ABC**](structSYN__FOC__ABC.md) \* abc, [**SYN\_FOC\_AB**](structSYN__FOC__AB.md) \* ab) <br>_Clarke transform: 3-phase (a,b,c) → 2-phase (α,β)._  |
|  void | [**syn\_foc\_inv\_clarke**](#function-syn_foc_inv_clarke) (const [**SYN\_FOC\_AB**](structSYN__FOC__AB.md) \* ab, [**SYN\_FOC\_ABC**](structSYN__FOC__ABC.md) \* abc) <br>_Inverse Clarke: 2-phase (α,β) → 3-phase (a,b,c)._  |
|  void | [**syn\_foc\_inv\_park**](#function-syn_foc_inv_park) (const [**SYN\_FOC\_DQ**](structSYN__FOC__DQ.md) \* dq, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) theta, [**SYN\_FOC\_AB**](structSYN__FOC__AB.md) \* ab) <br>_Inverse Park: rotating (d,q) → stationary (α,β)._  |
|  void | [**syn\_foc\_park**](#function-syn_foc_park) (const [**SYN\_FOC\_AB**](structSYN__FOC__AB.md) \* ab, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) theta, [**SYN\_FOC\_DQ**](structSYN__FOC__DQ.md) \* dq) <br>_Park transform: stationary (α,β) → rotating (d,q)._  |
|  void | [**syn\_foc\_svpwm**](#function-syn_foc_svpwm) (const [**SYN\_FOC\_AB**](structSYN__FOC__AB.md) \* ab, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) v\_bus, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* duty\_a, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* duty\_b, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* duty\_c) <br>_Compute Space Vector PWM duty cycles from (α, β)._  |




























## Detailed Description


Provides the mathematical transforms needed for BLDC/PMSM motor control:



* **Clarke transform**: 3-phase (a, b, c) → 2-phase stationary (α, β)
* **Inverse Clarke**: (α, β) → (a, b, c)
* **Park transform**: Stationary (α, β) → rotating (d, q) using rotor angle θ
* **Inverse Park**: Rotating (d, q) → stationary (α, β)




All values are Q16.16 fixed-point. No floating point, no heap allocation.




**
**


```C++
SYN_FOC_AB  ab;
SYN_FOC_DQ  dq;
SYN_FOC_ABC phase_currents = { ia, ib, ic };  // measured currents
q16_t theta = electrical_angle;                // rotor angle

// Forward: ABC → αβ → dq
syn_foc_clarke(&phase_currents, &ab);
syn_foc_park(&ab, theta, &dq);

// Now dq.d and dq.q are the field-aligned currents.
// Run PID on dq, then inverse transform for PWM:
syn_foc_inv_park(&dq_cmd, theta, &ab);
syn_foc_inv_clarke(&ab, &phase_voltages);
```
 





    
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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_foc.h`

