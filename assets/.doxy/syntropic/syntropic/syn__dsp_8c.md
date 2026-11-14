

# File syn\_dsp.c



[**FileList**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_dsp.c**](syn__dsp_8c.md)

[Go to the source code of this file](syn__dsp_8c_source.md)

_Fixed-Point DSP implementation for Discrete Cosine Transform._ 

* `#include "syn_dsp.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dsp\_dct2\_q7**](#function-syn_dsp_dct2_q7) (const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* time\_series, size\_t num\_samples, [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* dct\_coeffs, size\_t num\_coeffs) <br>_Compute Type-II Discrete Cosine Transform (DCT-II) on Q7 data._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) | [**dsp\_q16\_to\_q7\_trunc**](#function-dsp_q16_to_q7_trunc) (int64\_t val\_q16) <br>_Scale Q16.16 value to Q7 with truncation toward zero and signed 8-bit clamping._  |


























## Public Functions Documentation




### function syn\_dsp\_dct2\_q7 

_Compute Type-II Discrete Cosine Transform (DCT-II) on Q7 data._ 
```C++
SYN_Status syn_dsp_dct2_q7 (
    const q7_t * time_series,
    size_t num_samples,
    q7_t * dct_coeffs,
    size_t num_coeffs
) 
```



Transforms time-domain sensor samples into frequency-domain coefficients. Formula: X\_k = Sum\_{n=0}^{N-1} x\_n \* cos(pi/N \* (n + 0.5) \* k)




**Parameters:**


* `time_series` Pointer to input time-domain sample vector (length = num\_samples). 
* `num_samples` Number of time-domain input samples. 
* `dct_coeffs` Destination vector for frequency coefficients (length = num\_coeffs). 
* `num_coeffs` Number of output frequency coefficients to retain (num\_coeffs &lt;= num\_samples). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on failure. 





        

<hr>
## Public Static Functions Documentation




### function dsp\_q16\_to\_q7\_trunc 

_Scale Q16.16 value to Q7 with truncation toward zero and signed 8-bit clamping._ 
```C++
static q7_t dsp_q16_to_q7_trunc (
    int64_t val_q16
) 
```





**Parameters:**


* `val_q16` Input Q16.16 fixed-point value. 



**Returns:**

Clamped Q7 output sample. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_dsp.c`

