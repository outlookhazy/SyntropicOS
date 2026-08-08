

# Struct SYN\_Seg7



[**ClassList**](annotated.md) **>** [**SYN\_Seg7**](structSYN__Seg7.md)



_7-Segment Display Instance Context._ 

* `#include <syn_seg7.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**active\_digit**](#variable-active_digit)  <br> |
|  uint8\_t | [**digit\_buffers**](#variable-digit_buffers)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**digit\_pins**](#variable-digit_pins)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**leading\_zeros**](#variable-leading_zeros)  <br> |
|  uint8\_t | [**num\_digits**](#variable-num_digits)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**segment\_pins**](#variable-segment_pins)  <br> |
|  [**SYN\_Seg7Type**](syn__seg7_8h.md#enum-syn_seg7type) | [**type**](#variable-type)  <br> |












































## Public Attributes Documentation




### variable active\_digit 

```C++
uint8_t SYN_Seg7::active_digit;
```



Active scanning digit index 


        

<hr>



### variable digit\_buffers 

```C++
uint8_t SYN_Seg7::digit_buffers[SYN_SEG7_MAX_DIGITS];
```



Raw segment bitmask per digit 


        

<hr>



### variable digit\_pins 

```C++
SYN_GPIO_Pin SYN_Seg7::digit_pins[SYN_SEG7_MAX_DIGITS];
```



Digit selection pins 


        

<hr>



### variable leading\_zeros 

```C++
bool SYN_Seg7::leading_zeros;
```



True to display leading zeros 


        

<hr>



### variable num\_digits 

```C++
uint8_t SYN_Seg7::num_digits;
```



Total number of digits (1 to 8) 


        

<hr>



### variable segment\_pins 

```C++
SYN_GPIO_Pin SYN_Seg7::segment_pins[8];
```



Segment pins: A, B, C, D, E, F, G, DP 


        

<hr>



### variable type 

```C++
SYN_Seg7Type SYN_Seg7::type;
```



Common Cathode or Common Anode 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/display/syn_seg7.h`

