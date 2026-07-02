

# File syn\_gpio.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_gpio.c**](syn__gpio_8c.md)

[Go to the source code of this file](syn__gpio_8c_source.md)

_GPIO driver implementation._ 

* `#include "syn_gpio.h"`
* `#include "../util/syn_assert.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_gpio\_init\_multiple**](#function-syn_gpio_init_multiple) (const [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) \* pins, size\_t count, [**SYN\_GPIO\_Mode**](syn__defs_8h.md#enum-syn_gpio_mode) mode) <br>_Initialize multiple pins with the same mode._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_gpio\_write\_multiple**](#function-syn_gpio_write_multiple) (const [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) \* pins, size\_t count, [**SYN\_GPIO\_State**](syn__defs_8h.md#enum-syn_gpio_state) state) <br>_Write the same state to multiple pins._  |




























## Public Functions Documentation




### function syn\_gpio\_init\_multiple 

_Initialize multiple pins with the same mode._ 
```C++
SYN_Status syn_gpio_init_multiple (
    const SYN_GPIO_Pin * pins,
    size_t count,
    SYN_GPIO_Mode mode
) 
```





**Parameters:**


* `pins` Array of pin identifiers. 
* `count` Number of pins. 
* `mode` Mode to apply to all pins. 



**Returns:**

SYN\_OK if all succeeded, or the first error encountered. 





        

<hr>



### function syn\_gpio\_write\_multiple 

_Write the same state to multiple pins._ 
```C++
SYN_Status syn_gpio_write_multiple (
    const SYN_GPIO_Pin * pins,
    size_t count,
    SYN_GPIO_State state
) 
```





**Parameters:**


* `pins` Array of pin identifiers. 
* `count` Number of pins. 
* `state` State to write. 



**Returns:**

SYN\_OK if all succeeded, or the first error encountered. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_gpio.c`

