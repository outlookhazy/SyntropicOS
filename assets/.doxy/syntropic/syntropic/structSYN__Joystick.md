

# Struct SYN\_Joystick



[**ClassList**](annotated.md) **>** [**SYN\_Joystick**](structSYN__Joystick.md)



_Generic Analog Joystick Context._ 

* `#include <syn_joystick.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**adc\_max**](#variable-adc_max)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**button\_pressed**](#variable-button_pressed)  <br> |
|  uint16\_t | [**center\_x**](#variable-center_x)  <br> |
|  uint16\_t | [**center\_y**](#variable-center_y)  <br> |
|  int16\_t | [**current\_x\_pct**](#variable-current_x_pct)  <br> |
|  int16\_t | [**current\_y\_pct**](#variable-current_y_pct)  <br> |
|  uint16\_t | [**deadband**](#variable-deadband)  <br> |
|  [**SYN\_JoystickDir**](syn__joystick_8h.md#enum-syn_joystickdir) | [**dir**](#variable-dir)  <br> |












































## Public Attributes Documentation




### variable adc\_max 

```C++
uint16_t SYN_Joystick::adc_max;
```



ADC resolution max (e.g. 4095) 


        

<hr>



### variable button\_pressed 

```C++
bool SYN_Joystick::button_pressed;
```



Digital push button state 


        

<hr>



### variable center\_x 

```C++
uint16_t SYN_Joystick::center_x;
```



ADC zero-point center X 


        

<hr>



### variable center\_y 

```C++
uint16_t SYN_Joystick::center_y;
```



ADC zero-point center Y 


        

<hr>



### variable current\_x\_pct 

```C++
int16_t SYN_Joystick::current_x_pct;
```



Calculated X percentage (-100 to +100%) 


        

<hr>



### variable current\_y\_pct 

```C++
int16_t SYN_Joystick::current_y_pct;
```



Calculated Y percentage (-100 to +100%) 


        

<hr>



### variable deadband 

```C++
uint16_t SYN_Joystick::deadband;
```



Noise deadband radius 


        

<hr>



### variable dir 

```C++
SYN_JoystickDir SYN_Joystick::dir;
```



8-way directional state 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/input/syn_joystick.h`

