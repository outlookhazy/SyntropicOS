

# File syn\_pwm.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_pwm.h**](syn__pwm_8h.md)

[Go to the source code of this file](syn__pwm_8h_source.md)

_Hardware PWM (Pulse Width Modulation) driver._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_pwm.h"`
* `#include <stdbool.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_PWM**](structSYN__PWM.md) <br>_PWM channel handle. Caller allocates; zero heap._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_pwm\_enable**](#function-syn_pwm_enable) (const [**SYN\_PWM**](structSYN__PWM.md) \* pwm, [**bool**](syn__defs_8h.md#enum-bool) enable) <br>_Enable or disable the PWM output._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_pwm\_init**](#function-syn_pwm_init) ([**SYN\_PWM**](structSYN__PWM.md) \* pwm, uint8\_t channel, uint32\_t freq\_hz) <br>_Initialize a hardware PWM channel._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_pwm\_set\_duty**](#function-syn_pwm_set_duty) (const [**SYN\_PWM**](structSYN__PWM.md) \* pwm, uint8\_t duty\_pct) <br>_Set the PWM duty cycle as a percentage (0–100%)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_pwm\_set\_duty\_raw**](#function-syn_pwm_set_duty_raw) (const [**SYN\_PWM**](structSYN__PWM.md) \* pwm, uint16\_t duty\_u16) <br>_Set the PWM duty cycle with 16-bit raw resolution (0–65535)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_pwm\_set\_freq**](#function-syn_pwm_set_freq) ([**SYN\_PWM**](structSYN__PWM.md) \* pwm, uint32\_t freq\_hz) <br>_Change the PWM frequency at runtime._  |




























## Detailed Description


Provides a high-level, object-oriented wrapper around the platform's hardware PWM port interface ([**syn\_port\_pwm.h**](syn__port__pwm_8h.md)). Mirrors the pattern of [**syn\_dac.h**](syn__dac_8h.md).


Usage: 
```C++
SYN_PWM pwm;
syn_pwm_init(&pwm, 0, 1000);        // Channel 0 @ 1 kHz
syn_pwm_set_duty(&pwm, 75);         // 75% duty cycle
syn_pwm_enable(&pwm, true);
```
 


    
## Public Functions Documentation




### function syn\_pwm\_enable 

_Enable or disable the PWM output._ 
```C++
SYN_Status syn_pwm_enable (
    const SYN_PWM * pwm,
    bool enable
) 
```





**Parameters:**


* `pwm` Initialized PWM handle. 
* `enable` true to enable, false to disable. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if handle is invalid. 





        

<hr>



### function syn\_pwm\_init 

_Initialize a hardware PWM channel._ 
```C++
SYN_Status syn_pwm_init (
    SYN_PWM * pwm,
    uint8_t channel,
    uint32_t freq_hz
) 
```





**Parameters:**


* `pwm` Handle to initialize. Must not be NULL. 
* `channel` Platform PWM channel index. 
* `freq_hz` Desired PWM frequency in Hz (must be &gt; 0). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on failure. 





        

<hr>



### function syn\_pwm\_set\_duty 

_Set the PWM duty cycle as a percentage (0–100%)._ 
```C++
SYN_Status syn_pwm_set_duty (
    const SYN_PWM * pwm,
    uint8_t duty_pct
) 
```





**Parameters:**


* `pwm` Initialized PWM handle. 
* `duty_pct` Duty cycle percentage (0 = 0%, 100 = 100%). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if handle is invalid. 





        

<hr>



### function syn\_pwm\_set\_duty\_raw 

_Set the PWM duty cycle with 16-bit raw resolution (0–65535)._ 
```C++
SYN_Status syn_pwm_set_duty_raw (
    const SYN_PWM * pwm,
    uint16_t duty_u16
) 
```





**Parameters:**


* `pwm` Initialized PWM handle. 
* `duty_u16` 16-bit duty value (0 = off, 65535 = 100%). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if handle is invalid. 





        

<hr>



### function syn\_pwm\_set\_freq 

_Change the PWM frequency at runtime._ 
```C++
SYN_Status syn_pwm_set_freq (
    SYN_PWM * pwm,
    uint32_t freq_hz
) 
```





**Parameters:**


* `pwm` Initialized PWM handle. 
* `freq_hz` New frequency in Hz (must be &gt; 0). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if handle is invalid. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_pwm.h`

