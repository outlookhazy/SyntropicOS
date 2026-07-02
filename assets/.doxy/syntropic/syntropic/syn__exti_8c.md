

# File syn\_exti.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_exti.c**](syn__exti_8c.md)

[Go to the source code of this file](syn__exti_8c_source.md)

_GPIO interrupt dispatcher implementation._ 

* `#include "syn_exti.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**EXTI\_Entry**](structEXTI__Entry.md) <br>_EXTI callback table entry._  |








## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  size\_t | [**exti\_count**](#variable-exti_count)  <br> |
|  [**EXTI\_Entry**](structEXTI__Entry.md) | [**exti\_table**](#variable-exti_table)  <br> |














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


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**EXTI\_Entry**](structEXTI__Entry.md) \* | [**find\_entry**](#function-find_entry) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin) <br>_Find the table entry for a given pin._  |


























## Public Static Attributes Documentation




### variable exti\_count 

```C++
size_t exti_count;
```



Number of registered. 
 


        

<hr>



### variable exti\_table 

```C++
EXTI_Entry exti_table[SYN_EXTI_MAX_PINS];
```



Registered callbacks. 
 


        

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
## Public Static Functions Documentation




### function find\_entry 

_Find the table entry for a given pin._ 
```C++
static EXTI_Entry * find_entry (
    SYN_GPIO_Pin pin
) 
```





**Parameters:**


* `pin` GPIO pin. 



**Returns:**

Pointer to entry, or NULL if not found. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_exti.c`

