

# File syn\_soft\_pwm.c



[**FileList**](files.md) **>** [**output**](dir_1e1a1e0352188cb1bf4d84e529f1e02b.md) **>** [**syn\_soft\_pwm.c**](syn__soft__pwm_8c.md)

[Go to the source code of this file](syn__soft__pwm_8c_source.md)

_Software PWM implementation._ 

* `#include "syn_soft_pwm.h"`
* `#include "../drivers/syn_gpio.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_soft\_pwm\_init**](#function-syn_soft_pwm_init) ([**SYN\_SoftPWM**](structSYN__SoftPWM.md) \* pwm, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin, uint16\_t resolution) <br>_Initialize a soft PWM channel._  |
|  void | [**syn\_soft\_pwm\_service**](#function-syn_soft_pwm_service) ([**SYN\_SoftPWM**](structSYN__SoftPWM.md) \* channels, size\_t count) <br>_Service an array of PWM channels in one call._  |
|  void | [**syn\_soft\_pwm\_set\_duty**](#function-syn_soft_pwm_set_duty) ([**SYN\_SoftPWM**](structSYN__SoftPWM.md) \* pwm, uint16\_t duty) <br>_Set the duty cycle._  |
|  void | [**syn\_soft\_pwm\_set\_percent**](#function-syn_soft_pwm_set_percent) ([**SYN\_SoftPWM**](structSYN__SoftPWM.md) \* pwm, uint8\_t percent) <br>_Set duty cycle as a percentage (0–100)._  |
|  void | [**syn\_soft\_pwm\_tick**](#function-syn_soft_pwm_tick) ([**SYN\_SoftPWM**](structSYN__SoftPWM.md) \* pwm) <br>_Advance the PWM phase by one tick._  |




























## Public Functions Documentation




### function syn\_soft\_pwm\_init 

_Initialize a soft PWM channel._ 
```C++
void syn_soft_pwm_init (
    SYN_SoftPWM * pwm,
    SYN_GPIO_Pin pin,
    uint16_t resolution
) 
```





**Parameters:**


* `pwm` PWM instance to initialize. 
* `pin` GPIO pin to drive. 
* `resolution` Number of steps per PWM period (e.g., 100 or 256). Higher = finer duty control, but requires faster tick. 




        

<hr>



### function syn\_soft\_pwm\_service 

_Service an array of PWM channels in one call._ 
```C++
void syn_soft_pwm_service (
    SYN_SoftPWM * channels,
    size_t count
) 
```





**Parameters:**


* `channels` Array of PWM instances. 
* `count` Number of channels in the array. 




        

<hr>



### function syn\_soft\_pwm\_set\_duty 

_Set the duty cycle._ 
```C++
void syn_soft_pwm_set_duty (
    SYN_SoftPWM * pwm,
    uint16_t duty
) 
```





**Parameters:**


* `pwm` PWM instance. 
* `duty` Duty cycle value (0 = off, resolution = 100% on). 




        

<hr>



### function syn\_soft\_pwm\_set\_percent 

_Set duty cycle as a percentage (0–100)._ 
```C++
void syn_soft_pwm_set_percent (
    SYN_SoftPWM * pwm,
    uint8_t percent
) 
```





**Parameters:**


* `pwm` PWM instance. 
* `percent` Duty cycle percentage (0 to 100). 




        

<hr>



### function syn\_soft\_pwm\_tick 

_Advance the PWM phase by one tick._ 
```C++
void syn_soft_pwm_tick (
    SYN_SoftPWM * pwm
) 
```



Call this from a timer ISR or high-frequency polling loop. The tick rate determines the PWM frequency: PWM\_freq = tick\_rate / resolution


For example: 10 kHz tick with resolution=100 → 100 Hz PWM.




**Parameters:**


* `pwm` PWM instance. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/output/syn_soft_pwm.c`

