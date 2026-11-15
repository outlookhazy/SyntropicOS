

# File syn\_mfcc.h



[**FileList**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_mfcc.h**](syn__mfcc_8h.md)

[Go to the source code of this file](syn__mfcc_8h_source.md)

_Zero-Heap Fixed-Point Mel-Frequency Cepstral Coefficients (MFCC) Extractor._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/dsp/syn_dsp.h"`
* `#include "syntropic/dsp/syn_fft.h"`
* `#include "syntropic/util/syn_qmath.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_MFCC**](structSYN__MFCC.md) <br> |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mfcc\_init**](#function-syn_mfcc_init) ([**SYN\_MFCC**](structSYN__MFCC.md) \* mfcc, uint16\_t sample\_rate\_hz, uint16\_t fft\_size) <br>_Initialize MFCC Extractor Context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mfcc\_process\_frame**](#function-syn_mfcc_process_frame) ([**SYN\_MFCC**](structSYN__MFCC.md) \* mfcc, const int16\_t \* pcm\_in, [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) mfcc\_out) <br>_Process one window of 16-bit PCM audio samples and compute 13 Q7 MFCC coefficients._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_MFCC\_MAX\_FFT\_SIZE**](syn__mfcc_8h.md#define-syn_mfcc_max_fft_size)  `128U`<br> |
| define  | [**SYN\_MFCC\_NUM\_COEFFS**](syn__mfcc_8h.md#define-syn_mfcc_num_coeffs)  `13U`<br> |
| define  | [**SYN\_MFCC\_NUM\_MEL\_FILTERS**](syn__mfcc_8h.md#define-syn_mfcc_num_mel_filters)  `26U`<br> |

## Detailed Description


Extracts 13 Mel-Frequency Cepstral Coefficients (MFCC) from raw 16-bit PCM audio using fixed-point Q15 arithmetic for microcontrollers running keyword spotting (KWS) and acoustic anomaly detection with syn\_nn. 


    
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
## Macro Definition Documentation





### define SYN\_MFCC\_MAX\_FFT\_SIZE 

```C++
#define SYN_MFCC_MAX_FFT_SIZE `128U`
```



Maximum supported FFT length (128 samples) 


        

<hr>



### define SYN\_MFCC\_NUM\_COEFFS 

```C++
#define SYN_MFCC_NUM_COEFFS `13U`
```



Retained MFCC cepstral coefficient count (13) 


        

<hr>



### define SYN\_MFCC\_NUM\_MEL\_FILTERS 

```C++
#define SYN_MFCC_NUM_MEL_FILTERS `26U`
```



Standard Mel filterbank count (26 filters) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_mfcc.h`

