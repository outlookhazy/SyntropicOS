

# Struct SYN\_Button



[**ClassList**](annotated.md) **>** [**SYN\_Button**](structSYN__Button.md)



_Button descriptor — owns the FSM, debounce, multi-click, and callback state._ 

* `#include <syn_button.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**click\_count**](#variable-click_count)  <br> |
|  uint32\_t | [**click\_tick**](#variable-click_tick)  <br> |
|  uint16\_t | [**debounce\_ms**](#variable-debounce_ms)  <br> |
|  uint16\_t | [**double\_click\_ms**](#variable-double_click_ms)  <br> |
|  uint8\_t | [**events**](#variable-events)  <br> |
|  [**SYN\_FSM**](structSYN__FSM.md) | [**fsm**](#variable-fsm)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**in\_combo**](#variable-in_combo)  <br> |
|  uint16\_t | [**long\_press\_ms**](#variable-long_press_ms)  <br> |
|  [**SYN\_ButtonCallback**](syn__button_8h.md#typedef-syn_buttoncallback) | [**on\_double\_click**](#variable-on_double_click)  <br> |
|  void \* | [**on\_double\_click\_ctx**](#variable-on_double_click_ctx)  <br> |
|  [**SYN\_ButtonCallback**](syn__button_8h.md#typedef-syn_buttoncallback) | [**on\_long\_press**](#variable-on_long_press)  <br> |
|  void \* | [**on\_long\_press\_ctx**](#variable-on_long_press_ctx)  <br> |
|  [**SYN\_ButtonCallback**](syn__button_8h.md#typedef-syn_buttoncallback) | [**on\_multi\_click**](#variable-on_multi_click)  <br> |
|  void \* | [**on\_multi\_click\_ctx**](#variable-on_multi_click_ctx)  <br> |
|  [**SYN\_ButtonCallback**](syn__button_8h.md#typedef-syn_buttoncallback) | [**on\_press**](#variable-on_press)  <br> |
|  void \* | [**on\_press\_ctx**](#variable-on_press_ctx)  <br> |
|  [**SYN\_ButtonCallback**](syn__button_8h.md#typedef-syn_buttoncallback) | [**on\_release**](#variable-on_release)  <br> |
|  void \* | [**on\_release\_ctx**](#variable-on_release_ctx)  <br> |
|  [**SYN\_ButtonCallback**](syn__button_8h.md#typedef-syn_buttoncallback) | [**on\_repeat**](#variable-on_repeat)  <br> |
|  void \* | [**on\_repeat\_ctx**](#variable-on_repeat_ctx)  <br> |
|  [**SYN\_ButtonCallback**](syn__button_8h.md#typedef-syn_buttoncallback) | [**on\_single\_click**](#variable-on_single_click)  <br> |
|  void \* | [**on\_single\_click\_ctx**](#variable-on_single_click_ctx)  <br> |
|  [**SYN\_ButtonCallback**](syn__button_8h.md#typedef-syn_buttoncallback) | [**on\_triple\_click**](#variable-on_triple_click)  <br> |
|  void \* | [**on\_triple\_click\_ctx**](#variable-on_triple_click_ctx)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**pin**](#variable-pin)  <br> |
|  uint8\_t | [**polarity**](#variable-polarity)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**pressed**](#variable-pressed)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**raw\_pressed**](#variable-raw_pressed)  <br> |
|  uint16\_t | [**repeat\_ms**](#variable-repeat_ms)  <br> |
|  uint32\_t | [**repeat\_tick**](#variable-repeat_tick)  <br> |
|  uint32\_t | [**state\_tick**](#variable-state_tick)  <br> |












































## Public Attributes Documentation




### variable click\_count 

```C++
uint8_t SYN_Button::click_count;
```



Consecutive click counter 
 


        

<hr>



### variable click\_tick 

```C++
uint32_t SYN_Button::click_tick;
```



Tick of last release event 
 


        

<hr>



### variable debounce\_ms 

```C++
uint16_t SYN_Button::debounce_ms;
```



Debounce window (ms) 
 


        

<hr>



### variable double\_click\_ms 

```C++
uint16_t SYN_Button::double_click_ms;
```



Max gap between clicks (ms, default 250) 


        

<hr>



### variable events 

```C++
uint8_t SYN_Button::events;
```



Pending events bitmask 
 


        

<hr>



### variable fsm 

```C++
SYN_FSM SYN_Button::fsm;
```



Button FSM (uses syn\_fsm) 
 


        

<hr>



### variable in\_combo 

```C++
bool SYN_Button::in_combo;
```



True while button is part of active combo 


        

<hr>



### variable long\_press\_ms 

```C++
uint16_t SYN_Button::long_press_ms;
```



Long-press threshold (ms) 
 


        

<hr>



### variable on\_double\_click 

```C++
SYN_ButtonCallback SYN_Button::on_double_click;
```



Double-click callback 
 


        

<hr>



### variable on\_double\_click\_ctx 

```C++
void* SYN_Button::on_double_click_ctx;
```



Double-click context 
 


        

<hr>



### variable on\_long\_press 

```C++
SYN_ButtonCallback SYN_Button::on_long_press;
```



Long-press callback 
 


        

<hr>



### variable on\_long\_press\_ctx 

```C++
void* SYN_Button::on_long_press_ctx;
```



Long-press context 
 


        

<hr>



### variable on\_multi\_click 

```C++
SYN_ButtonCallback SYN_Button::on_multi_click;
```



Multi-click callback 
 


        

<hr>



### variable on\_multi\_click\_ctx 

```C++
void* SYN_Button::on_multi_click_ctx;
```



Multi-click context 
 


        

<hr>



### variable on\_press 

```C++
SYN_ButtonCallback SYN_Button::on_press;
```



Press callback 
 


        

<hr>



### variable on\_press\_ctx 

```C++
void* SYN_Button::on_press_ctx;
```



Press callback context 
 


        

<hr>



### variable on\_release 

```C++
SYN_ButtonCallback SYN_Button::on_release;
```



Release callback 
 


        

<hr>



### variable on\_release\_ctx 

```C++
void* SYN_Button::on_release_ctx;
```



Release callback context 
 


        

<hr>



### variable on\_repeat 

```C++
SYN_ButtonCallback SYN_Button::on_repeat;
```



Repeat callback 
 


        

<hr>



### variable on\_repeat\_ctx 

```C++
void* SYN_Button::on_repeat_ctx;
```



Repeat callback context 
 


        

<hr>



### variable on\_single\_click 

```C++
SYN_ButtonCallback SYN_Button::on_single_click;
```



Single-click callback 
 


        

<hr>



### variable on\_single\_click\_ctx 

```C++
void* SYN_Button::on_single_click_ctx;
```



Single-click context 
 


        

<hr>



### variable on\_triple\_click 

```C++
SYN_ButtonCallback SYN_Button::on_triple_click;
```



Triple-click callback 
 


        

<hr>



### variable on\_triple\_click\_ctx 

```C++
void* SYN_Button::on_triple_click_ctx;
```



Triple-click context 
 


        

<hr>



### variable pin 

```C++
SYN_GPIO_Pin SYN_Button::pin;
```



GPIO pin number 
 


        

<hr>



### variable polarity 

```C++
uint8_t SYN_Button::polarity;
```



SYN\_ButtonPolarity 
 


        

<hr>



### variable pressed 

```C++
bool SYN_Button::pressed;
```



Debounced pressed state 
 


        

<hr>



### variable raw\_pressed 

```C++
bool SYN_Button::raw_pressed;
```



Last raw GPIO reading 
 


        

<hr>



### variable repeat\_ms 

```C++
uint16_t SYN_Button::repeat_ms;
```



Auto-repeat interval (ms) 
 


        

<hr>



### variable repeat\_tick 

```C++
uint32_t SYN_Button::repeat_tick;
```



Tick of last repeat event 
 


        

<hr>



### variable state\_tick 

```C++
uint32_t SYN_Button::state_tick;
```



Tick when state was entered 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/input/syn_button.h`

