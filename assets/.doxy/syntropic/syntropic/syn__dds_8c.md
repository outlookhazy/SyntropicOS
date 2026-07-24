

# File syn\_dds.c



[**FileList**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_dds.c**](syn__dds_8c.md)

[Go to the source code of this file](syn__dds_8c_source.md)

_Direct Digital Synthesis (DDS) Waveform Synthesizer implementation._ 

* `#include "syn_dds.h"`
* `#include "../util/syn_random.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dds\_fill\_q16**](#function-syn_dds_fill_q16) ([**SYN\_DDS**](structSYN__DDS.md) \* dds, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* buf, size\_t count) <br>_Fill a Q16.16 buffer with consecutive DDS samples._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dds\_fill\_u16**](#function-syn_dds_fill_u16) ([**SYN\_DDS**](structSYN__DDS.md) \* dds, uint16\_t \* buf, size\_t count, uint16\_t dac\_center, uint16\_t dac\_span) <br>_Fill a unsigned 16-bit integer buffer for DAC DMA output._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dds\_init**](#function-syn_dds_init) ([**SYN\_DDS**](structSYN__DDS.md) \* dds, [**SYN\_DDS\_Waveform**](syn__dds_8h.md#enum-syn_dds_waveform) type, uint32\_t freq\_hz, uint32\_t sample\_rate\_hz) <br>_Initialize a DDS Generator instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dds\_set\_duty**](#function-syn_dds_set_duty) ([**SYN\_DDS**](structSYN__DDS.md) \* dds, float duty\_pct) <br>_Set PWM duty cycle (for SYN\_DDS\_SQUARE mode)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dds\_set\_freq**](#function-syn_dds_set_freq) ([**SYN\_DDS**](structSYN__DDS.md) \* dds, uint32\_t freq\_hz, uint32\_t sample\_rate\_hz) <br>_Set the output frequency of the DDS generator._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dds\_set\_gain**](#function-syn_dds_set_gain) ([**SYN\_DDS**](structSYN__DDS.md) \* dds, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) amplitude, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) offset) <br>_Set the peak amplitude and DC offset._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**syn\_dds\_step**](#function-syn_dds_step) ([**SYN\_DDS**](structSYN__DDS.md) \* dds) <br>_Compute and advance the DDS generator by one sample step._  |




























## Public Functions Documentation




### function syn\_dds\_fill\_q16 

_Fill a Q16.16 buffer with consecutive DDS samples._ 
```C++
SYN_Status syn_dds_fill_q16 (
    SYN_DDS * dds,
    q16_t * buf,
    size_t count
) 
```





**Parameters:**


* `dds` Pointer to DDS instance. 
* `buf` Output Q16.16 buffer. 
* `count` Number of samples to generate. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_dds\_fill\_u16 

_Fill a unsigned 16-bit integer buffer for DAC DMA output._ 
```C++
SYN_Status syn_dds_fill_u16 (
    SYN_DDS * dds,
    uint16_t * buf,
    size_t count,
    uint16_t dac_center,
    uint16_t dac_span
) 
```





**Parameters:**


* `dds` Pointer to DDS instance. 
* `buf` Output 16-bit DAC buffer. 
* `count` Number of samples. 
* `dac_center` Center DAC value (e.g. 2048 for 12-bit DAC). 
* `dac_span` Maximum peak-to-peak DAC range (e.g. 2047). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_dds\_init 

_Initialize a DDS Generator instance._ 
```C++
SYN_Status syn_dds_init (
    SYN_DDS * dds,
    SYN_DDS_Waveform type,
    uint32_t freq_hz,
    uint32_t sample_rate_hz
) 
```





**Parameters:**


* `dds` Pointer to DDS instance. 
* `type` Waveform type. 
* `freq_hz` Target output frequency in Hz. 
* `sample_rate_hz` Sample rate in Hz. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM. 





        

<hr>



### function syn\_dds\_set\_duty 

_Set PWM duty cycle (for SYN\_DDS\_SQUARE mode)._ 
```C++
SYN_Status syn_dds_set_duty (
    SYN_DDS * dds,
    float duty_pct
) 
```





**Parameters:**


* `dds` Pointer to DDS instance. 
* `duty_pct` Duty cycle percentage (0.0 to 100.0%). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_dds\_set\_freq 

_Set the output frequency of the DDS generator._ 
```C++
SYN_Status syn_dds_set_freq (
    SYN_DDS * dds,
    uint32_t freq_hz,
    uint32_t sample_rate_hz
) 
```





**Parameters:**


* `dds` Pointer to DDS instance. 
* `freq_hz` Output frequency in Hz. 
* `sample_rate_hz` Sample rate in Hz. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_dds\_set\_gain 

_Set the peak amplitude and DC offset._ 
```C++
SYN_Status syn_dds_set_gain (
    SYN_DDS * dds,
    q16_t amplitude,
    q16_t offset
) 
```





**Parameters:**


* `dds` Pointer to DDS instance. 
* `amplitude` Q16.16 peak amplitude. 
* `offset` Q16.16 DC offset. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_dds\_step 

_Compute and advance the DDS generator by one sample step._ 
```C++
q16_t syn_dds_step (
    SYN_DDS * dds
) 
```





**Parameters:**


* `dds` Pointer to DDS instance. 



**Returns:**

Generated sample in Q16.16 fixed-point format. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_dds.c`

