

# File syn\_biquad.c



[**FileList**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_biquad.c**](syn__biquad_8c.md)

[Go to the source code of this file](syn__biquad_8c_source.md)

_Fixed-point Q16.16 Biquad filter implementation._ 

* `#include "syn_biquad.h"`
* `#include "../util/syn_assert.h"`
* `#include <stddef.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_filter\_biquad\_bandpass**](#function-syn_filter_biquad_bandpass) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fc, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fs, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) q) <br>_Initialize a biquad bandpass filter._  |
|  void | [**syn\_filter\_biquad\_highpass**](#function-syn_filter_biquad_highpass) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fc, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fs) <br>_Initialize a biquad highpass filter._  |
|  void | [**syn\_filter\_biquad\_init**](#function-syn_filter_biquad_init) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b0, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b1, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b2, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a1, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a2) <br>_Initialize a biquad filter with raw coefficients._  |
|  void | [**syn\_filter\_biquad\_lowpass**](#function-syn_filter_biquad_lowpass) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fc, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fs) <br>_Initialize a biquad lowpass filter._  |
|  void | [**syn\_filter\_biquad\_notch**](#function-syn_filter_biquad_notch) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fc, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fs, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) q) <br>_Initialize a biquad notch (band-reject) filter._  |
|  void | [**syn\_filter\_biquad\_reset**](#function-syn_filter_biquad_reset) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f) <br>_Reset biquad filter delay lines to zero._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**syn\_filter\_biquad\_update**](#function-syn_filter_biquad_update) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) sample) <br>_Process a single sample through the biquad filter._  |




























## Public Functions Documentation




### function syn\_filter\_biquad\_bandpass 

_Initialize a biquad bandpass filter._ 
```C++
void syn_filter_biquad_bandpass (
    SYN_FilterBiquad * f,
    q16_t fc,
    q16_t fs,
    q16_t q
) 
```



Constant-skirt-gain bandpass. Peak gain = 1.0 at center frequency.




**Parameters:**


* `f` Filter instance. 
* `fc` Center frequency (Hz) in Q16.16. 
* `fs` Sample rate (Hz) in Q16.16. 
* `q` Quality factor in Q16.16 (higher = narrower band). 




        

<hr>



### function syn\_filter\_biquad\_highpass 

_Initialize a biquad highpass filter._ 
```C++
void syn_filter_biquad_highpass (
    SYN_FilterBiquad * f,
    q16_t fc,
    q16_t fs
) 
```



Computes standard Butterworth highpass coefficients in Q16.16.




**Parameters:**


* `f` Filter instance. 
* `fc` Cutoff frequency (Hz) in Q16.16. 
* `fs` Sample rate (Hz) in Q16.16. 




        

<hr>



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



### function syn\_filter\_biquad\_notch 

_Initialize a biquad notch (band-reject) filter._ 
```C++
void syn_filter_biquad_notch (
    SYN_FilterBiquad * f,
    q16_t fc,
    q16_t fs,
    q16_t q
) 
```



Rejects frequencies near fc, passes all others.




**Parameters:**


* `f` Filter instance. 
* `fc` Notch center frequency (Hz) in Q16.16. 
* `fs` Sample rate (Hz) in Q16.16. 
* `q` Quality factor in Q16.16 (higher = narrower notch). 




        

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
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_biquad.c`

