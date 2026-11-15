

# File syn\_joystick.h



[**FileList**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_joystick.h**](syn__joystick_8h.md)

[Go to the source code of this file](syn__joystick_8h_source.md)

_Generic Analog Joystick & Multi-Axis Potentiometer Driver._ 

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Joystick**](structSYN__Joystick.md) <br>_Generic Analog Joystick Context._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_JoystickDir**](#enum-syn_joystickdir)  <br>_8-Way D-Pad Directional State._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_joystick\_feed\_adc**](#function-syn_joystick_feed_adc) ([**SYN\_Joystick**](structSYN__Joystick.md) \* joy, uint16\_t raw\_x, uint16\_t raw\_y, [**bool**](syn__defs_8h.md#enum-bool) btn) <br>_Feed raw ADC readings for X and Y axes._  |
|  [**SYN\_JoystickDir**](syn__joystick_8h.md#enum-syn_joystickdir) | [**syn\_joystick\_get\_dir**](#function-syn_joystick_get_dir) (const [**SYN\_Joystick**](structSYN__Joystick.md) \* joy) <br>_Get 8-way directional state._  |
|  int16\_t | [**syn\_joystick\_get\_x\_pct**](#function-syn_joystick_get_x_pct) (const [**SYN\_Joystick**](structSYN__Joystick.md) \* joy) <br>_Get X axis position percentage (-100% to +100%)._  |
|  int16\_t | [**syn\_joystick\_get\_y\_pct**](#function-syn_joystick_get_y_pct) (const [**SYN\_Joystick**](structSYN__Joystick.md) \* joy) <br>_Get Y axis position percentage (-100% to +100%)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_joystick\_init**](#function-syn_joystick_init) ([**SYN\_Joystick**](structSYN__Joystick.md) \* joy, uint16\_t center\_x, uint16\_t center\_y, uint16\_t adc\_max, uint16\_t deadband) <br>_Initialize Analog Joystick instance._  |




























## Public Types Documentation




### enum SYN\_JoystickDir 

_8-Way D-Pad Directional State._ 
```C++
enum SYN_JoystickDir {
    SYN_JOYSTICK_DIR_CENTER = 0,
    SYN_JOYSTICK_DIR_UP = 1,
    SYN_JOYSTICK_DIR_UP_RIGHT = 2,
    SYN_JOYSTICK_DIR_RIGHT = 3,
    SYN_JOYSTICK_DIR_DOWN_RIGHT = 4,
    SYN_JOYSTICK_DIR_DOWN = 5,
    SYN_JOYSTICK_DIR_DOWN_LEFT = 6,
    SYN_JOYSTICK_DIR_LEFT = 7,
    SYN_JOYSTICK_DIR_UP_LEFT = 8
};
```




<hr>
## Public Functions Documentation




### function syn\_joystick\_feed\_adc 

_Feed raw ADC readings for X and Y axes._ 
```C++
void syn_joystick_feed_adc (
    SYN_Joystick * joy,
    uint16_t raw_x,
    uint16_t raw_y,
    bool btn
) 
```





**Parameters:**


* `joy` Joystick context. 
* `raw_x` Raw X axis ADC sample. 
* `raw_y` Raw Y axis ADC sample. 
* `btn` Pushbutton logical state. 




        

<hr>



### function syn\_joystick\_get\_dir 

_Get 8-way directional state._ 
```C++
SYN_JoystickDir syn_joystick_get_dir (
    const SYN_Joystick * joy
) 
```





**Parameters:**


* `joy` Joystick context. 



**Returns:**

Directional state enum. 





        

<hr>



### function syn\_joystick\_get\_x\_pct 

_Get X axis position percentage (-100% to +100%)._ 
```C++
int16_t syn_joystick_get_x_pct (
    const SYN_Joystick * joy
) 
```





**Parameters:**


* `joy` Joystick context. 



**Returns:**

X percentage. 





        

<hr>



### function syn\_joystick\_get\_y\_pct 

_Get Y axis position percentage (-100% to +100%)._ 
```C++
int16_t syn_joystick_get_y_pct (
    const SYN_Joystick * joy
) 
```





**Parameters:**


* `joy` Joystick context. 



**Returns:**

Y percentage. 





        

<hr>



### function syn\_joystick\_init 

_Initialize Analog Joystick instance._ 
```C++
SYN_Status syn_joystick_init (
    SYN_Joystick * joy,
    uint16_t center_x,
    uint16_t center_y,
    uint16_t adc_max,
    uint16_t deadband
) 
```





**Parameters:**


* `joy` Joystick context. 
* `center_x` Expected center X raw ADC reading. 
* `center_y` Expected center Y raw ADC reading. 
* `adc_max` Max ADC full-scale value (e.g. 4095 for 12-bit ADC). 
* `deadband` Deadband threshold radius (e.g. 100). 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/input/syn_joystick.h`

