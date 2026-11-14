

# File syn\_foc\_observer.c



[**FileList**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_foc\_observer.c**](syn__foc__observer_8c.md)

[Go to the source code of this file](syn__foc__observer_8c_source.md)

_Sensorless FOC Rotor Position & Speed Sliding Mode Observer implementation._ 

* `#include "syn_foc_observer.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**syn\_foc\_observer\_get\_angle**](#function-syn_foc_observer_get_angle) (const [**SYN\_FOCObserver**](structSYN__FOCObserver.md) \* obs) <br>_Get current estimated rotor electrical angle._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**syn\_foc\_observer\_get\_speed**](#function-syn_foc_observer_get_speed) (const [**SYN\_FOCObserver**](structSYN__FOCObserver.md) \* obs) <br>_Get current estimated rotor electrical angular speed._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_foc\_observer\_init**](#function-syn_foc_observer_init) ([**SYN\_FOCObserver**](structSYN__FOCObserver.md) \* obs, const [**SYN\_FOCObserverConfig**](structSYN__FOCObserverConfig.md) \* cfg) <br>_Initialize the sensorless FOC Sliding Mode Observer._  |
|  void | [**syn\_foc\_observer\_reset**](#function-syn_foc_observer_reset) ([**SYN\_FOCObserver**](structSYN__FOCObserver.md) \* obs) <br>_Reset observer state to zero._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_foc\_observer\_update**](#function-syn_foc_observer_update) ([**SYN\_FOCObserver**](structSYN__FOCObserver.md) \* obs, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) V\_alpha, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) V\_beta, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) I\_alpha, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) I\_beta) <br>_Update observer with latest phase voltages and measured phase currents._  |




























## Public Functions Documentation




### function syn\_foc\_observer\_get\_angle 

_Get current estimated rotor electrical angle._ 
```C++
q16_t syn_foc_observer_get_angle (
    const SYN_FOCObserver * obs
) 
```





**Parameters:**


* `obs` Observer instance. 



**Returns:**

Estimated angle in radians (Q16.16, 0 to 2pi). 





        

<hr>



### function syn\_foc\_observer\_get\_speed 

_Get current estimated rotor electrical angular speed._ 
```C++
q16_t syn_foc_observer_get_speed (
    const SYN_FOCObserver * obs
) 
```





**Parameters:**


* `obs` Observer instance. 



**Returns:**

Estimated angular speed in rad/s (Q16.16). 





        

<hr>



### function syn\_foc\_observer\_init 

_Initialize the sensorless FOC Sliding Mode Observer._ 
```C++
SYN_Status syn_foc_observer_init (
    SYN_FOCObserver * obs,
    const SYN_FOCObserverConfig * cfg
) 
```





**Parameters:**


* `obs` Pointer to observer instance. 
* `cfg` Observer configuration parameters. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL or invalid parameters. 





        

<hr>



### function syn\_foc\_observer\_reset 

_Reset observer state to zero._ 
```C++
void syn_foc_observer_reset (
    SYN_FOCObserver * obs
) 
```





**Parameters:**


* `obs` Observer instance. 




        

<hr>



### function syn\_foc\_observer\_update 

_Update observer with latest phase voltages and measured phase currents._ 
```C++
SYN_Status syn_foc_observer_update (
    SYN_FOCObserver * obs,
    q16_t V_alpha,
    q16_t V_beta,
    q16_t I_alpha,
    q16_t I_beta
) 
```





**Parameters:**


* `obs` Observer instance. 
* `V_alpha` Applied alpha-axis voltage in Volts (Q16.16). 
* `V_beta` Applied beta-axis voltage in Volts (Q16.16). 
* `I_alpha` Measured alpha-axis current in Amps (Q16.16). 
* `I_beta` Measured beta-axis current in Amps (Q16.16). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_foc_observer.c`

