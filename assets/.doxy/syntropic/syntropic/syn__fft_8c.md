

# File syn\_fft.c



[**FileList**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_fft.c**](syn__fft_8c.md)

[Go to the source code of this file](syn__fft_8c_source.md)

_Fixed-point Q16.16 Radix-2 Decimation-in-Time Fast Fourier Transform implementation._ 

* `#include "syn_fft.h"`
* `#include "../util/syn_assert.h"`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**g\_sin\_table**](#variable-g_sin_table)   = `/* multi line expression */`<br>_Quarter-sine lookup table for N=256 FFT (Q16 fixed-point)._  |














## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dsp\_fft**](#function-syn_dsp_fft) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* real, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* imag, uint16\_t n) <br>_Computes the in-place Radix-2 Decimation-in-Time FFT._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_fft\_find\_peaks**](#function-syn_fft_find_peaks) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* mag, uint16\_t n\_bins, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) sample\_rate\_hz, [**SYN\_FFTPeak**](structSYN__FFTPeak.md) \* peaks, uint8\_t max\_peaks, uint8\_t \* num\_peaks\_found) <br>_Find dominant peaks in magnitude spectrum._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_fft\_magnitude\_spectrum**](#function-syn_fft_magnitude_spectrum) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* real, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* imag, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* mag, uint16\_t n) <br>_Compute magnitude spectrum: mag[k] = sqrt(real[k]^2 + imag[k]^2)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_fft\_thd**](#function-syn_fft_thd) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* mag, uint16\_t n\_bins, uint16\_t fundamental\_bin, uint8\_t max\_harmonics, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* thd\_pct) <br>_Compute Total Harmonic Distortion (THD) percentage._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_fft\_window\_blackman**](#function-syn_fft_window_blackman) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out, uint16\_t n) <br>_Generate Blackman-Harris window: w[i] = 0.42 - 0.5\*cos(2\*pi\*i/(n-1)) + 0.08\*cos(4\*pi\*i/(n-1))._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_fft\_window\_hamming**](#function-syn_fft_window_hamming) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out, uint16\_t n) <br>_Generate Hamming window: w[i] = 0.54 - 0.46 \* cos(2\*pi\*i / (n-1))._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_fft\_window\_hanning**](#function-syn_fft_window_hanning) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out, uint16\_t n) <br>_Generate Hanning window: w[i] = 0.5 \* (1 - cos(2\*pi\*i / (n-1)))._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**bit\_reverse\_sort**](#function-bit_reverse_sort) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* real, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* imag, uint16\_t n, uint16\_t stages) <br>_Reorder samples by bit-reversed index._  |
|  void | [**get\_twiddle**](#function-get_twiddle) (uint16\_t k, uint16\_t stage, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* wr, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* wi) <br>_Compute twiddle factor (cos, -sin) for a butterfly stage._  |
|  uint16\_t | [**reverse\_bits**](#function-reverse_bits) (uint16\_t x, uint16\_t stages) <br>_Reverse the low bits of a value._  |


























## Public Static Attributes Documentation




### variable g\_sin\_table 

_Quarter-sine lookup table for N=256 FFT (Q16 fixed-point)._ 
```C++
const q16_t g_sin_table[65];
```




<hr>
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



### function syn\_fft\_find\_peaks 

_Find dominant peaks in magnitude spectrum._ 
```C++
SYN_Status syn_fft_find_peaks (
    const q16_t * mag,
    uint16_t n_bins,
    q16_t sample_rate_hz,
    SYN_FFTPeak * peaks,
    uint8_t max_peaks,
    uint8_t * num_peaks_found
) 
```





**Parameters:**


* `mag` Magnitude array (size n\_bins). 
* `n_bins` Number of bins (typically n/2 + 1). 
* `sample_rate_hz` Sampling rate in Hz (Q16.16). 
* `peaks` Output peak array. 
* `max_peaks` Capacity of peaks array. 
* `num_peaks_found` Output count of detected peaks. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid inputs. 





        

<hr>



### function syn\_fft\_magnitude\_spectrum 

_Compute magnitude spectrum: mag[k] = sqrt(real[k]^2 + imag[k]^2)._ 
```C++
SYN_Status syn_fft_magnitude_spectrum (
    const q16_t * real,
    const q16_t * imag,
    q16_t * mag,
    uint16_t n
) 
```





**Parameters:**


* `real` Real part array (size n). 
* `imag` Imaginary part array (size n). 
* `mag` Output magnitude array (size n/2 + 1). 
* `n` FFT size (number of points). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid inputs. 





        

<hr>



### function syn\_fft\_thd 

_Compute Total Harmonic Distortion (THD) percentage._ 
```C++
SYN_Status syn_fft_thd (
    const q16_t * mag,
    uint16_t n_bins,
    uint16_t fundamental_bin,
    uint8_t max_harmonics,
    q16_t * thd_pct
) 
```



THD = sqrt(sum(V\_h^2)) / V\_1 \* 100%




**Parameters:**


* `mag` Magnitude array (size n\_bins). 
* `n_bins` Number of bins. 
* `fundamental_bin` Bin index of the fundamental frequency. 
* `max_harmonics` Number of harmonics to sum (e.g. 5). 
* `thd_pct` Output THD percentage in Q16.16 (e.g., 5.2% = Q16\_FROM\_FLOAT(5.2)). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid inputs. 





        

<hr>



### function syn\_fft\_window\_blackman 

_Generate Blackman-Harris window: w[i] = 0.42 - 0.5\*cos(2\*pi\*i/(n-1)) + 0.08\*cos(4\*pi\*i/(n-1))._ 
```C++
SYN_Status syn_fft_window_blackman (
    q16_t * out,
    uint16_t n
) 
```





**Parameters:**


* `out` Output buffer of size n. 
* `n` Window length. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL or n &lt;= 1. 





        

<hr>



### function syn\_fft\_window\_hamming 

_Generate Hamming window: w[i] = 0.54 - 0.46 \* cos(2\*pi\*i / (n-1))._ 
```C++
SYN_Status syn_fft_window_hamming (
    q16_t * out,
    uint16_t n
) 
```





**Parameters:**


* `out` Output buffer of size n. 
* `n` Window length. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL or n &lt;= 1. 





        

<hr>



### function syn\_fft\_window\_hanning 

_Generate Hanning window: w[i] = 0.5 \* (1 - cos(2\*pi\*i / (n-1)))._ 
```C++
SYN_Status syn_fft_window_hanning (
    q16_t * out,
    uint16_t n
) 
```





**Parameters:**


* `out` Output buffer of size n. 
* `n` Window length. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL or n &lt;= 1. 





        

<hr>
## Public Static Functions Documentation




### function bit\_reverse\_sort 

_Reorder samples by bit-reversed index._ 
```C++
static void bit_reverse_sort (
    q16_t * real,
    q16_t * imag,
    uint16_t n,
    uint16_t stages
) 
```





**Parameters:**


* `real` Real components. 
* `imag` Imaginary components. 
* `n` Number of samples. 
* `stages` log2(n). 




        

<hr>



### function get\_twiddle 

_Compute twiddle factor (cos, -sin) for a butterfly stage._ 
```C++
static void get_twiddle (
    uint16_t k,
    uint16_t stage,
    q16_t * wr,
    q16_t * wi
) 
```





**Parameters:**


* `k` Butterfly index within group (0 &lt;= k &lt; m/2). 
* `stage` Current FFT stage (1-based). 
* `wr` [out] Twiddle real (cosine). 
* `wi` [out] Twiddle imaginary (-sine).

The Cooley-Tukey butterfly calls this with k &lt; 2^(stage-1), so idx = (256\*k) &gt;&gt; stage is always &lt; 128. Only quadrants 0 and 1 of the sine table are reachable. 


        

<hr>



### function reverse\_bits 

_Reverse the low bits of a value._ 
```C++
static uint16_t reverse_bits (
    uint16_t x,
    uint16_t stages
) 
```





**Parameters:**


* `x` Value to reverse. 
* `stages` Number of bits. 



**Returns:**

Bit-reversed value. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_fft.c`

