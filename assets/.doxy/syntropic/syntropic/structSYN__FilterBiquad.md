

# Struct SYN\_FilterBiquad



[**ClassList**](annotated.md) **>** [**SYN\_FilterBiquad**](structSYN__FilterBiquad.md)



_Fixed-point Q16.16 Biquad filter state (Direct Form I)._ 

* `#include <syn_biquad.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**a1**](#variable-a1)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**a2**](#variable-a2)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**b0**](#variable-b0)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**b1**](#variable-b1)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**b2**](#variable-b2)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**x1**](#variable-x1)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**x2**](#variable-x2)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**y1**](#variable-y1)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**y2**](#variable-y2)  <br> |












































## Public Attributes Documentation




### variable a1 

```C++
q16_t SYN_FilterBiquad::a1;
```



Feedback coefficient a1 (Q16.16, a0 assumed 1.0) 


        

<hr>



### variable a2 

```C++
q16_t SYN_FilterBiquad::a2;
```



Feedback coefficient a2 (Q16.16) 
 


        

<hr>



### variable b0 

```C++
q16_t SYN_FilterBiquad::b0;
```



Feedforward coefficient b0 (Q16.16) 
 


        

<hr>



### variable b1 

```C++
q16_t SYN_FilterBiquad::b1;
```



Feedforward coefficient b1 (Q16.16) 
 


        

<hr>



### variable b2 

```C++
q16_t SYN_FilterBiquad::b2;
```



Feedforward coefficient b2 (Q16.16) 
 


        

<hr>



### variable x1 

```C++
q16_t SYN_FilterBiquad::x1;
```



Input delay line x[n-1] 
 


        

<hr>



### variable x2 

```C++
q16_t SYN_FilterBiquad::x2;
```



Input delay line x[n-2] 
 


        

<hr>



### variable y1 

```C++
q16_t SYN_FilterBiquad::y1;
```



Output delay line y[n-1] 
 


        

<hr>



### variable y2 

```C++
q16_t SYN_FilterBiquad::y2;
```



Output delay line y[n-2] 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_biquad.h`

