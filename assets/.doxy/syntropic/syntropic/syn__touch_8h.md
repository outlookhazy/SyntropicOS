

# File syn\_touch.h



[**FileList**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_touch.h**](syn__touch_8h.md)

[Go to the source code of this file](syn__touch_8h_source.md)

_Capacitive Touch Sensor Driver (relaxation/ADC charge sensing)._ 

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_gpio.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Touch**](structSYN__Touch.md) <br>_Capacitive Touch Button Instance Context._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_touch\_calibrate**](#function-syn_touch_calibrate) ([**SYN\_Touch**](structSYN__Touch.md) \* touch, uint16\_t baseline) <br>_Calibrate baseline value from current environmental noise floor._  |
|  void | [**syn\_touch\_feed\_sample**](#function-syn_touch_feed_sample) ([**SYN\_Touch**](structSYN__Touch.md) \* touch, uint16\_t raw\_sample) <br>_Feed a raw capacitance charge-time / ADC sample._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_touch\_init**](#function-syn_touch_init) ([**SYN\_Touch**](structSYN__Touch.md) \* touch, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin, uint16\_t threshold) <br>_Initialize a Capacitive Touch Sensor instance._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_touch\_is\_pressed**](#function-syn_touch_is_pressed) (const [**SYN\_Touch**](structSYN__Touch.md) \* touch) <br>_Check if touch sensor is pressed._  |




























## Public Functions Documentation




### function syn\_touch\_calibrate 

_Calibrate baseline value from current environmental noise floor._ 
```C++
void syn_touch_calibrate (
    SYN_Touch * touch,
    uint16_t baseline
) 
```





**Parameters:**


* `touch` Touch context. 
* `baseline` Baseline raw value. 




        

<hr>



### function syn\_touch\_feed\_sample 

_Feed a raw capacitance charge-time / ADC sample._ 
```C++
void syn_touch_feed_sample (
    SYN_Touch * touch,
    uint16_t raw_sample
) 
```





**Parameters:**


* `touch` Touch context. 
* `raw_sample` Raw charge time / capacitance sample. 




        

<hr>



### function syn\_touch\_init 

_Initialize a Capacitive Touch Sensor instance._ 
```C++
SYN_Status syn_touch_init (
    SYN_Touch * touch,
    SYN_GPIO_Pin pin,
    uint16_t threshold
) 
```





**Parameters:**


* `touch` Touch context. 
* `pin` GPIO pin identifier. 
* `threshold` Touch detection delta threshold. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_touch\_is\_pressed 

_Check if touch sensor is pressed._ 
```C++
bool syn_touch_is_pressed (
    const SYN_Touch * touch
) 
```





**Parameters:**


* `touch` Touch context. 



**Returns:**

True if currently touched/pressed. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/input/syn_touch.h`

