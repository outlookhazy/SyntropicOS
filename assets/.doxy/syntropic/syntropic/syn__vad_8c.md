

# File syn\_vad.c



[**FileList**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_vad.c**](syn__vad_8c.md)

[Go to the source code of this file](syn__vad_8c_source.md)

_Zero-Heap Acoustic Voice Activity Detector (VAD) Engine Implementation._ 

* `#include "../port/syn_port_system.h"`
* `#include "syn_vad.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_VAD\_State**](syn__vad_8h.md#enum-syn_vad_state) | [**syn\_vad\_get\_state**](#function-syn_vad_get_state) (const [**SYN\_VAD**](structSYN__VAD.md) \* vad) <br>_Get current smoothed VAD state without processing a new frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_vad\_init**](#function-syn_vad_init) ([**SYN\_VAD**](structSYN__VAD.md) \* vad, const [**SYN\_VAD\_Config**](structSYN__VAD__Config.md) \* cfg) <br>_Initialize Voice Activity Detector context._  |
|  [**SYN\_VAD\_State**](syn__vad_8h.md#enum-syn_vad_state) | [**syn\_vad\_process\_frame**](#function-syn_vad_process_frame) ([**SYN\_VAD**](structSYN__VAD.md) \* vad, const int16\_t \* samples, size\_t num\_samples, [**SYN\_VAD\_Features**](structSYN__VAD__Features.md) \* out\_features) <br>_Process single frame of 16-bit signed PCM audio samples._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_vad\_reset**](#function-syn_vad_reset) ([**SYN\_VAD**](structSYN__VAD.md) \* vad) <br>_Reset VAD state counters and restore default noise floor._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_vad\_set\_sensitivity**](#function-syn_vad_set_sensitivity) ([**SYN\_VAD**](structSYN__VAD.md) \* vad, [**SYN\_VAD\_Sensitivity**](syn__vad_8h.md#enum-syn_vad_sensitivity) sensitivity) <br>_Adjust VAD sensitivity preset._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**vad\_apply\_sensitivity**](#function-vad_apply_sensitivity) ([**SYN\_VAD**](structSYN__VAD.md) \* vad, [**SYN\_VAD\_Sensitivity**](syn__vad_8h.md#enum-syn_vad_sensitivity) sensitivity) <br>_Configure internal threshold parameters based on sensitivity preset._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_VAD\_DEFAULT\_INITIAL\_NOISE**](syn__vad_8c.md#define-syn_vad_default_initial_noise)  `200U`<br> |
| define  | [**SYN\_VAD\_MIN\_NOISE\_FLOOR**](syn__vad_8c.md#define-syn_vad_min_noise_floor)  `50U`<br> |

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
## Public Static Functions Documentation




### function vad\_apply\_sensitivity 

_Configure internal threshold parameters based on sensitivity preset._ 
```C++
static void vad_apply_sensitivity (
    SYN_VAD * vad,
    SYN_VAD_Sensitivity sensitivity
) 
```





**Parameters:**


* `vad` VAD instance pointer. 
* `sensitivity` Sensitivity preset level. 




        

<hr>
## Macro Definition Documentation





### define SYN\_VAD\_DEFAULT\_INITIAL\_NOISE 

```C++
#define SYN_VAD_DEFAULT_INITIAL_NOISE `200U`
```



Default initial noise energy estimate 


        

<hr>



### define SYN\_VAD\_MIN\_NOISE\_FLOOR 

```C++
#define SYN_VAD_MIN_NOISE_FLOOR `50U`
```



Minimum floor clamp for adaptive noise 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_vad.c`

