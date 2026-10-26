

# Struct SYN\_SoftPWM



[**ClassList**](annotated.md) **>** [**SYN\_SoftPWM**](structSYN__SoftPWM.md)



_Software PWM channel descriptor._ 

* `#include <syn_soft_pwm.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active\_high**](#variable-active_high)  <br> |
|  uint16\_t | [**counter**](#variable-counter)  <br> |
|  uint16\_t | [**duty**](#variable-duty)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**pin**](#variable-pin)  <br> |
|  uint16\_t | [**resolution**](#variable-resolution)  <br> |












































## Public Attributes Documentation




### variable active\_high 

```C++
bool SYN_SoftPWM::active_high;
```



true = GPIO high during on-phase 
 


        

<hr>



### variable counter 

```C++
uint16_t SYN_SoftPWM::counter;
```



Current phase counter 
 


        

<hr>



### variable duty 

```C++
uint16_t SYN_SoftPWM::duty;
```



Duty cycle (0 to resolution) 
 


        

<hr>



### variable pin 

```C++
SYN_GPIO_Pin SYN_SoftPWM::pin;
```



Target GPIO pin identifier 


        

<hr>



### variable resolution 

```C++
uint16_t SYN_SoftPWM::resolution;
```



Total steps per period (e.g., 100) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/output/syn_soft_pwm.h`

