

# File syn\_gpio.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_gpio.h**](syn__gpio_8h.md)

[Go to the source code of this file](syn__gpio_8h_source.md)

_GPIO driver — high-level convenience functions._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_gpio.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_gpio\_init\_multiple**](#function-syn_gpio_init_multiple) (const [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) \* pins, size\_t count, [**SYN\_GPIO\_Mode**](syn__defs_8h.md#enum-syn_gpio_mode) mode) <br>_Initialize multiple pins with the same mode._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_gpio\_write\_multiple**](#function-syn_gpio_write_multiple) (const [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) \* pins, size\_t count, [**SYN\_GPIO\_State**](syn__defs_8h.md#enum-syn_gpio_state) state) <br>_Write the same state to multiple pins._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_gpio\_init**](#function-syn_gpio_init) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin, [**SYN\_GPIO\_Mode**](syn__defs_8h.md#enum-syn_gpio_mode) mode) <br>_Initialize a GPIO pin._  |
|  [**SYN\_GPIO\_State**](syn__defs_8h.md#enum-syn_gpio_state) | [**syn\_gpio\_read**](#function-syn_gpio_read) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin) <br>_Read a pin's logical state._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_gpio\_toggle**](#function-syn_gpio_toggle) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin) <br>_Toggle a pin._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_gpio\_write**](#function-syn_gpio_write) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin, [**SYN\_GPIO\_State**](syn__defs_8h.md#enum-syn_gpio_state) state) <br>_Write a logical state to a pin._  |


























## Detailed Description


This module builds on the port layer ([**syn\_port\_gpio.h**](syn__port__gpio_8h.md)) and adds convenience helpers like bulk operations. For simple use cases you can call the port functions directly — this module is optional. 


    
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
## Public Static Functions Documentation




### function syn\_gpio\_init 

_Initialize a GPIO pin._ 
```C++
static inline SYN_Status syn_gpio_init (
    SYN_GPIO_Pin pin,
    SYN_GPIO_Mode mode
) 
```





**Parameters:**


* `pin` Pin identifier. 
* `mode` Pin mode (input, output, etc.). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_gpio\_read 

_Read a pin's logical state._ 
```C++
static inline SYN_GPIO_State syn_gpio_read (
    SYN_GPIO_Pin pin
) 
```





**Parameters:**


* `pin` Pin identifier. 



**Returns:**

Logical state. 





        

<hr>



### function syn\_gpio\_toggle 

_Toggle a pin._ 
```C++
static inline SYN_Status syn_gpio_toggle (
    SYN_GPIO_Pin pin
) 
```





**Parameters:**


* `pin` Pin identifier. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_gpio\_write 

_Write a logical state to a pin._ 
```C++
static inline SYN_Status syn_gpio_write (
    SYN_GPIO_Pin pin,
    SYN_GPIO_State state
) 
```





**Parameters:**


* `pin` Pin identifier. 
* `state` Logical state. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_gpio.h`

