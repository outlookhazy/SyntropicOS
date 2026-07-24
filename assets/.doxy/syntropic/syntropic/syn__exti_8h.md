

# File syn\_exti.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_exti.h**](syn__exti_8h.md)

[Go to the source code of this file](syn__exti_8h_source.md)

_GPIO interrupt dispatcher — register callbacks per pin._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_exti.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`

















## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_EXTI\_Callback**](#typedef-syn_exti_callback)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_exti\_count**](#function-syn_exti_count) (void) <br>_Get the number of registered callbacks._  |
|  void | [**syn\_exti\_disable**](#function-syn_exti_disable) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin) <br>_Disable interrupt for a pin._  |
|  void | [**syn\_exti\_enable**](#function-syn_exti_enable) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin) <br>_Enable interrupt for a registered pin._  |
|  void | [**syn\_exti\_init**](#function-syn_exti_init) (void) <br> |
|  void | [**syn\_exti\_irq\_handler**](#function-syn_exti_irq_handler) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin) <br>_IRQ handler — call from platform ISR._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_exti\_register**](#function-syn_exti_register) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin, [**SYN\_EXTI\_Edge**](syn__port__exti_8h.md#enum-syn_exti_edge) edge, [**SYN\_EXTI\_Callback**](syn__exti_8h.md#typedef-syn_exti_callback) cb, void \* ctx) <br>_Register a callback for a pin interrupt._  |
|  void | [**syn\_exti\_unregister**](#function-syn_exti_unregister) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin) <br>_Unregister a pin callback._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_EXTI\_MAX\_PINS**](syn__exti_8h.md#define-syn_exti_max_pins)  `16`<br> |

## Detailed Description


Thin layer over the EXTI port. Registers a callback for each pin; when the platform ISR fires, call syn\_exti\_irq\_handler(pin) and the dispatcher invokes the registered callback.


Optionally integrates with the work queue for deferred processing (recommended for non-trivial handlers).




**
**


```C++
void on_button_press(SYN_GPIO_Pin pin, void *ctx) {
    // runs in ISR context — keep it fast!
    *(bool *)ctx = true;
}

syn_exti_init();
syn_exti_register(PIN_BUTTON, SYN_EXTI_FALLING,
                   on_button_press, &button_flag);
syn_exti_enable(PIN_BUTTON);
```





**
**


```C++
// STM32 example:
void EXTI0_IRQHandler(void) {
    if (EXTI->PR & (1 << 0)) {
        EXTI->PR = (1 << 0);  // clear
        syn_exti_irq_handler(0);
    }
}
```
 





    
## Public Types Documentation




### typedef SYN\_EXTI\_Callback 

```C++
typedef void(* SYN_EXTI_Callback) (SYN_GPIO_Pin pin, void *ctx);
```



EXTI callback — called from ISR context unless deferred. 


        

<hr>
## Public Functions Documentation




### function syn\_exti\_count 

_Get the number of registered callbacks._ 
```C++
size_t syn_exti_count (
    void
) 
```





**Returns:**

Registration count. 





        

<hr>



### function syn\_exti\_disable 

_Disable interrupt for a pin._ 
```C++
void syn_exti_disable (
    SYN_GPIO_Pin pin
) 
```





**Parameters:**


* `pin` GPIO pin. 




        

<hr>



### function syn\_exti\_enable 

_Enable interrupt for a registered pin._ 
```C++
void syn_exti_enable (
    SYN_GPIO_Pin pin
) 
```





**Parameters:**


* `pin` GPIO pin. 




        

<hr>



### function syn\_exti\_init 

```C++
void syn_exti_init (
    void
) 
```



Initialize the EXTI dispatcher. 


        

<hr>



### function syn\_exti\_irq\_handler 

_IRQ handler — call from platform ISR._ 
```C++
void syn_exti_irq_handler (
    SYN_GPIO_Pin pin
) 
```



Looks up the registered callback for `pin` and invokes it. This is the glue between hardware ISR and the dispatcher.




**Parameters:**


* `pin` GPIO pin that triggered the interrupt. 




        

<hr>



### function syn\_exti\_register 

_Register a callback for a pin interrupt._ 
```C++
SYN_Status syn_exti_register (
    SYN_GPIO_Pin pin,
    SYN_EXTI_Edge edge,
    SYN_EXTI_Callback cb,
    void * ctx
) 
```





**Parameters:**


* `pin` GPIO pin. 
* `edge` Trigger edge. 
* `cb` Callback (runs in ISR context). 
* `ctx` User context passed to callback. 



**Returns:**

SYN\_OK, or SYN\_ERROR if table full. 





        

<hr>



### function syn\_exti\_unregister 

_Unregister a pin callback._ 
```C++
void syn_exti_unregister (
    SYN_GPIO_Pin pin
) 
```





**Parameters:**


* `pin` GPIO pin. 




        

<hr>
## Macro Definition Documentation





### define SYN\_EXTI\_MAX\_PINS 

```C++
#define SYN_EXTI_MAX_PINS `16`
```



Max number of EXTI-enabled pins 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_exti.h`

