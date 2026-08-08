

# Struct SYN\_BiquadCoeffs



[**ClassList**](annotated.md) **>** [**SYN\_BiquadCoeffs**](structSYN__BiquadCoeffs.md)



_Normalized Biquad filter coefficients (where a0 = 1.0)._ 

* `#include <syn_filter_design.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**a1**](#variable-a1)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**a2**](#variable-a2)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**b0**](#variable-b0)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**b1**](#variable-b1)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**b2**](#variable-b2)  <br> |












































## Public Attributes Documentation




### variable a1 

```C++
q16_t SYN_BiquadCoeffs::a1;
```



Denominator y[n-1] coefficient 


        

<hr>



### variable a2 

```C++
q16_t SYN_BiquadCoeffs::a2;
```



Denominator y[n-2] coefficient 


        

<hr>



### variable b0 

```C++
q16_t SYN_BiquadCoeffs::b0;
```



Numerator x[n] coefficient 
 


        

<hr>



### variable b1 

```C++
q16_t SYN_BiquadCoeffs::b1;
```



Numerator x[n-1] coefficient 
 


        

<hr>



### variable b2 

```C++
q16_t SYN_BiquadCoeffs::b2;
```



Numerator x[n-2] coefficient 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_filter_design.h`

