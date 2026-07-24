

# File syn\_kalman.h



[**FileList**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_kalman.h**](syn__kalman_8h.md)

[Go to the source code of this file](syn__kalman_8h_source.md)

_General-purpose fixed-point Kalman filter._ [More...](#detailed-description)

* `#include "../util/syn_matrix.h"`
* `#include "../common/syn_defs.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Kalman**](structSYN__Kalman.md) <br>_Kalman filter instance._  |
| struct | [**SYN\_Kalman\_Config**](structSYN__Kalman__Config.md) <br>_Kalman filter configuration — all matrices are caller-owned._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kalman\_init**](#function-syn_kalman_init) ([**SYN\_Kalman**](structSYN__Kalman.md) \* kf, const [**SYN\_Kalman\_Config**](structSYN__Kalman__Config.md) \* cfg) <br>_Initialize the Kalman filter._  |
|  void | [**syn\_kalman\_predict**](#function-syn_kalman_predict) ([**SYN\_Kalman**](structSYN__Kalman.md) \* kf) <br>_Predict step: propagate state and covariance forward._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kalman\_update**](#function-syn_kalman_update) ([**SYN\_Kalman**](structSYN__Kalman.md) \* kf, const [**SYN\_Matrix**](structSYN__Matrix.md) \* z) <br>_Update step: incorporate a measurement._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_KALMAN\_MAX\_MEAS**](syn__kalman_8h.md#define-syn_kalman_max_meas)  `4`<br>_Maximum measurement dimension (default 4, override in syn\_config.h)._  |
| define  | [**SYN\_KALMAN\_MAX\_STATE**](syn__kalman_8h.md#define-syn_kalman_max_state)  `6`<br>_Maximum state dimension (default 6, override in syn\_config.h)._  |
| define  | [**SYN\_KALMAN\_SCRATCH\_ASSIGN**](syn__kalman_8h.md#define-syn_kalman_scratch_assign) (kf, prefix) `/* multi line expression */`<br>_Assign scratch matrices to the Kalman filter instance._  |
| define  | [**SYN\_KALMAN\_SCRATCH\_DECL**](syn__kalman_8h.md#define-syn_kalman_scratch_decl) (prefix, NS, NM) `/* multi line expression */`<br>_Convenience macro to declare all Kalman scratch matrices._  |

## Detailed Description


Implements a discrete-time linear Kalman filter using Q16.16 matrix operations. Supports arbitrary state and measurement dimensions (compile-time determined, no heap allocation).




**
**

Predict:
* x̂⁻ = F · x̂
* P⁻ = F · P · Fᵀ + Q




Update:
* y = z − H · x̂⁻ (innovation)
* S = H · P⁻ · Hᵀ + R (innovation covariance)
* K = P⁻ · Hᵀ · S⁻¹ (Kalman gain)
* x̂ = x̂⁻ + K · y (state update)
* P = (I − K · H) · P⁻ (covariance update)






**
**


```C++
// State: [position, velocity] → 2×1
// Measurement: [position]    → 1×1
#define N_STATE 2
#define N_MEAS  1

SYN_Kalman kf;
SYN_Kalman_Config cfg;

// Allocate all matrices on the stack
SYN_MAT_DECL(x,  N_STATE, 1);       // State vector
SYN_MAT_DECL(P,  N_STATE, N_STATE); // Error covariance
SYN_MAT_DECL(F,  N_STATE, N_STATE); // State transition
SYN_MAT_DECL(Q,  N_STATE, N_STATE); // Process noise
SYN_MAT_DECL(H,  N_MEAS,  N_STATE); // Measurement model
SYN_MAT_DECL(R,  N_MEAS,  N_MEAS);  // Measurement noise
SYN_MAT_DECL(z,  N_MEAS,  1);       // Measurement vector

cfg.x = &x;  cfg.P = &P;  cfg.F = &F;
cfg.Q = &Q;  cfg.H = &H;  cfg.R = &R;
cfg.n_state = N_STATE;
cfg.n_meas  = N_MEAS;

// Set up F = [[1, dt], [0, 1]] for constant-velocity model
syn_matrix_identity(&F);
SYN_MAT_AT(&F, 0, 1) = Q16_FROM_FRAC(1, 100); // dt = 10ms

// H = [[1, 0]] — we only measure position
syn_matrix_zero(&H);
SYN_MAT_AT(&H, 0, 0) = Q16_ONE;

syn_kalman_init(&kf, &cfg);

// In control loop:
z.data[0] = sensor_reading;
syn_kalman_predict(&kf);
syn_kalman_update(&kf, &z);
q16_t filtered_pos = kf.cfg->x->data[0];
```
 





    
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
## Macro Definition Documentation





### define SYN\_KALMAN\_MAX\_MEAS 

_Maximum measurement dimension (default 4, override in syn\_config.h)._ 
```C++
#define SYN_KALMAN_MAX_MEAS `4`
```




<hr>



### define SYN\_KALMAN\_MAX\_STATE 

_Maximum state dimension (default 6, override in syn\_config.h)._ 
```C++
#define SYN_KALMAN_MAX_STATE `6`
```



Controls the stack size of internal Kalman gain buffer during update. 


        

<hr>



### define SYN\_KALMAN\_SCRATCH\_ASSIGN 

_Assign scratch matrices to the Kalman filter instance._ 
```C++
#define SYN_KALMAN_SCRATCH_ASSIGN (
    kf,
    prefix
) `/* multi line expression */`
```





**Parameters:**


* `kf` Kalman filter instance. 
* `prefix` Name prefix used in SYN\_KALMAN\_SCRATCH\_DECL. 




        

<hr>



### define SYN\_KALMAN\_SCRATCH\_DECL 

_Convenience macro to declare all Kalman scratch matrices._ 
```C++
#define SYN_KALMAN_SCRATCH_DECL (
    prefix,
    NS,
    NM
) `SYN_MAT_DECL (prefix##_nn1, NS, NS);                        \ SYN_MAT_DECL (prefix##_nn2, NS, NS);                        \ SYN_MAT_DECL (prefix##_nm,  NS, NM);                        \ SYN_MAT_DECL (prefix##_mn,  NM, NS);                        \ SYN_MAT_DECL (prefix##_mm,  NM, NM);                        \ SYN_MAT_DECL (prefix##_mm2, NM, NM);                        \ SYN_MAT_DECL (prefix##_n1,  NS, 1);                         \ SYN_MAT_DECL (prefix##_m1,  NM, 1)`
```



Declares the 8 scratch matrices needed by [**SYN\_Kalman**](structSYN__Kalman.md) on the stack.




**Parameters:**


* `prefix` Name prefix for the scratch variables. 
* `NS` State dimension. 
* `NM` Measurement dimension. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_kalman.h`

