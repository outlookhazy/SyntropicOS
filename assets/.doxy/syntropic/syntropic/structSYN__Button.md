

# Struct SYN\_Button



[**ClassList**](annotated.md) **>** [**SYN\_Button**](structSYN__Button.md)



_Button descriptor — owns the FSM, debounce, and callback state._ 

* `#include <syn_button.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**debounce\_ms**](#variable-debounce_ms)  <br> |
|  uint8\_t | [**events**](#variable-events)  <br> |
|  [**SYN\_FSM**](structSYN__FSM.md) | [**fsm**](#variable-fsm)  <br> |
|  uint16\_t | [**long\_press\_ms**](#variable-long_press_ms)  <br> |
|  [**SYN\_ButtonCallback**](syn__button_8h.md#typedef-syn_buttoncallback) | [**on\_long\_press**](#variable-on_long_press)  <br> |
|  void \* | [**on\_long\_press\_ctx**](#variable-on_long_press_ctx)  <br> |
|  [**SYN\_ButtonCallback**](syn__button_8h.md#typedef-syn_buttoncallback) | [**on\_press**](#variable-on_press)  <br> |
|  void \* | [**on\_press\_ctx**](#variable-on_press_ctx)  <br> |
|  [**SYN\_ButtonCallback**](syn__button_8h.md#typedef-syn_buttoncallback) | [**on\_release**](#variable-on_release)  <br> |
|  void \* | [**on\_release\_ctx**](#variable-on_release_ctx)  <br> |
|  [**SYN\_ButtonCallback**](syn__button_8h.md#typedef-syn_buttoncallback) | [**on\_repeat**](#variable-on_repeat)  <br> |
|  void \* | [**on\_repeat\_ctx**](#variable-on_repeat_ctx)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**pin**](#variable-pin)  <br> |
|  uint8\_t | [**polarity**](#variable-polarity)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**pressed**](#variable-pressed)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**raw\_pressed**](#variable-raw_pressed)  <br> |
|  uint16\_t | [**repeat\_ms**](#variable-repeat_ms)  <br> |
|  uint32\_t | [**repeat\_tick**](#variable-repeat_tick)  <br> |
|  uint32\_t | [**state\_tick**](#variable-state_tick)  <br> |












































## Public Attributes Documentation




### variable debounce\_ms 

```C++
uint16_t SYN_Button::debounce_ms;
```



Debounce window (ms) 
 


        

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



### variable long\_press\_ms 

```C++
uint16_t SYN_Button::long_press_ms;
```



Long-press threshold (ms) 
 


        

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

