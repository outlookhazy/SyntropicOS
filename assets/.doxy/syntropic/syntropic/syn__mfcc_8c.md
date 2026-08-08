

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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_mfcc.c`

