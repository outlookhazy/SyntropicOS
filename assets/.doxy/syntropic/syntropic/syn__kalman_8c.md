

# File syn\_kalman.c



[**FileList**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_kalman.c**](syn__kalman_8c.md)

[Go to the source code of this file](syn__kalman_8c_source.md)

_General-purpose fixed-point Kalman filter implementation._ [More...](#detailed-description)

* `#include "syn_kalman.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kalman\_init**](#function-syn_kalman_init) ([**SYN\_Kalman**](structSYN__Kalman.md) \* kf, const [**SYN\_Kalman\_Config**](structSYN__Kalman__Config.md) \* cfg) <br>_Initialize the Kalman filter._  |
|  void | [**syn\_kalman\_predict**](#function-syn_kalman_predict) ([**SYN\_Kalman**](structSYN__Kalman.md) \* kf) <br>_Predict step: propagate state and covariance forward._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kalman\_update**](#function-syn_kalman_update) ([**SYN\_Kalman**](structSYN__Kalman.md) \* kf, const [**SYN\_Matrix**](structSYN__Matrix.md) \* z) <br>_Update step: incorporate a measurement._  |




























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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_kalman.c`

