

# File syn\_port\_pwm.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_pwm.h**](syn__port__pwm_8h.md)

[Go to the source code of this file](syn__port__pwm_8h_source.md)

_Hardware PWM port interface — implement these for your platform._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_port\_pwm\_enable**](#function-syn_port_pwm_enable) (uint8\_t channel, [**bool**](syn__defs_8h.md#enum-bool) enable) <br>_Enable/disable PWM output._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_pwm\_init**](#function-syn_port_pwm_init) (uint8\_t channel, uint32\_t freq\_hz) <br>_Initialize a PWM channel._  |
|  void | [**syn\_port\_pwm\_set\_duty**](#function-syn_port_pwm_set_duty) (uint8\_t channel, uint8\_t duty\_pct) <br>_Set PWM duty cycle._  |
|  void | [**syn\_port\_pwm\_set\_duty\_raw**](#function-syn_port_pwm_set_duty_raw) (uint8\_t channel, uint16\_t duty\_u16) <br>_Set PWM duty cycle with fine resolution._  |
|  void | [**syn\_port\_pwm\_set\_freq**](#function-syn_port_pwm_set_freq) (uint8\_t channel, uint32\_t freq\_hz) <br>_Set PWM frequency (runtime change)._  |




























## Detailed Description


Provides channel-based PWM control. Each channel maps to a physical timer/output pin on your MCU. 


    
## Public Functions Documentation




### function syn\_port\_pwm\_enable 

_Enable/disable PWM output._ 
```C++
void syn_port_pwm_enable (
    uint8_t channel,
    bool enable
) 
```





**Parameters:**


* `channel` PWM channel index. 
* `enable` true to enable, false to disable. 




        

<hr>



### function syn\_port\_pwm\_init 

_Initialize a PWM channel._ 
```C++
SYN_Status syn_port_pwm_init (
    uint8_t channel,
    uint32_t freq_hz
) 
```





**Parameters:**


* `channel` PWM channel number. 
* `freq_hz` PWM frequency in Hz. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_pwm\_set\_duty 

_Set PWM duty cycle._ 
```C++
void syn_port_pwm_set_duty (
    uint8_t channel,
    uint8_t duty_pct
) 
```





**Parameters:**


* `channel` PWM channel number. 
* `duty_pct` Duty cycle 0–100 (percent). 




        

<hr>



### function syn\_port\_pwm\_set\_duty\_raw 

_Set PWM duty cycle with fine resolution._ 
```C++
void syn_port_pwm_set_duty_raw (
    uint8_t channel,
    uint16_t duty_u16
) 
```





**Parameters:**


* `channel` PWM channel number. 
* `duty_u16` Duty cycle 0–65535 (0 = off, 65535 = 100%). 




        

<hr>



### function syn\_port\_pwm\_set\_freq 

_Set PWM frequency (runtime change)._ 
```C++
void syn_port_pwm_set_freq (
    uint8_t channel,
    uint32_t freq_hz
) 
```





**Parameters:**


* `channel` PWM channel number. 
* `freq_hz` New frequency in Hz. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_pwm.h`

