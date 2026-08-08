

# File syn\_dsp.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_dsp.c**](syn__dsp_8c.md)

[Go to the source code of this file](syn__dsp_8c_source.md)

_Fixed-Point DSP implementation for Discrete Cosine Transform._ 

* `#include "syn_dsp.h"`
* `#include <math.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dsp\_dct2\_q7**](#function-syn_dsp_dct2_q7) (const [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* time\_series, size\_t num\_samples, [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) \* dct\_coeffs, size\_t num\_coeffs) <br>_Compute Type-II Discrete Cosine Transform (DCT-II) on Q7 data._  |




























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
The documentation for this class was generated from the following file `src/syntropic/util/syn_dsp.c`

