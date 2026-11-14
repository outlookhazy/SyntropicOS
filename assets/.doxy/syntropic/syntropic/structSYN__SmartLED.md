

# Struct SYN\_SmartLED



[**ClassList**](annotated.md) **>** [**SYN\_SmartLED**](structSYN__SmartLED.md)



_Generic Smart LED Strip Instance Context._ 

* `#include <syn_smartled.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**brightness**](#variable-brightness)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**data\_pin**](#variable-data_pin)  <br> |
|  uint16\_t | [**num\_leds**](#variable-num_leds)  <br> |
|  [**SYN\_SmartLEDOrder**](syn__smartled_8h.md#enum-syn_smartledorder) | [**order**](#variable-order)  <br> |
|  [**SYN\_SmartLEDColor**](structSYN__SmartLEDColor.md) \* | [**pixel\_buf**](#variable-pixel_buf)  <br> |












































## Public Attributes Documentation




### variable brightness 

```C++
uint8_t SYN_SmartLED::brightness;
```



Global brightness scale (0 to 255) 


        

<hr>



### variable data\_pin 

```C++
SYN_GPIO_Pin SYN_SmartLED::data_pin;
```



Bit-bang or TIM/SPI GPIO data output pin 


        

<hr>



### variable num\_leds 

```C++
uint16_t SYN_SmartLED::num_leds;
```



Total number of LEDs in chain 


        

<hr>



### variable order 

```C++
SYN_SmartLEDOrder SYN_SmartLED::order;
```



Pixel color channel order enum 


        

<hr>



### variable pixel\_buf 

```C++
SYN_SmartLEDColor* SYN_SmartLED::pixel_buf;
```



Caller-allocated pixel buffer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/output/syn_smartled.h`

