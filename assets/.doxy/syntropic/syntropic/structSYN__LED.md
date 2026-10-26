

# Struct SYN\_LED



[**ClassList**](annotated.md) **>** [**SYN\_LED**](structSYN__LED.md)



_LED instance — pin, mode, blink/flash/pattern state._ 

* `#include <syn_led.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**flash\_remain**](#variable-flash_remain)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**lit**](#variable-lit)  <br> |
|  uint8\_t | [**mode**](#variable-mode)  <br> |
|  uint16\_t | [**off\_ms**](#variable-off_ms)  <br> |
|  uint16\_t | [**on\_ms**](#variable-on_ms)  <br> |
|  const char \* | [**pattern**](#variable-pattern)  <br> |
|  uint8\_t | [**pattern\_idx**](#variable-pattern_idx)  <br> |
|  uint16\_t | [**pattern\_unit**](#variable-pattern_unit)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**pin**](#variable-pin)  <br> |
|  uint8\_t | [**polarity**](#variable-polarity)  <br> |
|  uint32\_t | [**tick**](#variable-tick)  <br> |












































## Public Attributes Documentation




### variable flash\_remain 

```C++
uint8_t SYN_LED::flash_remain;
```



Remaining flashes 
 


        

<hr>



### variable lit 

```C++
bool SYN_LED::lit;
```



Current output state 
 


        

<hr>



### variable mode 

```C++
uint8_t SYN_LED::mode;
```



SYN\_LEDMode 
 


        

<hr>



### variable off\_ms 

```C++
uint16_t SYN_LED::off_ms;
```



Off duration (ms) 
 


        

<hr>



### variable on\_ms 

```C++
uint16_t SYN_LED::on_ms;
```



On duration (ms) 
 


        

<hr>



### variable pattern 

```C++
const char* SYN_LED::pattern;
```



Pattern string pointer 
 


        

<hr>



### variable pattern\_idx 

```C++
uint8_t SYN_LED::pattern_idx;
```



Current position in pattern 
 


        

<hr>



### variable pattern\_unit 

```C++
uint16_t SYN_LED::pattern_unit;
```



Base unit time (ms) 
 


        

<hr>



### variable pin 

```C++
SYN_GPIO_Pin SYN_LED::pin;
```



GPIO pin 
 


        

<hr>



### variable polarity 

```C++
uint8_t SYN_LED::polarity;
```



SYN\_LEDPolarity 
 


        

<hr>



### variable tick 

```C++
uint32_t SYN_LED::tick;
```



Last transition tick 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/output/syn_led.h`

