

# File syn\_dipswitch.c



[**FileList**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_dipswitch.c**](syn__dipswitch_8c.md)

[Go to the source code of this file](syn__dipswitch_8c_source.md)

_DIP Switch & Multi-Bit Rotary Selector Driver._ 

* `#include "syn_dipswitch.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**syn\_dipswitch\_get\_value**](#function-syn_dipswitch_get_value) (const [**SYN\_DipSwitch**](structSYN__DipSwitch.md) \* ds) <br>_Get the current combined integer bitmask value of switches._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_dipswitch\_has\_changed**](#function-syn_dipswitch_has_changed) ([**SYN\_DipSwitch**](structSYN__DipSwitch.md) \* ds) <br>_Check if DIP switch values changed since last read._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dipswitch\_init**](#function-syn_dipswitch_init) ([**SYN\_DipSwitch**](structSYN__DipSwitch.md) \* ds, const [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) \* pins, uint8\_t count, [**bool**](syn__defs_8h.md#enum-bool) active\_low) <br>_Initialize a DIP Switch instance._  |
|  void | [**syn\_dipswitch\_read**](#function-syn_dipswitch_read) ([**SYN\_DipSwitch**](structSYN__DipSwitch.md) \* ds) <br>_Read all DIP switch pins and update binary value._  |




























## Public Functions Documentation




### function syn\_dipswitch\_get\_value 

_Get the current combined integer bitmask value of switches._ 
```C++
uint32_t syn_dipswitch_get_value (
    const SYN_DipSwitch * ds
) 
```





**Parameters:**


* `ds` DIP switch context. 



**Returns:**

Bitmask where bit N corresponds to pin N. 





        

<hr>



### function syn\_dipswitch\_has\_changed 

_Check if DIP switch values changed since last read._ 
```C++
bool syn_dipswitch_has_changed (
    SYN_DipSwitch * ds
) 
```





**Parameters:**


* `ds` DIP switch context. 



**Returns:**

True if state changed. 





        

<hr>



### function syn\_dipswitch\_init 

_Initialize a DIP Switch instance._ 
```C++
SYN_Status syn_dipswitch_init (
    SYN_DipSwitch * ds,
    const SYN_GPIO_Pin * pins,
    uint8_t count,
    bool active_low
) 
```



Configures input GPIO pins with internal pull-ups or pull-downs.




**Parameters:**


* `ds` DIP switch context. 
* `pins` Array of GPIO pin identifiers. 
* `count` Number of switch pins (1 to 16). 
* `active_low` True if switch ON connects pin to GND (uses Pull-UP). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_dipswitch\_read 

_Read all DIP switch pins and update binary value._ 
```C++
void syn_dipswitch_read (
    SYN_DipSwitch * ds
) 
```





**Parameters:**


* `ds` DIP switch context. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/input/syn_dipswitch.c`

