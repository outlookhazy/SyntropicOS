

# File syn\_led.h



[**FileList**](files.md) **>** [**output**](dir_1e1a1e0352188cb1bf4d84e529f1e02b.md) **>** [**syn\_led.h**](syn__led_8h.md)

[Go to the source code of this file](syn__led_8h_source.md)

_Non-blocking LED controller — blink, flash, patterns._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../drivers/syn_gpio.h"`
* `#include "../port/syn_port_system.h"`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_LED**](structSYN__LED.md) <br>_LED instance — pin, mode, blink/flash/pattern state._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_LEDMode**](#enum-syn_ledmode)  <br>_LED operating mode._  |
| enum  | [**SYN\_LEDPolarity**](#enum-syn_ledpolarity)  <br>_LED output polarity._  |




















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
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_led\_is\_on**](#function-syn_led_is_on) (const [**SYN\_LED**](structSYN__LED.md) \* led) <br>_Check if the LED is currently lit._  |


























## Detailed Description


Drives LEDs through GPIO with non-blocking blink/flash/pattern sequencing. No timers or tasks required — just call [**syn\_led\_update()**](syn__led_8h.md#function-syn_led_update) from your main loop.




**
**


```C++
static SYN_LED led;
syn_led_init(&led, LED_PIN, SYN_LED_ACTIVE_HIGH);
syn_led_blink(&led, 500, 500);     // 500ms on, 500ms off
while (1) { syn_led_update(&led); }
```
 





    
## Public Types Documentation




### enum SYN\_LEDMode 

_LED operating mode._ 
```C++
enum SYN_LEDMode {
    SYN_LED_MODE_OFF = 0,
    SYN_LED_MODE_ON = 1,
    SYN_LED_MODE_BLINK = 2,
    SYN_LED_MODE_FLASH = 3,
    SYN_LED_MODE_PATTERN = 4
};
```




<hr>



### enum SYN\_LEDPolarity 

_LED output polarity._ 
```C++
enum SYN_LEDPolarity {
    SYN_LED_ACTIVE_HIGH = 0,
    SYN_LED_ACTIVE_LOW = 1
};
```




<hr>
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




### function syn\_led\_is\_on 

_Check if the LED is currently lit._ 
```C++
static inline bool syn_led_is_on (
    const SYN_LED * led
) 
```





**Parameters:**


* `led` LED instance. 



**Returns:**

true if lit. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/output/syn_led.h`

