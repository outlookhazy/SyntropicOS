

# File syn\_goertzel.h



[**FileList**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_goertzel.h**](syn__goertzel_8h.md)

[Go to the source code of this file](syn__goertzel_8h_source.md)

_Integer-Only Goertzel Algorithm Single-Frequency Tone Detector._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Goertzel**](structSYN__Goertzel.md) <br>_Goertzel Filter Instance State._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint64\_t | [**syn\_goertzel\_get\_magnitude\_sq**](#function-syn_goertzel_get_magnitude_sq) (const [**SYN\_Goertzel**](structSYN__Goertzel.md) \* g) <br>_Calculate the squared magnitude \|X(k)\|^2 at the end of a sample block._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_goertzel\_init**](#function-syn_goertzel_init) ([**SYN\_Goertzel**](structSYN__Goertzel.md) \* g, uint32\_t sample\_rate\_hz, uint32\_t target\_freq\_hz, size\_t block\_size) <br>_Initialize a Goertzel Filter instance._  |
|  size\_t | [**syn\_goertzel\_process\_block**](#function-syn_goertzel_process_block) ([**SYN\_Goertzel**](structSYN__Goertzel.md) \* g, const int16\_t \* samples, size\_t count) <br>_Process a buffer of 16-bit PCM samples through the Goertzel filter._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_goertzel\_process\_sample**](#function-syn_goertzel_process_sample) ([**SYN\_Goertzel**](structSYN__Goertzel.md) \* g, int16\_t sample) <br>_Process a single 16-bit PCM sample through the Goertzel filter._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_goertzel\_reset**](#function-syn_goertzel_reset) ([**SYN\_Goertzel**](structSYN__Goertzel.md) \* g) <br>_Reset Goertzel filter delay accumulators for a new sample block._  |




























## Detailed Description


Provides a zero-malloc fixed-point implementation of the Goertzel algorithm for efficient single-frequency magnitude evaluation and DTMF tone decoding on resource-constrained microcontrollers without running full FFTs. 


    
## Public Functions Documentation




### function syn\_goertzel\_get\_magnitude\_sq 

_Calculate the squared magnitude \|X(k)\|^2 at the end of a sample block._ 
```C++
uint64_t syn_goertzel_get_magnitude_sq (
    const SYN_Goertzel * g
) 
```





**Parameters:**


* `g` Pointer to Goertzel instance. 



**Returns:**

64-bit squared magnitude value \|X(k)\|^2 (or 0 if invalid). 





        

<hr>



### function syn\_goertzel\_init 

_Initialize a Goertzel Filter instance._ 
```C++
SYN_Status syn_goertzel_init (
    SYN_Goertzel * g,
    uint32_t sample_rate_hz,
    uint32_t target_freq_hz,
    size_t block_size
) 
```





**Parameters:**


* `g` Pointer to Goertzel instance. 
* `sample_rate_hz` Sampling frequency in Hz (e.g. 8000). 
* `target_freq_hz` Target tone frequency in Hz (e.g. 697 for DTMF row 1). 
* `block_size` Sample block size N (e.g. 205). 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM. 





        

<hr>



### function syn\_goertzel\_process\_block 

_Process a buffer of 16-bit PCM samples through the Goertzel filter._ 
```C++
size_t syn_goertzel_process_block (
    SYN_Goertzel * g,
    const int16_t * samples,
    size_t count
) 
```





**Parameters:**


* `g` Pointer to Goertzel instance. 
* `samples` Array of 16-bit signed PCM input samples. 
* `count` Sample count in array. 



**Returns:**

Number of samples actually processed. 





        

<hr>



### function syn\_goertzel\_process\_sample 

_Process a single 16-bit PCM sample through the Goertzel filter._ 
```C++
SYN_Status syn_goertzel_process_sample (
    SYN_Goertzel * g,
    int16_t sample
) 
```





**Parameters:**


* `g` Pointer to Goertzel instance. 
* `sample` 16-bit signed PCM input sample. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM. 





        

<hr>



### function syn\_goertzel\_reset 

_Reset Goertzel filter delay accumulators for a new sample block._ 
```C++
SYN_Status syn_goertzel_reset (
    SYN_Goertzel * g
) 
```





**Parameters:**


* `g` Pointer to Goertzel instance. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_goertzel.h`

