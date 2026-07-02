

# File syn\_signal.c



[**FileList**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_signal.c**](syn__signal_8c.md)

[Go to the source code of this file](syn__signal_8c_source.md)

_Signal statistics implementation._ 

* `#include "syn_signal.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`
* `#include <limits.h>`





































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
|  void | [**recompute\_minmax**](#function-recompute_minmax) ([**SYN\_Signal**](structSYN__Signal.md) \* sig) <br>_Recompute cached min/max values over the signal window._  |

























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

