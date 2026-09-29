

# File syn\_filter\_design.c



[**FileList**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_filter\_design.c**](syn__filter__design_8c.md)

[Go to the source code of this file](syn__filter__design_8c_source.md)

_Dynamic runtime IIR Biquad filter coefficient generator implementation._ 

* `#include "syn_filter_design.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_filter\_design\_apply\_to\_biquad**](#function-syn_filter_design_apply_to_biquad) (const [**SYN\_BiquadCoeffs**](structSYN__BiquadCoeffs.md) \* coeffs, [**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) \* biquad) <br>_Apply calculated coefficients to a_ [_**SYN\_FilterBiquad**_](structSYN__FilterBiquad.md) _instance._ |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_filter\_design\_bandpass**](#function-syn_filter_design_bandpass) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fc\_hz, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fs\_hz, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) Q, [**SYN\_BiquadCoeffs**](structSYN__BiquadCoeffs.md) \* out) <br>_Calculate Bandpass filter coefficients (constant skirt gain)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_filter\_design\_highpass**](#function-syn_filter_design_highpass) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fc\_hz, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fs\_hz, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) Q, [**SYN\_BiquadCoeffs**](structSYN__BiquadCoeffs.md) \* out) <br>_Calculate Butterworth Highpass filter coefficients._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_filter\_design\_lowpass**](#function-syn_filter_design_lowpass) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fc\_hz, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fs\_hz, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) Q, [**SYN\_BiquadCoeffs**](structSYN__BiquadCoeffs.md) \* out) <br>_Calculate Butterworth Lowpass filter coefficients._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_filter\_design\_notch**](#function-syn_filter_design_notch) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fc\_hz, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fs\_hz, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) Q, [**SYN\_BiquadCoeffs**](structSYN__BiquadCoeffs.md) \* out) <br>_Calculate Notch / Bandstop filter coefficients._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**compute\_intermediate\_vars**](#function-compute_intermediate_vars) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fc\_hz, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) fs\_hz, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) Q, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* cos\_w0, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* sin\_w0, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* alpha, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* a0\_inv) <br>_Helper to pre-warp frequency and compute intermediate bilinear transform constants._  |


























## Public Functions Documentation




### function syn\_filter\_design\_apply\_to\_biquad 

_Apply calculated coefficients to a_ [_**SYN\_FilterBiquad**_](structSYN__FilterBiquad.md) _instance._
```C++
SYN_Status syn_filter_design_apply_to_biquad (
    const SYN_BiquadCoeffs * coeffs,
    SYN_FilterBiquad * biquad
) 
```





**Parameters:**


* `coeffs` Calculated biquad coefficients. 
* `biquad` Target biquad filter instance. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL. 





        

<hr>



### function syn\_filter\_design\_bandpass 

_Calculate Bandpass filter coefficients (constant skirt gain)._ 
```C++
SYN_Status syn_filter_design_bandpass (
    q16_t fc_hz,
    q16_t fs_hz,
    q16_t Q,
    SYN_BiquadCoeffs * out
) 
```





**Parameters:**


* `fc_hz` Center frequency in Hz (Q16.16). 
* `fs_hz` Sampling frequency in Hz (Q16.16). 
* `Q` Quality factor / bandwidth selectivity (Q16.16, e.g., Q16\_FROM\_FLOAT(2.0)). 
* `out` Pointer to output coefficient structure. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if invalid. 





        

<hr>



### function syn\_filter\_design\_highpass 

_Calculate Butterworth Highpass filter coefficients._ 
```C++
SYN_Status syn_filter_design_highpass (
    q16_t fc_hz,
    q16_t fs_hz,
    q16_t Q,
    SYN_BiquadCoeffs * out
) 
```





**Parameters:**


* `fc_hz` Cutoff frequency in Hz (Q16.16). 
* `fs_hz` Sampling frequency in Hz (Q16.16). 
* `Q` Quality factor (Q16.16, e.g., Q16\_FROM\_FLOAT(0.7071) for Butterworth). 
* `out` Pointer to output coefficient structure. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if invalid. 





        

<hr>



### function syn\_filter\_design\_lowpass 

_Calculate Butterworth Lowpass filter coefficients._ 
```C++
SYN_Status syn_filter_design_lowpass (
    q16_t fc_hz,
    q16_t fs_hz,
    q16_t Q,
    SYN_BiquadCoeffs * out
) 
```





**Parameters:**


* `fc_hz` Cutoff frequency in Hz (Q16.16). 
* `fs_hz` Sampling frequency in Hz (Q16.16). 
* `Q` Quality factor (Q16.16, e.g., Q16\_FROM\_FLOAT(0.7071) for Butterworth). 
* `out` Pointer to output coefficient structure. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if invalid. 





        

<hr>



### function syn\_filter\_design\_notch 

_Calculate Notch / Bandstop filter coefficients._ 
```C++
SYN_Status syn_filter_design_notch (
    q16_t fc_hz,
    q16_t fs_hz,
    q16_t Q,
    SYN_BiquadCoeffs * out
) 
```





**Parameters:**


* `fc_hz` Rejection frequency in Hz (Q16.16). 
* `fs_hz` Sampling frequency in Hz (Q16.16). 
* `Q` Quality factor / notch depth selectivity (Q16.16, e.g., Q16\_FROM\_FLOAT(5.0)). 
* `out` Pointer to output coefficient structure. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if invalid. 





        

<hr>
## Public Static Functions Documentation




### function compute\_intermediate\_vars 

_Helper to pre-warp frequency and compute intermediate bilinear transform constants._ 
```C++
static SYN_Status compute_intermediate_vars (
    q16_t fc_hz,
    q16_t fs_hz,
    q16_t Q,
    q16_t * cos_w0,
    q16_t * sin_w0,
    q16_t * alpha,
    q16_t * a0_inv
) 
```





**Parameters:**


* `fc_hz` Cutoff frequency in Hz. 
* `fs_hz` Sampling frequency in Hz. 
* `Q` Quality factor. 
* `cos_w0` Output cos(w0). 
* `sin_w0` Output sin(w0). 
* `alpha` Output alpha. 
* `a0_inv` Output 1/a0 scale factor. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if invalid limits. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_filter_design.c`

