

# File syn\_signal.c



[**FileList**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_signal.c**](syn__signal_8c.md)

[Go to the source code of this file](syn__signal_8c_source.md)

_Signal statistics implementation._ 

* `#include "../util/syn_assert.h"`
* `#include "../util/syn_qmath.h"`
* `#include "syn_signal.h"`
* `#include <limits.h>`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  int32\_t | [**syn\_signal\_at**](#function-syn_signal_at) (const [**SYN\_Signal**](structSYN__Signal.md) \* sig, size\_t index) <br>_Get sample at index (0 = oldest, count-1 = newest)._  |
|  void | [**syn\_signal\_clear**](#function-syn_signal_clear) ([**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Clear all samples._  |
|  int32\_t | [**syn\_signal\_crest\_factor\_q16**](#function-syn_signal_crest_factor_q16) ([**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Signal Crest Factor in Q16.16: Peak / RMS._  |
|  int32\_t | [**syn\_signal\_delta**](#function-syn_signal_delta) (const [**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Rate of change: latest - previous._  |
|  void | [**syn\_signal\_init**](#function-syn_signal_init) ([**SYN\_Signal**](structSYN__Signal.md) \* sig, int32\_t \* buf, size\_t capacity) <br>_Initialize with a caller-provided sample buffer._  |
|  int32\_t | [**syn\_signal\_latest**](#function-syn_signal_latest) (const [**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Get the most recent sample._  |
|  int32\_t | [**syn\_signal\_max**](#function-syn_signal_max) ([**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Maximum value in the window._  |
|  int32\_t | [**syn\_signal\_min**](#function-syn_signal_min) ([**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Minimum value in the window._  |
|  int32\_t | [**syn\_signal\_power\_q16**](#function-syn_signal_power_q16) (const [**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Mean signal power in Q16.16: Σ(x²) / N._  |
|  void | [**syn\_signal\_push**](#function-syn_signal_push) ([**SYN\_Signal**](structSYN__Signal.md) \* sig, int32\_t sample) <br>_Push a new sample._  |
|  int32\_t | [**syn\_signal\_rms\_q16**](#function-syn_signal_rms_q16) (const [**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Root-Mean-Square in Q16.16: sqrt(Σ(x²) / N)._  |
|  int32\_t | [**syn\_signal\_std\_dev\_q16**](#function-syn_signal_std_dev_q16) (const [**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Standard deviation in Q16.16: sqrt(variance)._  |
|  int32\_t | [**syn\_signal\_variance\_q16**](#function-syn_signal_variance_q16) (const [**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Variance in Q16.16 format._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**recompute\_minmax**](#function-recompute_minmax) ([**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Recompute cached min/max values over the signal window._  |
|  void | [**signal\_accum\_mean\_sq**](#function-signal_accum_mean_sq) (uint64\_t sq, uint64\_t n, uint64\_t \* q\_io, uint64\_t \* r\_io) <br>_Accumulate one unsigned squared magnitude into running (quotient, remainder) mod count._  |
|  void | [**signal\_mean\_sq\_parts**](#function-signal_mean_sq_parts) (const [**SYN\_Signal**](structSYN__Signal.md) \* sig, uint64\_t \* out\_whole, uint64\_t \* out\_rem) <br>_Compute exact (whole, rem) of mean squared samples across active window without overflow._  |
|  int32\_t | [**signal\_mean\_sq\_parts\_to\_q16**](#function-signal_mean_sq_parts_to_q16) (uint64\_t whole, uint64\_t rem, uint64\_t n) <br>_Convert exact (whole, rem) of sum\_sq / count into clamped Q16.16 mean square value._  |
|  uint64\_t | [**signal\_rms\_q16\_u64**](#function-signal_rms_q16_u64) (const [**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Compute unsaturated RMS value of active window in unsigned Q16.16 format._  |
|  void | [**signal\_variance\_parts**](#function-signal_variance_parts) (const [**SYN\_Signal**](structSYN__Signal.md) \* sig, uint64\_t \* out\_whole, uint64\_t \* out\_frac\_q32) <br>_Compute population variance as 64-bit integer and Q0.32 fractional parts without overflow._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**INT32\_MAX**](syn__signal_8c.md#define-int32_max)  `2147483647`<br>_INT32\_MAX fallback for pre-C99 compilers._  |
| define  | [**INT32\_MIN**](syn__signal_8c.md#define-int32_min)  `(-2147483647 - 1)`<br>_INT32\_MIN fallback for pre-C99 compilers._  |

## Public Functions Documentation




### function syn\_signal\_at 

_Get sample at index (0 = oldest, count-1 = newest)._ 
```C++
int32_t syn_signal_at (
    const SYN_Signal * sig,
    size_t index
) 
```





**Parameters:**


* `sig` Signal instance. 
* `index` Sample index. 



**Returns:**

Sample value. 





        

<hr>



### function syn\_signal\_clear 

_Clear all samples._ 
```C++
void syn_signal_clear (
    SYN_Signal * sig
) 
```





**Parameters:**


* `sig` Signal instance. 




        

<hr>



### function syn\_signal\_crest\_factor\_q16 

_Signal Crest Factor in Q16.16: Peak / RMS._ 
```C++
int32_t syn_signal_crest_factor_q16 (
    SYN_Signal * sig
) 
```



Measures signal peakiness (e.g. 1.414 for sine wave). Returns 0 if RMS is 0 or window is empty.




**Parameters:**


* `sig` Signal instance. 



**Returns:**

Crest Factor in Q16.16 format. 





        

<hr>



### function syn\_signal\_delta 

_Rate of change: latest - previous._ 
```C++
int32_t syn_signal_delta (
    const SYN_Signal * sig
) 
```



Returns 0 if fewer than 2 samples.




**Parameters:**


* `sig` Signal instance. 



**Returns:**

Delta between last two samples. 





        

<hr>



### function syn\_signal\_init 

_Initialize with a caller-provided sample buffer._ 
```C++
void syn_signal_init (
    SYN_Signal * sig,
    int32_t * buf,
    size_t capacity
) 
```





**Parameters:**


* `sig` Signal instance. 
* `buf` Sample buffer (caller-owned). 
* `capacity` Buffer capacity (number of samples). 




        

<hr>



### function syn\_signal\_latest 

_Get the most recent sample._ 
```C++
int32_t syn_signal_latest (
    const SYN_Signal * sig
) 
```





**Parameters:**


* `sig` Signal instance. 



**Returns:**

Latest sample, or 0 if empty. 





        

<hr>



### function syn\_signal\_max 

_Maximum value in the window._ 
```C++
int32_t syn_signal_max (
    SYN_Signal * sig
) 
```





**Parameters:**


* `sig` Signal instance. 



**Returns:**

Max, or 0 if empty. 





        

<hr>



### function syn\_signal\_min 

_Minimum value in the window._ 
```C++
int32_t syn_signal_min (
    SYN_Signal * sig
) 
```





**Parameters:**


* `sig` Signal instance. 



**Returns:**

Min, or 0 if empty. 





        

<hr>



### function syn\_signal\_power\_q16 

_Mean signal power in Q16.16: Σ(x²) / N._ 
```C++
int32_t syn_signal_power_q16 (
    const SYN_Signal * sig
) 
```



Returns 0 if the window is empty.




**Parameters:**


* `sig` Signal instance. 



**Returns:**

Mean power in Q16.16 format. 





        

<hr>



### function syn\_signal\_push 

_Push a new sample._ 
```C++
void syn_signal_push (
    SYN_Signal * sig,
    int32_t sample
) 
```



If the window is full, the oldest sample is evicted.




**Parameters:**


* `sig` Signal instance. 
* `sample` New sample value. 




        

<hr>



### function syn\_signal\_rms\_q16 

_Root-Mean-Square in Q16.16: sqrt(Σ(x²) / N)._ 
```C++
int32_t syn_signal_rms_q16 (
    const SYN_Signal * sig
) 
```



Returns 0 if the window is empty.




**Parameters:**


* `sig` Signal instance. 



**Returns:**

RMS value in Q16.16. 





        

<hr>



### function syn\_signal\_std\_dev\_q16 

_Standard deviation in Q16.16: sqrt(variance)._ 
```C++
int32_t syn_signal_std_dev_q16 (
    const SYN_Signal * sig
) 
```



Returns 0 if fewer than 2 samples.




**Parameters:**


* `sig` Signal instance. 



**Returns:**

Standard deviation in Q16.16. 





        

<hr>



### function syn\_signal\_variance\_q16 

_Variance in Q16.16 format._ 
```C++
int32_t syn_signal_variance_q16 (
    const SYN_Signal * sig
) 
```



Population variance: Σ(x - mean)² / N, returned as Q16.16. Returns 0 if fewer than 2 samples.




**Parameters:**


* `sig` Signal instance. 



**Returns:**

Variance in Q16.16. 





        

<hr>
## Public Static Functions Documentation




### function recompute\_minmax 

_Recompute cached min/max values over the signal window._ 
```C++
static void recompute_minmax (
    SYN_Signal * sig
) 
```





**Parameters:**


* `sig` Signal instance. 




        

<hr>



### function signal\_accum\_mean\_sq 

_Accumulate one unsigned squared magnitude into running (quotient, remainder) mod count._ 
```C++
static void signal_accum_mean_sq (
    uint64_t sq,
    uint64_t n,
    uint64_t * q_io,
    uint64_t * r_io
) 
```





**Parameters:**


* `sq` Squared sample or deviation magnitude. 
* `n` Total sample count (must be &gt; 0). 
* `q_io` [in,out] Running integer quotient of sum(sq) / n. 
* `r_io` [in,out] Running remainder of sum(sq) % n (always &lt; n). 




        

<hr>



### function signal\_mean\_sq\_parts 

_Compute exact (whole, rem) of mean squared samples across active window without overflow._ 
```C++
static void signal_mean_sq_parts (
    const SYN_Signal * sig,
    uint64_t * out_whole,
    uint64_t * out_rem
) 
```





**Parameters:**


* `sig` Signal instance (count &gt; 0). 
* `out_whole` Output integer part of mean square. 
* `out_rem` Output remainder part of mean square mod count. 




        

<hr>



### function signal\_mean\_sq\_parts\_to\_q16 

_Convert exact (whole, rem) of sum\_sq / count into clamped Q16.16 mean square value._ 
```C++
static int32_t signal_mean_sq_parts_to_q16 (
    uint64_t whole,
    uint64_t rem,
    uint64_t n
) 
```





**Parameters:**


* `whole` Integer part of sum\_sq / count. 
* `rem` Remainder part of sum\_sq % count. 
* `n` Sample count (must be &gt; 0). 



**Returns:**

Mean square value in Q16.16 format, clamped to INT32\_MAX. 





        

<hr>



### function signal\_rms\_q16\_u64 

_Compute unsaturated RMS value of active window in unsigned Q16.16 format._ 
```C++
static uint64_t signal_rms_q16_u64 (
    const SYN_Signal * sig
) 
```





**Parameters:**


* `sig` Signal instance. 



**Returns:**

Unsigned 64-bit Q16.16 RMS value, or 0 if window is empty. 





        

<hr>



### function signal\_variance\_parts 

_Compute population variance as 64-bit integer and Q0.32 fractional parts without overflow._ 
```C++
static void signal_variance_parts (
    const SYN_Signal * sig,
    uint64_t * out_whole,
    uint64_t * out_frac_q32
) 
```



Uses sum = q\_m \* N + r\_m (\|r\_m\| &lt; N) and the identity E[(X - mu)^2] = E[(X - q\_m)^2] - (r\_m / N)^2 so fractional means are exact.




**Parameters:**


* `sig` Signal instance (count &gt;= 2). 
* `out_whole` Output integer part of variance. 
* `out_frac_q32` Output Q0.32 fractional part of variance in [0, 2^32 - 1]. 




        

<hr>
## Macro Definition Documentation





### define INT32\_MAX 

_INT32\_MAX fallback for pre-C99 compilers._ 
```C++
#define INT32_MAX `2147483647`
```




<hr>



### define INT32\_MIN 

_INT32\_MIN fallback for pre-C99 compilers._ 
```C++
#define INT32_MIN `(-2147483647 - 1)`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_signal.c`

