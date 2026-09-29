

# Struct SYN\_Touch



[**ClassList**](annotated.md) **>** [**SYN\_Touch**](structSYN__Touch.md)



_Capacitive Touch Button Instance Context._ 

* `#include <syn_touch.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**baseline**](#variable-baseline)  <br> |
|  uint16\_t | [**current\_val**](#variable-current_val)  <br> |
|  uint16\_t | [**hysteresis**](#variable-hysteresis)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_pressed**](#variable-is_pressed)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**pin**](#variable-pin)  <br> |
|  uint32\_t | [**press\_count**](#variable-press_count)  <br> |
|  uint16\_t | [**threshold**](#variable-threshold)  <br> |












































## Public Attributes Documentation




### variable baseline 

```C++
uint16_t SYN_Touch::baseline;
```



Dynamic baseline capacitance value 


        

<hr>



### variable current\_val 

```C++
uint16_t SYN_Touch::current_val;
```



Current raw sample value 


        

<hr>



### variable hysteresis 

```C++
uint16_t SYN_Touch::hysteresis;
```



Hysteresis for press/release stability 


        

<hr>



### variable is\_pressed 

```C++
bool SYN_Touch::is_pressed;
```



True if touch button is pressed 


        

<hr>



### variable pin 

```C++
SYN_GPIO_Pin SYN_Touch::pin;
```



GPIO pin identifier 


        

<hr>



### variable press\_count 

```C++
uint32_t SYN_Touch::press_count;
```



Total press count 


        

<hr>



### variable threshold 

```C++
uint16_t SYN_Touch::threshold;
```



Touch detection threshold Delta 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/input/syn_touch.h`

