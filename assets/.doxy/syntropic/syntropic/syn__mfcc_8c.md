

# File syn\_mfcc.c



[**FileList**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_mfcc.c**](syn__mfcc_8c.md)

[Go to the source code of this file](syn__mfcc_8c_source.md)

_Zero-Heap Fixed-Point Mel-Frequency Cepstral Coefficients (MFCC) Implementation._ 

* `#include "syn_mfcc.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mfcc\_init**](#function-syn_mfcc_init) ([**SYN\_MFCC**](structSYN__MFCC.md) \* mfcc, uint16\_t sample\_rate\_hz, uint16\_t fft\_size) <br>_Initialize MFCC Extractor Context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mfcc\_process\_frame**](#function-syn_mfcc_process_frame) ([**SYN\_MFCC**](structSYN__MFCC.md) \* mfcc, const int16\_t \* pcm\_in, [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) mfcc\_out) <br>_Process one window of 16-bit PCM audio samples and compute 13 Q7 MFCC coefficients._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**mfcc\_compute\_mel\_edges**](#function-mfcc_compute_mel_edges) (uint16\_t sample\_rate\_hz, uint16\_t fft\_size, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) edges) <br>_Compute Mel-warped FFT bin edges in Q16.16 across [0, sample\_rate\_hz / 2]._  |
|  uint64\_t | [**mfcc\_integrate\_band**](#function-mfcc_integrate_band) (const [**SYN\_MFCC**](structSYN__MFCC.md) \* mfcc, uint16\_t num\_bins, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) left, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) center, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) right) <br>_Integrate triangular Mel filter energy over one filter band._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**mfcc\_log2\_u64\_q16**](#function-mfcc_log2_u64_q16) (uint64\_t x) <br>_Compute fixed-point base-2 logarithm log2(x) in Q16.16 for x &gt;= 1._  |


























## Public Functions Documentation




### function syn\_mfcc\_init 

_Initialize MFCC Extractor Context._ 
```C++
SYN_Status syn_mfcc_init (
    SYN_MFCC * mfcc,
    uint16_t sample_rate_hz,
    uint16_t fft_size
) 
```





**Parameters:**


* `mfcc` Pointer to MFCC context. 
* `sample_rate_hz` Audio sample rate in Hz. 
* `fft_size` FFT window size (must be 64 or 128). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if invalid. 





        

<hr>



### function syn\_mfcc\_process\_frame 

_Process one window of 16-bit PCM audio samples and compute 13 Q7 MFCC coefficients._ 
```C++
SYN_Status syn_mfcc_process_frame (
    SYN_MFCC * mfcc,
    const int16_t * pcm_in,
    q7_t mfcc_out
) 
```





**Parameters:**


* `mfcc` Pointer to MFCC context. 
* `pcm_in` Array of 16-bit PCM input samples (length = fft\_size). 
* `mfcc_out` Output buffer for 13 Q7 MFCC coefficients. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL. 





        

<hr>
## Public Static Functions Documentation




### function mfcc\_compute\_mel\_edges 

_Compute Mel-warped FFT bin edges in Q16.16 across [0, sample\_rate\_hz / 2]._ 
```C++
static void mfcc_compute_mel_edges (
    uint16_t sample_rate_hz,
    uint16_t fft_size,
    q16_t edges
) 
```





**Parameters:**


* `sample_rate_hz` Audio sample rate in Hz. 
* `fft_size` FFT length N. 
* `edges` Output array of (SYN\_MFCC\_NUM\_MEL\_FILTERS + 2) Q16.16 bin coordinates. 




        

<hr>



### function mfcc\_integrate\_band 

_Integrate triangular Mel filter energy over one filter band._ 
```C++
static uint64_t mfcc_integrate_band (
    const SYN_MFCC * mfcc,
    uint16_t num_bins,
    q16_t left,
    q16_t center,
    q16_t right
) 
```





**Parameters:**


* `mfcc` MFCC context containing FFT spectrum. 
* `num_bins` Half-spectrum bin count (N / 2). 
* `left` Left Mel edge in Q16.16 bins. 
* `center` Center Mel peak in Q16.16 bins. 
* `right` Right Mel edge in Q16.16 bins. 



**Returns:**

Weighted power sum in Q16.16. 





        

<hr>



### function mfcc\_log2\_u64\_q16 

_Compute fixed-point base-2 logarithm log2(x) in Q16.16 for x &gt;= 1._ 
```C++
static q16_t mfcc_log2_u64_q16 (
    uint64_t x
) 
```





**Parameters:**


* `x` Positive 64-bit integer input (&gt;= 1). 



**Returns:**

log2(x) in Q16.16 format. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_mfcc.c`

