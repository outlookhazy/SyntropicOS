

# File syn\_biquad.h



[**FileList**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_biquad.h**](syn__biquad_8h.md)

[Go to the source code of this file](syn__biquad_8h_source.md)

_Fixed-point Q16.16 Biquad filter (Direct Form I)._ 

* `#include "../util/syn_qmath.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) <br>_Fixed-point Q16.16 Biquad filter state (Direct Form I)._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_filter\_biquad\_init**](#function-syn_filter_biquad_init) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b0, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b1, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b2, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a1, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a2) <br>_Initialize a biquad filter with raw coefficients._  |
|  void | [**syn\_filter\_biquad\_lowpass**](#function-syn_filter_biquad_lowpass) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fc, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fs) <br>_Initialize a biquad lowpass filter._  |
|  void | [**syn\_filter\_biquad\_reset**](#function-syn_filter_biquad_reset) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f) <br>_Reset biquad filter delay lines to zero._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**syn\_filter\_biquad\_update**](#function-syn_filter_biquad_update) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) sample) <br>_Process a single sample through the biquad filter._  |




























## Public Functions Documentation




### function syn\_filter\_biquad\_init 

_Initialize a biquad filter with raw coefficients._ 
```C++
void syn_filter_biquad_init (
    SYN_FilterBiquad * f,
    q16_t b0,
    q16_t b1,
    q16_t b2,
    q16_t a1,
    q16_t a2
) 
```





**Parameters:**


* `f` Filter instance. 
* `b0` Feedforward coefficient b0 (Q16.16). 
* `b1` Feedforward coefficient b1 (Q16.16). 
* `b2` Feedforward coefficient b2 (Q16.16). 
* `a1` Feedback coefficient a1 (Q16.16). 
* `a2` Feedback coefficient a2 (Q16.16). 




        

<hr>



### function syn\_filter\_biquad\_lowpass 

_Initialize a biquad lowpass filter._ 
```C++
void syn_filter_biquad_lowpass (
    SYN_FilterBiquad * f,
    q16_t fc,
    q16_t fs
) 
```



Computes standard Butterworth coefficients in Q16.16.




**Parameters:**


* `f` Filter instance. 
* `fc` Cutoff frequency (Hz) in Q16.16. 
* `fs` Sample rate (Hz) in Q16.16. 




        

<hr>



### function syn\_filter\_biquad\_reset 

_Reset biquad filter delay lines to zero._ 
```C++
void syn_filter_biquad_reset (
    SYN_FilterBiquad * f
) 
```





**Parameters:**


* `f` Filter instance. 




        

<hr>



### function syn\_filter\_biquad\_update 

_Process a single sample through the biquad filter._ 
```C++
q16_t syn_filter_biquad_update (
    SYN_FilterBiquad * f,
    q16_t sample
) 
```



Uses 64-bit intermediate calculations to prevent overflows.




**Parameters:**


* `f` Biquad filter instance. 
* `sample` Input sample in Q16.16. 



**Returns:**

Filtered output in Q16.16. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_biquad.h`

