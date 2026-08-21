

# File syn\_port\_hrpwm.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_hrpwm.h**](syn__port__hrpwm_8h.md)

[Go to the source code of this file](syn__port__hrpwm_8h_source.md)

_Platform Port Interface for High-Resolution PWM & Power Driver._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../util/syn_qmath.h"`
* `#include <stdbool.h>`
* `#include <stdint.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_hrpwm\_enable**](#function-syn_port_hrpwm_enable) (uint8\_t channel, [**bool**](syn__defs_8h.md#enum-bool) enable) <br>_Enable or disable HRPWM complementary outputs._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_hrpwm\_enable\_fault**](#function-syn_port_hrpwm_enable_fault) (uint8\_t channel, uint8\_t fault\_id, [**bool**](syn__defs_8h.md#enum-bool) enable) <br>_Enable hardware emergency fault trip protection input._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_hrpwm\_init**](#function-syn_port_hrpwm_init) (uint8\_t channel, uint32\_t freq\_hz) <br>_Initialize high-resolution PWM hardware timer channel._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_hrpwm\_set\_deadtime\_ns**](#function-syn_port_hrpwm_set_deadtime_ns) (uint8\_t channel, uint16\_t rise\_ns, uint16\_t fall\_ns) <br>_Configure complementary dead-time insertion in nanoseconds._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_hrpwm\_set\_duty\_q16**](#function-syn_port_hrpwm_set_duty_q16) (uint8\_t channel, int32\_t duty\_q16) <br>_Set high-resolution duty cycle using Q16.16 fixed-point ratio._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_hrpwm\_set\_phase\_deg**](#function-syn_port_hrpwm_set_phase_deg) (uint8\_t channel, uint16\_t phase\_deg) <br>_Set phase shift angle in degrees (0–360)._  |




























## Detailed Description


Defines low-level target hardware binding for sub-nanosecond edge placement, complementary gate output pairs with dead-time insertion, phase shifting, and zero-latency hardware fault trip interlocks. 


    
## Public Functions Documentation




### function syn\_port\_hrpwm\_enable 

_Enable or disable HRPWM complementary outputs._ 
```C++
SYN_Status syn_port_hrpwm_enable (
    uint8_t channel,
    bool enable
) 
```





**Parameters:**


* `channel` Platform-specific HRPWM channel index. 
* `enable` true to enable complementary gate drivers, false to disable. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_hrpwm\_enable\_fault 

_Enable hardware emergency fault trip protection input._ 
```C++
SYN_Status syn_port_hrpwm_enable_fault (
    uint8_t channel,
    uint8_t fault_id,
    bool enable
) 
```





**Parameters:**


* `channel` Platform-specific HRPWM channel index. 
* `fault_id` Hardware fault pin/comparator ID. 
* `enable` true to enable hardware trip interlock. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_hrpwm\_init 

_Initialize high-resolution PWM hardware timer channel._ 
```C++
SYN_Status syn_port_hrpwm_init (
    uint8_t channel,
    uint32_t freq_hz
) 
```





**Parameters:**


* `channel` Platform-specific HRPWM channel index (0..5). 
* `freq_hz` Switching frequency in Hz. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on hardware error. 





        

<hr>



### function syn\_port\_hrpwm\_set\_deadtime\_ns 

_Configure complementary dead-time insertion in nanoseconds._ 
```C++
SYN_Status syn_port_hrpwm_set_deadtime_ns (
    uint8_t channel,
    uint16_t rise_ns,
    uint16_t fall_ns
) 
```





**Parameters:**


* `channel` Platform-specific HRPWM channel index. 
* `rise_ns` High-side turn-on rise dead-time in nanoseconds. 
* `fall_ns` Low-side turn-on fall dead-time in nanoseconds. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_hrpwm\_set\_duty\_q16 

_Set high-resolution duty cycle using Q16.16 fixed-point ratio._ 
```C++
SYN_Status syn_port_hrpwm_set_duty_q16 (
    uint8_t channel,
    int32_t duty_q16
) 
```





**Parameters:**


* `channel` Platform-specific HRPWM channel index. 
* `duty_q16` Q16.16 fixed point ratio (0 = 0.0, 65536 = 1.0). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_hrpwm\_set\_phase\_deg 

_Set phase shift angle in degrees (0–360)._ 
```C++
SYN_Status syn_port_hrpwm_set_phase_deg (
    uint8_t channel,
    uint16_t phase_deg
) 
```





**Parameters:**


* `channel` Platform-specific HRPWM channel index. 
* `phase_deg` Phase offset angle in degrees. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_hrpwm.h`

