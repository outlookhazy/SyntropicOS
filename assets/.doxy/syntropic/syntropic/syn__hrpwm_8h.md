

# File syn\_hrpwm.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_hrpwm.h**](syn__hrpwm_8h.md)

[Go to the source code of this file](syn__hrpwm_8h_source.md)

_High-Resolution Power & Precision Motion Control Driver._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_hrpwm.h"`
* `#include <stdbool.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_HRPWM**](structSYN__HRPWM.md) <br>_High-Resolution PWM handle. Caller allocates; zero heap._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_hrpwm\_bind\_fault**](#function-syn_hrpwm_bind_fault) (const [**SYN\_HRPWM**](structSYN__HRPWM.md) \* hrpwm, uint8\_t fault\_id) <br>_Bind hardware emergency fault trip protection._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_hrpwm\_enable**](#function-syn_hrpwm_enable) ([**SYN\_HRPWM**](structSYN__HRPWM.md) \* hrpwm, [**bool**](syn__defs_8h.md#enum-bool) enable) <br>_Enable or disable complementary HRPWM gate drivers._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_hrpwm\_init**](#function-syn_hrpwm_init) ([**SYN\_HRPWM**](structSYN__HRPWM.md) \* hrpwm, uint8\_t channel, uint32\_t freq\_hz) <br>_Initialize a High-Resolution PWM driver channel._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_hrpwm\_set\_deadtime\_ns**](#function-syn_hrpwm_set_deadtime_ns) ([**SYN\_HRPWM**](structSYN__HRPWM.md) \* hrpwm, uint16\_t rise\_ns, uint16\_t fall\_ns) <br>_Configure complementary dead-time insertion._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_hrpwm\_set\_duty\_float**](#function-syn_hrpwm_set_duty_float) (const [**SYN\_HRPWM**](structSYN__HRPWM.md) \* hrpwm, float duty\_float) <br>_Set high-resolution duty cycle ratio using float (0.0 to 1.0)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_hrpwm\_set\_duty\_q16**](#function-syn_hrpwm_set_duty_q16) (const [**SYN\_HRPWM**](structSYN__HRPWM.md) \* hrpwm, int32\_t duty\_q16) <br>_Set high-resolution duty cycle ratio using Q16.16 fixed-point format._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_hrpwm\_set\_phase\_deg**](#function-syn_hrpwm_set_phase_deg) ([**SYN\_HRPWM**](structSYN__HRPWM.md) \* hrpwm, uint16\_t phase\_deg) <br>_Set channel phase shift offset in degrees._  |




























## Detailed Description


Provides a vendor-agnostic, zero-allocation OS interface for sub-nanosecond PWM edge placement, complementary high-side/low-side gate driver control, hardware dead-time insertion, phase shifting, and emergency fault trips.


Usage: 
```C++
SYN_HRPWM hrpwm;
syn_hrpwm_init(&hrpwm, 0, 500000);                     // 500 kHz switching frequency
syn_hrpwm_set_deadtime_ns(&hrpwm, 100, 100);          // 100 ns dead-time
syn_hrpwm_set_duty_q16(&hrpwm, Q16_FROM_FLOAT(0.50f)); // 50% duty cycle
syn_hrpwm_enable(&hrpwm, true);
```
 


    
## Public Functions Documentation




### function syn\_hrpwm\_bind\_fault 

_Bind hardware emergency fault trip protection._ 
```C++
SYN_Status syn_hrpwm_bind_fault (
    const SYN_HRPWM * hrpwm,
    uint8_t fault_id
) 
```





**Parameters:**


* `hrpwm` Initialized HRPWM handle. 
* `fault_id` Hardware fault pin/comparator ID. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_hrpwm\_enable 

_Enable or disable complementary HRPWM gate drivers._ 
```C++
SYN_Status syn_hrpwm_enable (
    SYN_HRPWM * hrpwm,
    bool enable
) 
```





**Parameters:**


* `hrpwm` Initialized HRPWM handle. 
* `enable` true to enable outputs, false to disable. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_hrpwm\_init 

_Initialize a High-Resolution PWM driver channel._ 
```C++
SYN_Status syn_hrpwm_init (
    SYN_HRPWM * hrpwm,
    uint8_t channel,
    uint32_t freq_hz
) 
```





**Parameters:**


* `hrpwm` Handle to initialize. Must not be NULL. 
* `channel` Platform channel index. 
* `freq_hz` Target switching frequency in Hz. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid argument. 





        

<hr>



### function syn\_hrpwm\_set\_deadtime\_ns 

_Configure complementary dead-time insertion._ 
```C++
SYN_Status syn_hrpwm_set_deadtime_ns (
    SYN_HRPWM * hrpwm,
    uint16_t rise_ns,
    uint16_t fall_ns
) 
```





**Parameters:**


* `hrpwm` Initialized HRPWM handle. 
* `rise_ns` High-side rise dead-time in nanoseconds. 
* `fall_ns` Low-side fall dead-time in nanoseconds. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_hrpwm\_set\_duty\_float 

_Set high-resolution duty cycle ratio using float (0.0 to 1.0)._ 
```C++
SYN_Status syn_hrpwm_set_duty_float (
    const SYN_HRPWM * hrpwm,
    float duty_float
) 
```





**Parameters:**


* `hrpwm` Initialized HRPWM handle. 
* `duty_float` Duty ratio (0.0f = 0%, 1.0f = 100%). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_hrpwm\_set\_duty\_q16 

_Set high-resolution duty cycle ratio using Q16.16 fixed-point format._ 
```C++
SYN_Status syn_hrpwm_set_duty_q16 (
    const SYN_HRPWM * hrpwm,
    int32_t duty_q16
) 
```





**Parameters:**


* `hrpwm` Initialized HRPWM handle. 
* `duty_q16` Duty ratio in Q16.16 (0 = 0%, 65536 = 100%). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_hrpwm\_set\_phase\_deg 

_Set channel phase shift offset in degrees._ 
```C++
SYN_Status syn_hrpwm_set_phase_deg (
    SYN_HRPWM * hrpwm,
    uint16_t phase_deg
) 
```





**Parameters:**


* `hrpwm` Initialized HRPWM handle. 
* `phase_deg` Phase angle offset in degrees (0–360). 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_hrpwm.h`

