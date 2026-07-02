

# File syn\_signal.h



[**FileList**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_signal.h**](syn__signal_8h.md)

[Go to the source code of this file](syn__signal_8h_source.md)

_Signal statistics — sliding window min/max/mean/variance._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Signal**](structSYN__Signal.md) <br>_Signal statistics — sliding window with incremental min/max/mean._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  int32\_t | [**syn\_signal\_at**](#function-syn_signal_at) (const [**SYN\_Signal**](structSYN__Signal.md) \* sig, size\_t index) <br>_Get sample at index (0 = oldest, count-1 = newest)._  |
|  void | [**syn\_signal\_clear**](#function-syn_signal_clear) ([**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Clear all samples._  |
|  int32\_t | [**syn\_signal\_delta**](#function-syn_signal_delta) (const [**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Rate of change: latest - previous._  |
|  void | [**syn\_signal\_init**](#function-syn_signal_init) ([**SYN\_Signal**](structSYN__Signal.md) \* sig, int32\_t \* buf, size\_t capacity) <br>_Initialize with a caller-provided sample buffer._  |
|  int32\_t | [**syn\_signal\_latest**](#function-syn_signal_latest) (const [**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Get the most recent sample._  |
|  int32\_t | [**syn\_signal\_max**](#function-syn_signal_max) ([**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Maximum value in the window._  |
|  int32\_t | [**syn\_signal\_min**](#function-syn_signal_min) ([**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Minimum value in the window._  |
|  void | [**syn\_signal\_push**](#function-syn_signal_push) ([**SYN\_Signal**](structSYN__Signal.md) \* sig, int32\_t sample) <br>_Push a new sample._  |
|  int32\_t | [**syn\_signal\_variance\_q16**](#function-syn_signal_variance_q16) (const [**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Variance in Q16.16 format._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_signal\_count**](#function-syn_signal_count) (const [**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Number of samples in the window._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_signal\_full**](#function-syn_signal_full) (const [**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_True if the window is full._  |
|  int32\_t | [**syn\_signal\_mean**](#function-syn_signal_mean) (const [**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Integer mean (truncated)._  |
|  int32\_t | [**syn\_signal\_peak\_to\_peak**](#function-syn_signal_peak_to_peak) ([**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Peak-to-peak = max - min._  |
|  int64\_t | [**syn\_signal\_sum**](#function-syn_signal_sum) (const [**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Sum of all samples in window._  |


























## Detailed Description


Maintains a circular buffer of samples and computes statistics incrementally. Designed for sensor data conditioning.


All math is integer-only. Mean and variance use Q16.16 for fractional precision without floating point.




**
**


```C++
int32_t samples[32];
SYN_Signal sig;
syn_signal_init(&sig, samples, 32);

// Feed samples:
syn_signal_push(&sig, adc_read());

// Query stats:
int32_t min = syn_signal_min(&sig);
int32_t max = syn_signal_max(&sig);
int32_t avg = syn_signal_mean(&sig);       // integer mean
int32_t pp  = syn_signal_peak_to_peak(&sig);
int32_t var = syn_signal_variance_q16(&sig); // Q16.16
```
 





    
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




### function syn\_signal\_count 

_Number of samples in the window._ 
```C++
static inline size_t syn_signal_count (
    const SYN_Signal * sig
) 
```





**Parameters:**


* `sig` Signal instance. 



**Returns:**

Sample count. 





        

<hr>



### function syn\_signal\_full 

_True if the window is full._ 
```C++
static inline bool syn_signal_full (
    const SYN_Signal * sig
) 
```





**Parameters:**


* `sig` Signal instance. 



**Returns:**

true if count == capacity. 





        

<hr>



### function syn\_signal\_mean 

_Integer mean (truncated)._ 
```C++
static inline int32_t syn_signal_mean (
    const SYN_Signal * sig
) 
```





**Parameters:**


* `sig` Signal instance. 



**Returns:**

Truncated mean, or 0 if empty. 





        

<hr>



### function syn\_signal\_peak\_to\_peak 

_Peak-to-peak = max - min._ 
```C++
static inline int32_t syn_signal_peak_to_peak (
    SYN_Signal * sig
) 
```





**Parameters:**


* `sig` Signal instance. 



**Returns:**

Peak-to-peak value, or 0 if empty. 





        

<hr>



### function syn\_signal\_sum 

_Sum of all samples in window._ 
```C++
static inline int64_t syn_signal_sum (
    const SYN_Signal * sig
) 
```





**Parameters:**


* `sig` Signal instance. 



**Returns:**

Running sum. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_signal.h`

