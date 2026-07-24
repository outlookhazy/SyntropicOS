

# File syn\_led.c



[**FileList**](files.md) **>** [**output**](dir_1e1a1e0352188cb1bf4d84e529f1e02b.md) **>** [**syn\_led.c**](syn__led_8c.md)

[Go to the source code of this file](syn__led_8c_source.md)

_LED controller implementation._ 

* `#include "syn_led.h"`
* `#include "../drivers/syn_gpio.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_led\_blink**](#function-syn_led_blink) ([**SYN\_LED**](structSYN__LED.md) \* led, uint16\_t on\_ms, uint16\_t off\_ms) <br>_Start a continuous blink._  |
|  void | [**syn\_led\_flash**](#function-syn_led_flash) ([**SYN\_LED**](structSYN__LED.md) \* led, uint16\_t on\_ms, uint16\_t off\_ms, uint8\_t count) <br>_Flash the LED N times, then turn off._  |
|  void | [**syn\_led\_init**](#function-syn_led_init) ([**SYN\_LED**](structSYN__LED.md) \* led, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin, [**SYN\_LEDPolarity**](syn__led_8h.md#enum-syn_ledpolarity) polarity) <br>_Initialize an LED descriptor._  |
|  uint32\_t | [**syn\_led\_next\_ms**](#function-syn_led_next_ms) (const [**SYN\_LED**](structSYN__LED.md) \* led) <br>_Milliseconds until the next state transition._  |
|  void | [**syn\_led\_off**](#function-syn_led_off) ([**SYN\_LED**](structSYN__LED.md) \* led) <br>_Turn the LED off._  |
|  void | [**syn\_led\_on**](#function-syn_led_on) ([**SYN\_LED**](structSYN__LED.md) \* led) <br>_Turn the LED on (steady)._  |
|  void | [**syn\_led\_pattern**](#function-syn_led_pattern) ([**SYN\_LED**](structSYN__LED.md) \* led, const char \* pattern, uint16\_t unit\_ms) <br>_Play a pattern string._  |
|  void | [**syn\_led\_service**](#function-syn_led_service) ([**SYN\_LED**](structSYN__LED.md) \* leds, size\_t count) <br>_Service an array of LEDs._  |
|  void | [**syn\_led\_toggle**](#function-syn_led_toggle) ([**SYN\_LED**](structSYN__LED.md) \* led) <br>_Toggle the LED (only meaningful in ON/OFF modes)._  |
|  void | [**syn\_led\_update**](#function-syn_led_update) ([**SYN\_LED**](structSYN__LED.md) \* led) <br>_Update the LED state machine._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**led\_set\_output**](#function-led_set_output) ([**SYN\_LED**](structSYN__LED.md) \* led, [**bool**](syn__defs_8h.md#enum-bool) on) <br>_Set the LED GPIO output based on polarity._  |


























## Public Functions Documentation




### function syn\_led\_blink 

_Start a continuous blink._ 
```C++
void syn_led_blink (
    SYN_LED * led,
    uint16_t on_ms,
    uint16_t off_ms
) 
```





**Parameters:**


* `led` LED. 
* `on_ms` On time in milliseconds. 
* `off_ms` Off time in milliseconds. 




        

<hr>



### function syn\_led\_flash 

_Flash the LED N times, then turn off._ 
```C++
void syn_led_flash (
    SYN_LED * led,
    uint16_t on_ms,
    uint16_t off_ms,
    uint8_t count
) 
```





**Parameters:**


* `led` LED. 
* `on_ms` On time per flash (ms). 
* `off_ms` Off time between flashes (ms). 
* `count` Number of flashes. 




        

<hr>



### function syn\_led\_init 

_Initialize an LED descriptor._ 
```C++
void syn_led_init (
    SYN_LED * led,
    SYN_GPIO_Pin pin,
    SYN_LEDPolarity polarity
) 
```





**Parameters:**


* `led` LED to initialize. 
* `pin` GPIO pin number. 
* `polarity` Active-high or active-low. 




        

<hr>



### function syn\_led\_next\_ms 

_Milliseconds until the next state transition._ 
```C++
uint32_t syn_led_next_ms (
    const SYN_LED * led
) 
```



Returns 0 if a transition is overdue, UINT32\_MAX if the LED is static (ON or OFF). Useful for sleeping in a protothread:



```C++
uint32_t wait = syn_led_next_ms(&led);
if (wait > 0 && wait < UINT32_MAX)
    PT_TASK_DELAY_MS(pt, task, wait);
```





**Parameters:**


* `led` LED instance. 



**Returns:**

Milliseconds until the next toggle. 





        

<hr>



### function syn\_led\_off 

_Turn the LED off._ 
```C++
void syn_led_off (
    SYN_LED * led
) 
```





**Parameters:**


* `led` LED instance. 




        

<hr>



### function syn\_led\_on 

_Turn the LED on (steady)._ 
```C++
void syn_led_on (
    SYN_LED * led
) 
```





**Parameters:**


* `led` LED instance. 




        

<hr>



### function syn\_led\_pattern 

_Play a pattern string._ 
```C++
void syn_led_pattern (
    SYN_LED * led,
    const char * pattern,
    uint16_t unit_ms
) 
```



Pattern characters: '.' = short on (1 unit) '-' = long on (3 units) ' ' = pause (1 unit) '\|' = long pause (3 units)


The pattern repeats indefinitely. Set a null or empty string to stop.




**Parameters:**


* `led` LED. 
* `pattern` Null-terminated pattern string. 
* `unit_ms` Base time unit in milliseconds (e.g., 100). 




        

<hr>



### function syn\_led\_service 

_Service an array of LEDs._ 
```C++
void syn_led_service (
    SYN_LED * leds,
    size_t count
) 
```





**Parameters:**


* `leds` LED array. 
* `count` Number of LEDs. 




        

<hr>



### function syn\_led\_toggle 

_Toggle the LED (only meaningful in ON/OFF modes)._ 
```C++
void syn_led_toggle (
    SYN_LED * led
) 
```





**Parameters:**


* `led` LED instance. 




        

<hr>



### function syn\_led\_update 

_Update the LED state machine._ 
```C++
void syn_led_update (
    SYN_LED * led
) 
```



Call from your main loop or a scheduler task.




**Parameters:**


* `led` LED instance. 




        

<hr>
## Public Static Functions Documentation




### function led\_set\_output 

_Set the LED GPIO output based on polarity._ 
```C++
static void led_set_output (
    SYN_LED * led,
    bool on
) 
```





**Parameters:**


* `led` LED instance. 
* `on` true to light the LED. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/output/syn_led.c`

