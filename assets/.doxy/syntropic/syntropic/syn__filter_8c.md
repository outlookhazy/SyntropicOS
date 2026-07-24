

# File syn\_filter.c



[**FileList**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_filter.c**](syn__filter_8c.md)

[Go to the source code of this file](syn__filter_8c_source.md)

_Digital filter implementations._ 

* `#include "syn_filter.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_filter\_ema\_init**](#function-syn_filter_ema_init) ([**SYN\_FilterEMA**](structSYN__FilterEMA.md) \* f, uint8\_t alpha) <br>_Initialize an EMA filter._  |
|  void | [**syn\_filter\_ema\_reset**](#function-syn_filter_ema_reset) ([**SYN\_FilterEMA**](structSYN__FilterEMA.md) \* f) <br>_Reset the filter to its initial state._  |
|  int16\_t | [**syn\_filter\_ema\_update**](#function-syn_filter_ema_update) ([**SYN\_FilterEMA**](structSYN__FilterEMA.md) \* f, int16\_t sample) <br>_Feed a new sample and get the filtered output._  |
|  void | [**syn\_filter\_fir\_init**](#function-syn_filter_fir_init) ([**SYN\_FilterFIR**](structSYN__FilterFIR.md) \* f, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* taps, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* history, uint16\_t num\_taps) <br>_Initialize an FIR filter._  |
|  void | [**syn\_filter\_fir\_reset**](#function-syn_filter_fir_reset) ([**SYN\_FilterFIR**](structSYN__FilterFIR.md) \* f) <br>_Reset FIR filter sample history to 0._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**syn\_filter\_fir\_update**](#function-syn_filter_fir_update) ([**SYN\_FilterFIR**](structSYN__FilterFIR.md) \* f, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) sample) <br>_Feed a new Q16 sample and get the FIR filtered output._  |
|  void | [**syn\_filter\_ma\_init**](#function-syn_filter_ma_init) ([**SYN\_FilterMA**](structSYN__FilterMA.md) \* f, uint8\_t window) <br>_Initialize a moving average filter._  |
|  void | [**syn\_filter\_ma\_reset**](#function-syn_filter_ma_reset) ([**SYN\_FilterMA**](structSYN__FilterMA.md) \* f) <br>_Reset the filter to its initial state._  |
|  int16\_t | [**syn\_filter\_ma\_update**](#function-syn_filter_ma_update) ([**SYN\_FilterMA**](structSYN__FilterMA.md) \* f, int16\_t sample) <br>_Feed a new sample and get the filtered output._  |
|  void | [**syn\_filter\_median\_init**](#function-syn_filter_median_init) ([**SYN\_FilterMedian**](structSYN__FilterMedian.md) \* f, uint8\_t window) <br>_Initialize a median filter._  |
|  void | [**syn\_filter\_median\_reset**](#function-syn_filter_median_reset) ([**SYN\_FilterMedian**](structSYN__FilterMedian.md) \* f) <br>_Reset the filter to its initial state._  |
|  int16\_t | [**syn\_filter\_median\_update**](#function-syn_filter_median_update) ([**SYN\_FilterMedian**](structSYN__FilterMedian.md) \* f, int16\_t sample) <br>_Feed a new sample and get the filtered output._  |




























## Public Functions Documentation




### function syn\_filter\_ema\_init 

_Initialize an EMA filter._ 
```C++
void syn_filter_ema_init (
    SYN_FilterEMA * f,
    uint8_t alpha
) 
```





**Parameters:**


* `f` Filter instance. 
* `alpha` Smoothing factor (0–255). Higher = more responsive. Equivalent to alpha = value/256. Example: 64 ≈ 0.25 (slow), 192 ≈ 0.75 (fast). 




        

<hr>



### function syn\_filter\_ema\_reset 

_Reset the filter to its initial state._ 
```C++
void syn_filter_ema_reset (
    SYN_FilterEMA * f
) 
```





**Parameters:**


* `f` Filter instance. 




        

<hr>



### function syn\_filter\_ema\_update 

_Feed a new sample and get the filtered output._ 
```C++
int16_t syn_filter_ema_update (
    SYN_FilterEMA * f,
    int16_t sample
) 
```





**Parameters:**


* `f` Filter instance. 
* `sample` New raw sample. 



**Returns:**

Filtered value. 





        

<hr>



### function syn\_filter\_fir\_init 

_Initialize an FIR filter._ 
```C++
void syn_filter_fir_init (
    SYN_FilterFIR * f,
    const q16_t * taps,
    q16_t * history,
    uint16_t num_taps
) 
```





**Parameters:**


* `f` Filter instance. 
* `taps` Array of Q16 filter coefficients. 
* `history` Caller-provided Q16 history buffer of size `num_taps`. 
* `num_taps` Number of filter taps. 




        

<hr>



### function syn\_filter\_fir\_reset 

_Reset FIR filter sample history to 0._ 
```C++
void syn_filter_fir_reset (
    SYN_FilterFIR * f
) 
```





**Parameters:**


* `f` Filter instance. 




        

<hr>



### function syn\_filter\_fir\_update 

_Feed a new Q16 sample and get the FIR filtered output._ 
```C++
q16_t syn_filter_fir_update (
    SYN_FilterFIR * f,
    q16_t sample
) 
```





**Parameters:**


* `f` Filter instance. 
* `sample` New Q16 sample. 



**Returns:**

Filtered output in Q16. 





        

<hr>



### function syn\_filter\_ma\_init 

_Initialize a moving average filter._ 
```C++
void syn_filter_ma_init (
    SYN_FilterMA * f,
    uint8_t window
) 
```





**Parameters:**


* `f` Filter instance. 
* `window` Window size (must be &lt;= SYN\_FILTER\_MAX\_WINDOW). 




        

<hr>



### function syn\_filter\_ma\_reset 

_Reset the filter to its initial state._ 
```C++
void syn_filter_ma_reset (
    SYN_FilterMA * f
) 
```





**Parameters:**


* `f` Filter instance. 




        

<hr>



### function syn\_filter\_ma\_update 

_Feed a new sample and get the filtered output._ 
```C++
int16_t syn_filter_ma_update (
    SYN_FilterMA * f,
    int16_t sample
) 
```





**Parameters:**


* `f` Filter instance. 
* `sample` New raw sample. 



**Returns:**

Filtered value (average of the last `window` samples). 





        

<hr>



### function syn\_filter\_median\_init 

_Initialize a median filter._ 
```C++
void syn_filter_median_init (
    SYN_FilterMedian * f,
    uint8_t window
) 
```





**Parameters:**


* `f` Filter instance. 
* `window` Window size (should be odd for a true median; must be &lt;= SYN\_FILTER\_MAX\_WINDOW). 




        

<hr>



### function syn\_filter\_median\_reset 

_Reset the filter to its initial state._ 
```C++
void syn_filter_median_reset (
    SYN_FilterMedian * f
) 
```





**Parameters:**


* `f` Filter instance. 




        

<hr>



### function syn\_filter\_median\_update 

_Feed a new sample and get the filtered output._ 
```C++
int16_t syn_filter_median_update (
    SYN_FilterMedian * f,
    int16_t sample
) 
```





**Parameters:**


* `f` Filter instance. 
* `sample` New raw sample. 



**Returns:**

Median of the last `window` samples. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_filter.c`

