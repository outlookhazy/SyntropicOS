

# File syn\_actuator.c



[**FileList**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_actuator.c**](syn__actuator_8c.md)

[Go to the source code of this file](syn__actuator_8c_source.md)

_Linear actuator implementation._ 

* `#include "syn_actuator.h"`
* `#include "../motor/syn_dc_motor.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_actuator\_clear\_stall**](#function-syn_actuator_clear_stall) ([**SYN\_Actuator**](structSYN__Actuator.md) \* act) <br>_Clear stall condition and re-enable._  |
|  void | [**syn\_actuator\_init**](#function-syn_actuator_init) ([**SYN\_Actuator**](structSYN__Actuator.md) \* act, const [**SYN\_Actuator\_Config**](structSYN__Actuator__Config.md) \* cfg) <br>_Initialize linear actuator._  |
|  void | [**syn\_actuator\_set\_position**](#function-syn_actuator_set_position) ([**SYN\_Actuator**](structSYN__Actuator.md) \* act, int16\_t pct\_x10) <br>_Set desired position._  |
|  void | [**syn\_actuator\_stop**](#function-syn_actuator_stop) ([**SYN\_Actuator**](structSYN__Actuator.md) \* act) <br>_Stop the actuator immediately._  |
|  int16\_t | [**syn\_actuator\_update**](#function-syn_actuator_update) ([**SYN\_Actuator**](structSYN__Actuator.md) \* act) <br>_Update — read pot, run PID, drive motor._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  int16\_t | [**adc\_to\_pct**](#function-adc_to_pct) (const [**SYN\_Actuator**](structSYN__Actuator.md) \* act, int32\_t adc) <br>_Convert ADC value to position percentage._  |
|  int32\_t | [**pct\_to\_adc**](#function-pct_to_adc) (const [**SYN\_Actuator**](structSYN__Actuator.md) \* act, int16\_t pct) <br>_Convert position percentage (0.0–1000 = 0–100%) to ADC units._  |


























## Public Functions Documentation




### function syn\_actuator\_clear\_stall 

_Clear stall condition and re-enable._ 
```C++
void syn_actuator_clear_stall (
    SYN_Actuator * act
) 
```





**Parameters:**


* `act` Actuator. 




        

<hr>



### function syn\_actuator\_init 

_Initialize linear actuator._ 
```C++
void syn_actuator_init (
    SYN_Actuator * act,
    const SYN_Actuator_Config * cfg
) 
```





**Parameters:**


* `act` Actuator instance. 
* `cfg` Configuration. 




        

<hr>



### function syn\_actuator\_set\_position 

_Set desired position._ 
```C++
void syn_actuator_set_position (
    SYN_Actuator * act,
    int16_t pct_x10
) 
```





**Parameters:**


* `act` Actuator instance. 
* `pct_x10` Position in 0.1% units (0 = retracted, 1000 = extended). 




        

<hr>



### function syn\_actuator\_stop 

_Stop the actuator immediately._ 
```C++
void syn_actuator_stop (
    SYN_Actuator * act
) 
```





**Parameters:**


* `act` Actuator instance. 




        

<hr>



### function syn\_actuator\_update 

_Update — read pot, run PID, drive motor._ 
```C++
int16_t syn_actuator_update (
    SYN_Actuator * act
) 
```



Call from your scheduler task at the configured update\_hz.




**Parameters:**


* `act` Actuator instance. 



**Returns:**

Current position (0-1000). 





        

<hr>
## Public Static Functions Documentation




### function adc\_to\_pct 

_Convert ADC value to position percentage._ 
```C++
static int16_t adc_to_pct (
    const SYN_Actuator * act,
    int32_t adc
) 
```





**Parameters:**


* `act` Actuator instance. 
* `adc` ADC reading. 



**Returns:**

Position in 0.1% units (0–1000). 





        

<hr>



### function pct\_to\_adc 

_Convert position percentage (0.0–1000 = 0–100%) to ADC units._ 
```C++
static int32_t pct_to_adc (
    const SYN_Actuator * act,
    int16_t pct
) 
```





**Parameters:**


* `act` Actuator instance. 
* `pct` Position in 0.1% units (0–1000). 



**Returns:**

ADC value. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_actuator.c`

