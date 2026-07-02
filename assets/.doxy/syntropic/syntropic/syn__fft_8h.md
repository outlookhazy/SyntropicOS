

# File syn\_fft.h



[**FileList**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_fft.h**](syn__fft_8h.md)

[Go to the source code of this file](syn__fft_8h_source.md)

_Fixed-point Q16.16 Fast Fourier Transform (FFT)._ 

* `#include "../common/syn_defs.h"`
* `#include "../util/syn_qmath.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dsp\_fft**](#function-syn_dsp_fft) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* real, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* imag, uint16\_t n) <br>_Computes the in-place Radix-2 Decimation-in-Time FFT._  |




























## Public Functions Documentation




### function syn\_dsp\_fft 

_Computes the in-place Radix-2 Decimation-in-Time FFT._ 
```C++
SYN_Status syn_dsp_fft (
    q16_t * real,
    q16_t * imag,
    uint16_t n
) 
```



n must be a power of 2 (e.g., 8, 16, 32, 64, 128, 256). Max supported size is 256.




**Parameters:**


* `real` Array of real parts (size n). 
* `imag` Array of imaginary parts (size n). 
* `n` Size of the FFT (must be power of 2). 



**Returns:**

SYN\_OK on success, SYN\_ERROR on invalid parameters (e.g. not power of 2). 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_fft.h`

