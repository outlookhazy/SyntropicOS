

# File syn\_port\_gpio.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_gpio.h**](syn__port__gpio_8h.md)

[Go to the source code of this file](syn__port__gpio_8h_source.md)

_GPIO port interface — functions the user must implement._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_gpio\_deinit**](#function-syn_port_gpio_deinit) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin) <br>_De-initialize a GPIO pin, returning it to its reset state._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_gpio\_init**](#function-syn_port_gpio_init) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin, [**SYN\_GPIO\_Mode**](syn__defs_8h.md#enum-syn_gpio_mode) mode) <br>_Initialize a GPIO pin with the given mode._  |
|  [**SYN\_GPIO\_State**](syn__defs_8h.md#enum-syn_gpio_state) | [**syn\_port\_gpio\_read**](#function-syn_port_gpio_read) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin) <br>_Read the current logical state of a pin._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_gpio\_toggle**](#function-syn_port_gpio_toggle) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin) <br>_Toggle the state of an output pin._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_gpio\_write**](#function-syn_port_gpio_write) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin, [**SYN\_GPIO\_State**](syn__defs_8h.md#enum-syn_gpio_state) state) <br>_Write a logical state to an output pin._  |




























## Detailed Description


To use SyntropicOS GPIO functionality, provide strong definitions of every function declared here in your platform-specific source file.


If you compile [**syn\_port\_stubs.c**](syn__port__stubs_8c.md), any function you forget to implement will link to a weak stub that fires an assert, making integration mistakes visible at runtime. 


    
## Public Functions Documentation




### function syn\_port\_gpio\_deinit 

_De-initialize a GPIO pin, returning it to its reset state._ 
```C++
SYN_Status syn_port_gpio_deinit (
    SYN_GPIO_Pin pin
) 
```





**Parameters:**


* `pin` Pin to de-initialize. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_gpio\_init 

_Initialize a GPIO pin with the given mode._ 
```C++
SYN_Status syn_port_gpio_init (
    SYN_GPIO_Pin pin,
    SYN_GPIO_Mode mode
) 
```





**Parameters:**


* `pin` Platform-specific pin identifier. 
* `mode` Desired pin mode (input, output, etc.). 



**Returns:**

SYN\_OK on success, or an error code. 





        

<hr>



### function syn\_port\_gpio\_read 

_Read the current logical state of a pin._ 
```C++
SYN_GPIO_State syn_port_gpio_read (
    SYN_GPIO_Pin pin
) 
```





**Parameters:**


* `pin` Pin to read. 



**Returns:**

SYN\_GPIO\_HIGH or SYN\_GPIO\_LOW. 





        

<hr>



### function syn\_port\_gpio\_toggle 

_Toggle the state of an output pin._ 
```C++
SYN_Status syn_port_gpio_toggle (
    SYN_GPIO_Pin pin
) 
```





**Parameters:**


* `pin` Pin to toggle. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_gpio\_write 

_Write a logical state to an output pin._ 
```C++
SYN_Status syn_port_gpio_write (
    SYN_GPIO_Pin pin,
    SYN_GPIO_State state
) 
```





**Parameters:**


* `pin` Pin to write. 
* `state` SYN\_GPIO\_HIGH or SYN\_GPIO\_LOW. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_gpio.h`

