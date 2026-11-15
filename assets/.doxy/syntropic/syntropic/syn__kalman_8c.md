

# File syn\_kalman.c



[**FileList**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_kalman.c**](syn__kalman_8c.md)

[Go to the source code of this file](syn__kalman_8c_source.md)

_General-purpose fixed-point Kalman filter implementation._ [More...](#detailed-description)

* `#include "../util/syn_assert.h"`
* `#include "syn_kalman.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kalman\_init**](#function-syn_kalman_init) ([**SYN\_Kalman**](structSYN__Kalman.md) \* kf, const [**SYN\_Kalman\_Config**](structSYN__Kalman__Config.md) \* cfg) <br>_Initialize the Kalman filter._  |
|  void | [**syn\_kalman\_predict**](#function-syn_kalman_predict) ([**SYN\_Kalman**](structSYN__Kalman.md) \* kf) <br>_Predict step: propagate state and covariance forward._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kalman\_update**](#function-syn_kalman_update) ([**SYN\_Kalman**](structSYN__Kalman.md) \* kf, const [**SYN\_Matrix**](structSYN__Matrix.md) \* z) <br>_Update step: incorporate a measurement._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**kalman\_cfg\_ready**](#function-kalman_cfg_ready) (const [**SYN\_Kalman**](structSYN__Kalman.md) \* kf) <br>_Validate that a Kalman filter instance has an initialized state configuration._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**kalman\_mat\_matches**](#function-kalman_mat_matches) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* m, uint8\_t rows, uint8\_t cols) <br>_Check that a matrix descriptor, backing buffer, and dimensions are valid._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**kalman\_predict\_valid**](#function-kalman_predict_valid) (const [**SYN\_Kalman**](structSYN__Kalman.md) \* kf) <br>_Validate configuration and scratch matrices required by syn\_kalman\_predict._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**kalman\_update\_valid**](#function-kalman_update_valid) (const [**SYN\_Kalman**](structSYN__Kalman.md) \* kf, const [**SYN\_Matrix**](structSYN__Matrix.md) \* z) <br>_Validate configuration, measurement vector, and scratch buffers for syn\_kalman\_update._  |


























## Detailed Description


All operations use caller-owned [**SYN\_Matrix**](structSYN__Matrix.md) instances. No heap allocation. Uses int64\_t accumulator matrix multiply for full Q16 precision. 


    
## Public Functions Documentation




### function syn\_kalman\_init 

_Initialize the Kalman filter._ 
```C++
SYN_Status syn_kalman_init (
    SYN_Kalman * kf,
    const SYN_Kalman_Config * cfg
) 
```



The caller must have populated cfg-&gt;F, cfg-&gt;H, cfg-&gt;Q, cfg-&gt;R, and initial cfg-&gt;x and cfg-&gt;P before calling this. Scratch matrices must be assigned via SYN\_KALMAN\_SCRATCH\_ASSIGN.




**Parameters:**


* `kf` Kalman filter instance. 
* `cfg` Configuration (caller-owned, must outlive kf). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on dimension mismatch. 





        

<hr>



### function syn\_kalman\_predict 

_Predict step: propagate state and covariance forward._ 
```C++
void syn_kalman_predict (
    SYN_Kalman * kf
) 
```



After this call:
* x̂⁻ = F · x̂ (state predicted forward)
* P⁻ = F · P · Fᵀ + Q (covariance grows)






**Parameters:**


* `kf` Kalman filter instance. 




        

<hr>



### function syn\_kalman\_update 

_Update step: incorporate a measurement._ 
```C++
SYN_Status syn_kalman_update (
    SYN_Kalman * kf,
    const SYN_Matrix * z
) 
```



After this call:
* x̂ is corrected toward the measurement
* P is reduced (uncertainty decreased)






**Parameters:**


* `kf` Kalman filter instance. 
* `z` Measurement vector (n\_meas × 1). 



**Returns:**

SYN\_OK on success, SYN\_ERROR if innovation covariance is singular. 





        

<hr>
## Public Static Functions Documentation




### function kalman\_cfg\_ready 

_Validate that a Kalman filter instance has an initialized state configuration._ 
```C++
static bool kalman_cfg_ready (
    const SYN_Kalman * kf
) 
```





**Parameters:**


* `kf` Kalman filter instance. 



**Returns:**

True if kf and kf-&gt;cfg are non-NULL with valid dimensions and state matrices. 





        

<hr>



### function kalman\_mat\_matches 

_Check that a matrix descriptor, backing buffer, and dimensions are valid._ 
```C++
static bool kalman_mat_matches (
    const SYN_Matrix * m,
    uint8_t rows,
    uint8_t cols
) 
```





**Parameters:**


* `m` Matrix descriptor to validate. 
* `rows` Expected row count. 
* `cols` Expected column count. 



**Returns:**

True if matrix is non-NULL with matching dimensions and non-NULL data. 





        

<hr>



### function kalman\_predict\_valid 

_Validate configuration and scratch matrices required by syn\_kalman\_predict._ 
```C++
static bool kalman_predict_valid (
    const SYN_Kalman * kf
) 
```





**Parameters:**


* `kf` Kalman filter instance. 



**Returns:**

True if predict can execute safely, false otherwise. 





        

<hr>



### function kalman\_update\_valid 

_Validate configuration, measurement vector, and scratch buffers for syn\_kalman\_update._ 
```C++
static bool kalman_update_valid (
    const SYN_Kalman * kf,
    const SYN_Matrix * z
) 
```





**Parameters:**


* `kf` Kalman filter instance. 
* `z` Measurement vector (n\_meas x 1). 



**Returns:**

True if update can execute safely, false otherwise. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_kalman.c`

