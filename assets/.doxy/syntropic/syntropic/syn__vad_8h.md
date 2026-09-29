

# File syn\_vad.h



[**FileList**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_vad.h**](syn__vad_8h.md)

[Go to the source code of this file](syn__vad_8h_source.md)

_Zero-Heap Acoustic Voice Activity Detector (VAD) Engine (Q15/Q16 fixed-point)._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../util/syn_qmath.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_VAD**](structSYN__VAD.md) <br>_Voice Activity Detector instance context._  |
| struct | [**SYN\_VAD\_Config**](structSYN__VAD__Config.md) <br>_VAD configuration descriptor._  |
| struct | [**SYN\_VAD\_Features**](structSYN__VAD__Features.md) <br>_Extracted audio frame feature metrics._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_VAD\_Sensitivity**](#enum-syn_vad_sensitivity)  <br>_VAD sensitivity preset level._  |
| enum  | [**SYN\_VAD\_State**](#enum-syn_vad_state)  <br>_Voice Activity Detection state._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_VAD\_State**](syn__vad_8h.md#enum-syn_vad_state) | [**syn\_vad\_get\_state**](#function-syn_vad_get_state) (const [**SYN\_VAD**](structSYN__VAD.md) \* vad) <br>_Get current smoothed VAD state without processing a new frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_vad\_init**](#function-syn_vad_init) ([**SYN\_VAD**](structSYN__VAD.md) \* vad, const [**SYN\_VAD\_Config**](structSYN__VAD__Config.md) \* cfg) <br>_Initialize Voice Activity Detector context._  |
|  [**SYN\_VAD\_State**](syn__vad_8h.md#enum-syn_vad_state) | [**syn\_vad\_process\_frame**](#function-syn_vad_process_frame) ([**SYN\_VAD**](structSYN__VAD.md) \* vad, const int16\_t \* samples, size\_t num\_samples, [**SYN\_VAD\_Features**](structSYN__VAD__Features.md) \* out\_features) <br>_Process single frame of 16-bit signed PCM audio samples._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_vad\_reset**](#function-syn_vad_reset) ([**SYN\_VAD**](structSYN__VAD.md) \* vad) <br>_Reset VAD state counters and restore default noise floor._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_vad\_set\_sensitivity**](#function-syn_vad_set_sensitivity) ([**SYN\_VAD**](structSYN__VAD.md) \* vad, [**SYN\_VAD\_Sensitivity**](syn__vad_8h.md#enum-syn_vad_sensitivity) sensitivity) <br>_Adjust VAD sensitivity preset._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_VAD\_DEFAULT\_ATTACK\_FRAMES**](syn__vad_8h.md#define-syn_vad_default_attack_frames)  `2U`<br> |
| define  | [**SYN\_VAD\_DEFAULT\_FRAME\_LEN**](syn__vad_8h.md#define-syn_vad_default_frame_len)  `160U`<br> |
| define  | [**SYN\_VAD\_DEFAULT\_HANGOVER\_FRAMES**](syn__vad_8h.md#define-syn_vad_default_hangover_frames)  `6U`<br> |

## Detailed Description


Implements a low-latency, deterministic Voice Activity Detector for embedded microcontrollers:
* Frame-by-frame acoustic analysis (10ms - 30ms windows of 16-bit PCM audio).
* Multi-feature extraction: Short-Time Energy (STE), Zero-Crossing Rate (ZCR), and High/Low Spectral Energy Ratio.
* Dynamic background noise floor tracking via Exponential Moving Average (EMA).
* Configurable attack onset validation and hangover release timing.
* Sensitivity presets (Aggressive, Normal, Sensitive). 




    
## Public Types Documentation




### enum SYN\_VAD\_Sensitivity 

_VAD sensitivity preset level._ 
```C++
enum SYN_VAD_Sensitivity {
    SYN_VAD_SENSITIVITY_SENSITIVE = 0,
    SYN_VAD_SENSITIVITY_NORMAL = 1,
    SYN_VAD_SENSITIVITY_AGGRESSIVE = 2
};
```




<hr>



### enum SYN\_VAD\_State 

_Voice Activity Detection state._ 
```C++
enum SYN_VAD_State {
    SYN_VAD_STATE_SILENCE = 0,
    SYN_VAD_STATE_SPEECH = 1
};
```




<hr>
## Public Functions Documentation




### function syn\_vad\_get\_state 

_Get current smoothed VAD state without processing a new frame._ 
```C++
SYN_VAD_State syn_vad_get_state (
    const SYN_VAD * vad
) 
```





**Parameters:**


* `vad` VAD instance pointer. 



**Returns:**

Current VAD state. 





        

<hr>



### function syn\_vad\_init 

_Initialize Voice Activity Detector context._ 
```C++
SYN_Status syn_vad_init (
    SYN_VAD * vad,
    const SYN_VAD_Config * cfg
) 
```





**Parameters:**


* `vad` VAD instance pointer. 
* `cfg` Configuration descriptor (or NULL for default 16kHz parameters). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid argument. 





        

<hr>



### function syn\_vad\_process\_frame 

_Process single frame of 16-bit signed PCM audio samples._ 
```C++
SYN_VAD_State syn_vad_process_frame (
    SYN_VAD * vad,
    const int16_t * samples,
    size_t num_samples,
    SYN_VAD_Features * out_features
) 
```





**Parameters:**


* `vad` VAD instance pointer. 
* `samples` Array of 16-bit PCM audio samples. 
* `num_samples` Number of samples in frame (should match configured frame\_length). 
* `out_features` Optional pointer to receive extracted frame metrics (can be NULL). 



**Returns:**

Current smoothed VAD decision (SYN\_VAD\_STATE\_SPEECH or SYN\_VAD\_STATE\_SILENCE). 





        

<hr>



### function syn\_vad\_reset 

_Reset VAD state counters and restore default noise floor._ 
```C++
SYN_Status syn_vad_reset (
    SYN_VAD * vad
) 
```





**Parameters:**


* `vad` VAD instance pointer. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_vad\_set\_sensitivity 

_Adjust VAD sensitivity preset._ 
```C++
SYN_Status syn_vad_set_sensitivity (
    SYN_VAD * vad,
    SYN_VAD_Sensitivity sensitivity
) 
```





**Parameters:**


* `vad` VAD instance pointer. 
* `sensitivity` Sensitivity preset level. 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Macro Definition Documentation





### define SYN\_VAD\_DEFAULT\_ATTACK\_FRAMES 

```C++
#define SYN_VAD_DEFAULT_ATTACK_FRAMES `2U`
```



Consecutive active frames required to trigger 


        

<hr>



### define SYN\_VAD\_DEFAULT\_FRAME\_LEN 

```C++
#define SYN_VAD_DEFAULT_FRAME_LEN `160U`
```



Default 160 samples (10ms @ 16kHz or 20ms @ 8kHz) 


        

<hr>



### define SYN\_VAD\_DEFAULT\_HANGOVER\_FRAMES 

```C++
#define SYN_VAD_DEFAULT_HANGOVER_FRAMES `6U`
```



Hangover frames to sustain active speech 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_vad.h`

