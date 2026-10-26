

# File syn\_biquad.c



[**FileList**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_biquad.c**](syn__biquad_8c.md)

[Go to the source code of this file](syn__biquad_8c_source.md)

_Fixed-point Q16.16 Biquad filter implementation._ 

* `#include "../util/syn_assert.h"`
* `#include "syn_biquad.h"`
* `#include <stddef.h>`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_filter\_biquad\_allpass**](#function-syn_filter_biquad_allpass) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fc, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fs, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) q) <br>_Initialize a biquad allpass filter (modifies phase while preserving magnitude)._  |
|  void | [**syn\_filter\_biquad\_bandpass**](#function-syn_filter_biquad_bandpass) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fc, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fs, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) q) <br>_Initialize a biquad bandpass filter._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_filter\_biquad\_cascade\_init**](#function-syn_filter_biquad_cascade_init) ([**SYN\_FilterBiquadCascade**](structSYN__FilterBiquadCascade.md) \* c, uint8\_t num\_stages) <br>_Initialize a cascaded biquad filter structure._  |
|  void | [**syn\_filter\_biquad\_cascade\_process\_block**](#function-syn_filter_biquad_cascade_process_block) ([**SYN\_FilterBiquadCascade**](structSYN__FilterBiquadCascade.md) \* c, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* in, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out, uint16\_t count) <br>_Process a block of samples through all active biquad stages._  |
|  void | [**syn\_filter\_biquad\_cascade\_reset**](#function-syn_filter_biquad_cascade_reset) ([**SYN\_FilterBiquadCascade**](structSYN__FilterBiquadCascade.md) \* c) <br>_Reset all delay lines across all stages in the biquad cascade._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**syn\_filter\_biquad\_cascade\_update**](#function-syn_filter_biquad_cascade_update) ([**SYN\_FilterBiquadCascade**](structSYN__FilterBiquadCascade.md) \* c, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) sample) <br>_Process a single sample sequentially through all active biquad stages._  |
|  void | [**syn\_filter\_biquad\_highpass**](#function-syn_filter_biquad_highpass) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fc, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fs) <br>_Initialize a biquad highpass filter._  |
|  void | [**syn\_filter\_biquad\_init**](#function-syn_filter_biquad_init) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b0, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b1, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) b2, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a1, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) a2) <br>_Initialize a biquad filter with raw coefficients._  |
|  void | [**syn\_filter\_biquad\_lowpass**](#function-syn_filter_biquad_lowpass) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fc, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fs) <br>_Initialize a biquad lowpass filter._  |
|  void | [**syn\_filter\_biquad\_notch**](#function-syn_filter_biquad_notch) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fc, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fs, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) q) <br>_Initialize a biquad notch (band-reject) filter._  |
|  void | [**syn\_filter\_biquad\_peaking\_eq**](#function-syn_filter_biquad_peaking_eq) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fc, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fs, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) gain\_db, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) q) <br>_Initialize a biquad peaking equalizer filter (boost/cut centered at fc)._  |
|  void | [**syn\_filter\_biquad\_process\_block**](#function-syn_filter_biquad_process_block) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* in, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out, uint16\_t count) <br>_Process a block of samples through the biquad filter._  |
|  void | [**syn\_filter\_biquad\_reset**](#function-syn_filter_biquad_reset) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f) <br>_Reset biquad filter delay lines to zero._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**syn\_filter\_biquad\_update**](#function-syn_filter_biquad_update) ([**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* f, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) sample) <br>_Process a single sample through the biquad filter._  |




























## Public Functions Documentation




### function syn\_filter\_biquad\_allpass 

_Initialize a biquad allpass filter (modifies phase while preserving magnitude)._ 
```C++
void syn_filter_biquad_allpass (
    SYN_FilterBiquad * f,
    q16_t fc,
    q16_t fs,
    q16_t q
) 
```





**Parameters:**


* `f` Filter instance. 
* `fc` Corner frequency (Hz) in Q16.16. 
* `fs` Sample rate (Hz) in Q16.16. 
* `q` Quality factor in Q16.16. 




        

<hr>



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



### function syn\_filter\_biquad\_cascade\_init 

_Initialize a cascaded biquad filter structure._ 
```C++
SYN_Status syn_filter_biquad_cascade_init (
    SYN_FilterBiquadCascade * c,
    uint8_t num_stages
) 
```





**Parameters:**


* `c` Cascade instance. 
* `num_stages` Number of active biquad stages (1 to SYN\_BIQUAD\_CASCADE\_MAX\_STAGES). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_filter\_biquad\_cascade\_process\_block 

_Process a block of samples through all active biquad stages._ 
```C++
void syn_filter_biquad_cascade_process_block (
    SYN_FilterBiquadCascade * c,
    const q16_t * in,
    q16_t * out,
    uint16_t count
) 
```





**Parameters:**


* `c` Cascade instance. 
* `in` Input sample array. 
* `out` Output sample array (may alias in). 
* `count` Number of samples to process. 




        

<hr>



### function syn\_filter\_biquad\_cascade\_reset 

_Reset all delay lines across all stages in the biquad cascade._ 
```C++
void syn_filter_biquad_cascade_reset (
    SYN_FilterBiquadCascade * c
) 
```





**Parameters:**


* `c` Cascade instance. 




        

<hr>



### function syn\_filter\_biquad\_cascade\_update 

_Process a single sample sequentially through all active biquad stages._ 
```C++
q16_t syn_filter_biquad_cascade_update (
    SYN_FilterBiquadCascade * c,
    q16_t sample
) 
```





**Parameters:**


* `c` Cascade instance. 
* `sample` Input sample in Q16.16. 



**Returns:**

Filtered output in Q16.16. 





        

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



### function syn\_filter\_biquad\_peaking\_eq 

_Initialize a biquad peaking equalizer filter (boost/cut centered at fc)._ 
```C++
void syn_filter_biquad_peaking_eq (
    SYN_FilterBiquad * f,
    q16_t fc,
    q16_t fs,
    q16_t gain_db,
    q16_t q
) 
```





**Parameters:**


* `f` Filter instance. 
* `fc` Center frequency (Hz) in Q16.16. 
* `fs` Sample rate (Hz) in Q16.16. 
* `gain_db` Gain in dB (Q16.16, e.g. +6.0 for boost, -6.0 for cut). 
* `q` Quality factor in Q16.16. 




        

<hr>



### function syn\_filter\_biquad\_process\_block 

_Process a block of samples through the biquad filter._ 
```C++
void syn_filter_biquad_process_block (
    SYN_FilterBiquad * f,
    const q16_t * in,
    q16_t * out,
    uint16_t count
) 
```





**Parameters:**


* `f` Biquad filter instance. 
* `in` Input sample array (n elements). 
* `out` Output sample array (n elements). May alias in. 
* `count` Number of samples to process. 




        

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

