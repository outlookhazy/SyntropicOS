

# File syn\_dsp.h



[**FileList**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_dsp.h**](syn__dsp_8h.md)

[Go to the source code of this file](syn__dsp_8h_source.md)

_Zero-heap Fixed-Point DSP & Spectral Processing Primitives._ [More...](#detailed-description)

* `#include "../util/syn_qmath.h"`
* `#include "syntropic/common/syn_defs.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dsp\_dct2\_q7**](#function-syn_dsp_dct2_q7) (const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* time\_series, size\_t num\_samples, [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* dct\_coeffs, size\_t num\_coeffs) <br>_Compute Type-II Discrete Cosine Transform (DCT-II) on Q7 data._  |




























## Detailed Description


Provides fixed-point Discrete Cosine Transform (DCT-II) and signal processing helpers tailored for resource-constrained microcontrollers. 


    
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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_dsp.h`

