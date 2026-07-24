

# File syn\_button.h



[**FileList**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_button.h**](syn__button_8h.md)

[Go to the source code of this file](syn__button_8h_source.md)

_Debounced button input with press/release/long-press/repeat._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../drivers/syn_gpio.h"`
* `#include "../port/syn_port_system.h"`
* `#include "../util/syn_fsm.h"`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Button**](structSYN__Button.md) <br>_Button descriptor — owns the FSM, debounce, and callback state._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_ButtonCallback**](#typedef-syn_buttoncallback)  <br>_Button event callback._  |
| enum  | [**SYN\_ButtonPolarity**](#enum-syn_buttonpolarity)  <br>_Button polarity selector._  |
| enum  | [**SYN\_ButtonState**](#enum-syn_buttonstate)  <br>_Button FSM states._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_button\_init**](#function-syn_button_init) ([**SYN\_Button**](structSYN__Button.md) \* btn, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin, [**SYN\_ButtonPolarity**](syn__button_8h.md#enum-syn_buttonpolarity) polarity, uint16\_t debounce\_ms) <br>_Initialize a button descriptor._  |
|  void | [**syn\_button\_on\_long\_press**](#function-syn_button_on_long_press) ([**SYN\_Button**](structSYN__Button.md) \* btn, [**SYN\_ButtonCallback**](syn__button_8h.md#typedef-syn_buttoncallback) cb, uint16\_t hold\_ms, void \* ctx) <br>_Register a long-press callback._  |
|  void | [**syn\_button\_on\_press**](#function-syn_button_on_press) ([**SYN\_Button**](structSYN__Button.md) \* btn, [**SYN\_ButtonCallback**](syn__button_8h.md#typedef-syn_buttoncallback) cb, void \* ctx) <br>_Register a press callback._  |
|  void | [**syn\_button\_on\_release**](#function-syn_button_on_release) ([**SYN\_Button**](structSYN__Button.md) \* btn, [**SYN\_ButtonCallback**](syn__button_8h.md#typedef-syn_buttoncallback) cb, void \* ctx) <br>_Register a release callback._  |
|  void | [**syn\_button\_on\_repeat**](#function-syn_button_on_repeat) ([**SYN\_Button**](structSYN__Button.md) \* btn, [**SYN\_ButtonCallback**](syn__button_8h.md#typedef-syn_buttoncallback) cb, uint16\_t interval\_ms, void \* ctx) <br>_Register an auto-repeat callback._  |
|  void | [**syn\_button\_service**](#function-syn_button_service) ([**SYN\_Button**](structSYN__Button.md) \* buttons, size\_t count) <br>_Service an array of buttons._  |
|  void | [**syn\_button\_update**](#function-syn_button_update) ([**SYN\_Button**](structSYN__Button.md) \* btn) <br>_Update a single button's state machine._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**syn\_button\_held\_ms**](#function-syn_button_held_ms) (const [**SYN\_Button**](structSYN__Button.md) \* btn) <br>_How long has the button been in its current state (ms)?_  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_button\_is\_pressed**](#function-syn_button_is_pressed) (const [**SYN\_Button**](structSYN__Button.md) \* btn) <br>_Is the button currently pressed (debounced)?_  |
|  uint8\_t | [**syn\_button\_poll\_events**](#function-syn_button_poll_events) ([**SYN\_Button**](structSYN__Button.md) \* btn) <br>_Read and clear pending events (bitmask of SYN\_BUTTON\_EVT\_\*)._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**PT\_WAIT\_BUTTON\_PRESS**](syn__button_8h.md#define-pt_wait_button_press) (pt, btn) `/* multi line expression */`<br> |
| define  | [**PT\_WAIT\_BUTTON\_RELEASE**](syn__button_8h.md#define-pt_wait_button_release) (pt, btn) `/* multi line expression */`<br> |
| define  | [**SYN\_BUTTON\_EVT\_LONG\_PRESS**](syn__button_8h.md#define-syn_button_evt_long_press)  `((uint8\_t)(1u &lt;&lt; 2))`<br> |
| define  | [**SYN\_BUTTON\_EVT\_PRESS**](syn__button_8h.md#define-syn_button_evt_press)  `((uint8\_t)(1u &lt;&lt; 0))`<br> |
| define  | [**SYN\_BUTTON\_EVT\_RELEASE**](syn__button_8h.md#define-syn_button_evt_release)  `((uint8\_t)(1u &lt;&lt; 1))`<br> |
| define  | [**SYN\_BUTTON\_EVT\_REPEAT**](syn__button_8h.md#define-syn_button_evt_repeat)  `((uint8\_t)(1u &lt;&lt; 3))`<br> |

## Detailed Description


Pure polling — no interrupts required. Call [**syn\_button\_update()**](syn__button_8h.md#function-syn_button_update) (or [**syn\_button\_service()**](syn__button_8h.md#function-syn_button_service) for an array) from your main loop or a scheduler task. The module handles debouncing, edge detection, and long-press timing internally.




**
**


```C++
static SYN_Button btn;
syn_button_init(&btn, PIN_BUTTON, SYN_BUTTON_ACTIVE_LOW, 50);
syn_button_on_press(&btn, my_press_handler, NULL);
syn_button_on_long_press(&btn, my_long_handler, 1000, NULL);

// Main loop:
while (1) { syn_button_update(&btn); }

// Or in a protothread:
PT_WAIT_BUTTON_PRESS(pt, &btn);
```
 





    
## Public Types Documentation




### typedef SYN\_ButtonCallback 

_Button event callback._ 
```C++
typedef void(* SYN_ButtonCallback) (struct SYN_Button *btn, void *user_data);
```





**Parameters:**


* `btn` The button that generated the event. 
* `user_data` User-provided context pointer. 




        

<hr>



### enum SYN\_ButtonPolarity 

_Button polarity selector._ 
```C++
enum SYN_ButtonPolarity {
    SYN_BUTTON_ACTIVE_HIGH = 0,
    SYN_BUTTON_ACTIVE_LOW = 1
};
```




<hr>



### enum SYN\_ButtonState 

_Button FSM states._ 
```C++
enum SYN_ButtonState {
    SYN_BUTTON_STATE_IDLE = 0,
    SYN_BUTTON_STATE_DEBOUNCING = 1,
    SYN_BUTTON_STATE_PRESSED = 2,
    SYN_BUTTON_STATE_HELD = 3
};
```




<hr>
## Public Functions Documentation




### function syn\_button\_init 

_Initialize a button descriptor._ 
```C++
void syn_button_init (
    SYN_Button * btn,
    SYN_GPIO_Pin pin,
    SYN_ButtonPolarity polarity,
    uint16_t debounce_ms
) 
```





**Parameters:**


* `btn` Button to initialize. 
* `pin` GPIO pin number. 
* `polarity` Active-high or active-low. 
* `debounce_ms` Debounce window in milliseconds (e.g., 50). 




        

<hr>



### function syn\_button\_on\_long\_press 

_Register a long-press callback._ 
```C++
void syn_button_on_long_press (
    SYN_Button * btn,
    SYN_ButtonCallback cb,
    uint16_t hold_ms,
    void * ctx
) 
```





**Parameters:**


* `btn` Button. 
* `cb` Callback (or NULL to disable). 
* `hold_ms` Time the button must be held before firing (ms). 
* `ctx` User context. 




        

<hr>



### function syn\_button\_on\_press 

_Register a press callback._ 
```C++
void syn_button_on_press (
    SYN_Button * btn,
    SYN_ButtonCallback cb,
    void * ctx
) 
```





**Parameters:**


* `btn` Button. 
* `cb` Callback (or NULL to disable). 
* `ctx` User context. 




        

<hr>



### function syn\_button\_on\_release 

_Register a release callback._ 
```C++
void syn_button_on_release (
    SYN_Button * btn,
    SYN_ButtonCallback cb,
    void * ctx
) 
```





**Parameters:**


* `btn` Button. 
* `cb` Callback (or NULL to disable). 
* `ctx` User context. 




        

<hr>



### function syn\_button\_on\_repeat 

_Register an auto-repeat callback._ 
```C++
void syn_button_on_repeat (
    SYN_Button * btn,
    SYN_ButtonCallback cb,
    uint16_t interval_ms,
    void * ctx
) 
```



Fires repeatedly while the button is held, after the initial press.




**Parameters:**


* `btn` Button. 
* `cb` Callback (or NULL to disable). 
* `interval_ms` Repeat interval in milliseconds. 
* `ctx` User context. 




        

<hr>



### function syn\_button\_service 

_Service an array of buttons._ 
```C++
void syn_button_service (
    SYN_Button * buttons,
    size_t count
) 
```





**Parameters:**


* `buttons` Array of buttons. 
* `count` Number of buttons. 




        

<hr>



### function syn\_button\_update 

_Update a single button's state machine._ 
```C++
void syn_button_update (
    SYN_Button * btn
) 
```



Call this from your main loop, a scheduler task, or a timer callback. Reads the GPIO, runs debouncing, and fires callbacks as needed.




**Parameters:**


* `btn` Button to update. 




        

<hr>
## Public Static Functions Documentation




### function syn\_button\_held\_ms 

_How long has the button been in its current state (ms)?_ 
```C++
static inline uint32_t syn_button_held_ms (
    const SYN_Button * btn
) 
```





**Parameters:**


* `btn` Button. 



**Returns:**

Duration in milliseconds. 





        

<hr>



### function syn\_button\_is\_pressed 

_Is the button currently pressed (debounced)?_ 
```C++
static inline bool syn_button_is_pressed (
    const SYN_Button * btn
) 
```





**Parameters:**


* `btn` Button. 



**Returns:**

true if pressed. 





        

<hr>



### function syn\_button\_poll\_events 

_Read and clear pending events (bitmask of SYN\_BUTTON\_EVT\_\*)._ 
```C++
static inline uint8_t syn_button_poll_events (
    SYN_Button * btn
) 
```



Useful for polling instead of callbacks.




**Parameters:**


* `btn` Button. 



**Returns:**

Bitmask of events that occurred since last poll. 





        

<hr>
## Macro Definition Documentation





### define PT\_WAIT\_BUTTON\_PRESS 

```C++
#define PT_WAIT_BUTTON_PRESS (
    pt,
    btn
) `/* multi line expression */`
```



Block until the button is pressed (debounced). 


        

<hr>



### define PT\_WAIT\_BUTTON\_RELEASE 

```C++
#define PT_WAIT_BUTTON_RELEASE (
    pt,
    btn
) `/* multi line expression */`
```



Block until the button is released. 


        

<hr>



### define SYN\_BUTTON\_EVT\_LONG\_PRESS 

```C++
#define SYN_BUTTON_EVT_LONG_PRESS `((uint8_t)(1u << 2))`
```



Long press detected 


        

<hr>



### define SYN\_BUTTON\_EVT\_PRESS 

```C++
#define SYN_BUTTON_EVT_PRESS `((uint8_t)(1u << 0))`
```



Button pressed 
 


        

<hr>



### define SYN\_BUTTON\_EVT\_RELEASE 

```C++
#define SYN_BUTTON_EVT_RELEASE `((uint8_t)(1u << 1))`
```



Button released 
 


        

<hr>



### define SYN\_BUTTON\_EVT\_REPEAT 

```C++
#define SYN_BUTTON_EVT_REPEAT `((uint8_t)(1u << 3))`
```



Auto-repeat fire 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/input/syn_button.h`

