

# File syn\_button.c



[**FileList**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_button.c**](syn__button_8c.md)

[Go to the source code of this file](syn__button_8c_source.md)

_Debounced button implementation using syn\_fsm._ 

* `#include "syn_button.h"`
* `#include "../drivers/syn_gpio.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`

















## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**syn\_\_button\_8c\_1a06fc87d81c62e9abb8790b6e5713c55b**](#enum-syn__button_8c_1a06fc87d81c62e9abb8790b6e5713c55b)  <br> |






## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const [**SYN\_FSM\_Transition**](structSYN__FSM__Transition.md) | [**g\_button\_transitions**](#variable-g_button_transitions)   = `/* multi line expression */`<br>_Button FSM transition table._  |














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
|  void | [**action\_bounce\_back**](#function-action_bounce_back) (void \* ctx) <br>_FSM action: raw release during debounce → return to idle._  |
|  void | [**action\_confirm\_press**](#function-action_confirm_press) (void \* ctx) <br>_FSM action: debounce confirmed, fire press event._  |
|  void | [**action\_long\_press**](#function-action_long_press) (void \* ctx) <br>_FSM action: held long enough, fire long-press event._  |
|  void | [**action\_release**](#function-action_release) (void \* ctx) <br>_FSM action: button released, fire release event._  |
|  void | [**action\_start\_debounce**](#function-action_start_debounce) (void \* ctx) <br>_FSM action: start debounce timer on raw press._  |
|  void | [**button\_fire\_event**](#function-button_fire_event) ([**SYN\_Button**](structSYN__Button.md) \* btn, uint8\_t evt, [**SYN\_ButtonCallback**](syn__button_8h.md#typedef-syn_buttoncallback) cb, void \* ctx) <br>_Set an event flag and invoke the callback._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**button\_read\_raw**](#function-button_read_raw) (const [**SYN\_Button**](structSYN__Button.md) \* btn) <br>_Read the raw (debounce-agnostic) button state._  |


























## Public Types Documentation




### enum syn\_\_button\_8c\_1a06fc87d81c62e9abb8790b6e5713c55b 

```C++
enum syn__button_8c_1a06fc87d81c62e9abb8790b6e5713c55b {
    BTN_EV_RAW_PRESS = 0,
    BTN_EV_RAW_RELEASE,
    BTN_EV_DEBOUNCE_OK,
    BTN_EV_LONG_PRESS
};
```




<hr>
## Public Static Attributes Documentation




### variable g\_button\_transitions 

_Button FSM transition table._ 
```C++
const SYN_FSM_Transition g_button_transitions[];
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




### function action\_bounce\_back 

_FSM action: raw release during debounce → return to idle._ 
```C++
static void action_bounce_back (
    void * ctx
) 
```





**Parameters:**


* `ctx` Button (as void\*). 




        

<hr>



### function action\_confirm\_press 

_FSM action: debounce confirmed, fire press event._ 
```C++
static void action_confirm_press (
    void * ctx
) 
```





**Parameters:**


* `ctx` Button (as void\*). 




        

<hr>



### function action\_long\_press 

_FSM action: held long enough, fire long-press event._ 
```C++
static void action_long_press (
    void * ctx
) 
```





**Parameters:**


* `ctx` Button (as void\*). 




        

<hr>



### function action\_release 

_FSM action: button released, fire release event._ 
```C++
static void action_release (
    void * ctx
) 
```





**Parameters:**


* `ctx` Button (as void\*). 




        

<hr>



### function action\_start\_debounce 

_FSM action: start debounce timer on raw press._ 
```C++
static void action_start_debounce (
    void * ctx
) 
```





**Parameters:**


* `ctx` Button (as void\*). 




        

<hr>



### function button\_fire\_event 

_Set an event flag and invoke the callback._ 
```C++
static void button_fire_event (
    SYN_Button * btn,
    uint8_t evt,
    SYN_ButtonCallback cb,
    void * ctx
) 
```





**Parameters:**


* `btn` Button instance. 
* `evt` Event bitmask. 
* `cb` Callback (may be NULL). 
* `ctx` User context for callback. 




        

<hr>



### function button\_read\_raw 

_Read the raw (debounce-agnostic) button state._ 
```C++
static bool button_read_raw (
    const SYN_Button * btn
) 
```





**Parameters:**


* `btn` Button instance. 



**Returns:**

true if the GPIO indicates pressed. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/input/syn_button.c`

