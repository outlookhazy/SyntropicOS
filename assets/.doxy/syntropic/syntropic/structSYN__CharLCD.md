

# Struct SYN\_CharLCD



[**ClassList**](annotated.md) **>** [**SYN\_CharLCD**](structSYN__CharLCD.md)



_Generic Character LCD Context._ 

* `#include <syn_charlcd.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**backlight**](#variable-backlight)  <br> |
|  uint8\_t | [**backlight\_mask**](#variable-backlight_mask)  <br> |
|  uint8\_t | [**cols**](#variable-cols)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**d\_pins**](#variable-d_pins)  <br> |
|  uint8\_t | [**display\_control**](#variable-display_control)  <br> |
|  uint8\_t | [**display\_mode**](#variable-display_mode)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**en\_pin**](#variable-en_pin)  <br> |
|  [**SYN\_SoftI2C**](structSYN__SoftI2C.md) | [**i2c**](#variable-i2c)  <br> |
|  uint8\_t | [**i2c\_addr**](#variable-i2c_addr)  <br> |
|  [**SYN\_CharLCDMode**](syn__charlcd_8h.md#enum-syn_charlcdmode) | [**mode**](#variable-mode)  <br> |
|  uint8\_t | [**rows**](#variable-rows)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**rs\_pin**](#variable-rs_pin)  <br> |












































## Public Attributes Documentation




### variable backlight 

```C++
bool SYN_CharLCD::backlight;
```



True if backlight enabled 


        

<hr>



### variable backlight\_mask 

```C++
uint8_t SYN_CharLCD::backlight_mask;
```



Backlight bit mask 


        

<hr>



### variable cols 

```C++
uint8_t SYN_CharLCD::cols;
```



Display columns (e.g. 16, 20) 


        

<hr>



### variable d\_pins 

```C++
SYN_GPIO_Pin SYN_CharLCD::d_pins[4];
```



D4, D5, D6, D7 pins 


        

<hr>



### variable display\_control 

```C++
uint8_t SYN_CharLCD::display_control;
```



Display ON/OFF control state 


        

<hr>



### variable display\_mode 

```C++
uint8_t SYN_CharLCD::display_mode;
```



Entry mode state 


        

<hr>



### variable en\_pin 

```C++
SYN_GPIO_Pin SYN_CharLCD::en_pin;
```



Enable pulse GPIO pin 


        

<hr>



### variable i2c 

```C++
SYN_SoftI2C SYN_CharLCD::i2c;
```



Software I2C bus context 


        

<hr>



### variable i2c\_addr 

```C++
uint8_t SYN_CharLCD::i2c_addr;
```



7-bit I2C device address 


        

<hr>



### variable mode 

```C++
SYN_CharLCDMode SYN_CharLCD::mode;
```



Interface mode (I2C or 4-bit GPIO) 


        

<hr>



### variable rows 

```C++
uint8_t SYN_CharLCD::rows;
```



Display rows (e.g. 2, 4) 


        

<hr>



### variable rs\_pin 

```C++
SYN_GPIO_Pin SYN_CharLCD::rs_pin;
```



Register select GPIO pin 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/display/syn_charlcd.h`

