

# File syn\_port\_exti.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_exti.h**](syn__port__exti_8h.md)

[Go to the source code of this file](syn__port__exti_8h_source.md)

_GPIO interrupt (EXTI) port interface — implement for your platform._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_gpio.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`

















## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_EXTI\_Edge**](#enum-syn_exti_edge)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_port\_exti\_clear\_pending**](#function-syn_port_exti_clear_pending) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin) <br>_Clear pending interrupt flag for a pin._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_exti\_configure**](#function-syn_port_exti_configure) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin, [**SYN\_EXTI\_Edge**](syn__port__exti_8h.md#enum-syn_exti_edge) edge) <br>_Configure a pin for interrupt generation._  |
|  void | [**syn\_port\_exti\_disable**](#function-syn_port_exti_disable) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin) <br>_Disable interrupt on a pin (without deconfiguring)._  |
|  void | [**syn\_port\_exti\_enable**](#function-syn_port_exti_enable) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin) <br>_Enable interrupt on a configured pin._  |




























## Detailed Description


Provides pin-change interrupt registration. The platform layer manages the hardware (NVIC, EXTI lines, etc.) and calls back into the syn\_exti dispatcher. 


    
## Public Types Documentation




### enum SYN\_EXTI\_Edge 

```C++
enum SYN_EXTI_Edge {
    SYN_EXTI_RISING = 0,
    SYN_EXTI_FALLING = 1,
    SYN_EXTI_BOTH = 2
};
```



Interrupt trigger edge. 


        

<hr>
## Public Functions Documentation




### function syn\_port\_exti\_clear\_pending 

_Clear pending interrupt flag for a pin._ 
```C++
void syn_port_exti_clear_pending (
    SYN_GPIO_Pin pin
) 
```





**Parameters:**


* `pin` GPIO pin. 




        

<hr>



### function syn\_port\_exti\_configure 

_Configure a pin for interrupt generation._ 
```C++
SYN_Status syn_port_exti_configure (
    SYN_GPIO_Pin pin,
    SYN_EXTI_Edge edge
) 
```





**Parameters:**


* `pin` GPIO pin number. 
* `edge` Trigger edge. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_exti\_disable 

_Disable interrupt on a pin (without deconfiguring)._ 
```C++
void syn_port_exti_disable (
    SYN_GPIO_Pin pin
) 
```





**Parameters:**


* `pin` GPIO pin. 




        

<hr>



### function syn\_port\_exti\_enable 

_Enable interrupt on a configured pin._ 
```C++
void syn_port_exti_enable (
    SYN_GPIO_Pin pin
) 
```





**Parameters:**


* `pin` GPIO pin. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_exti.h`

