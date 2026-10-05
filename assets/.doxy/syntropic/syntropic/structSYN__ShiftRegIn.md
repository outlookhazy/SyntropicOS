

# Struct SYN\_ShiftRegIn



[**ClassList**](annotated.md) **>** [**SYN\_ShiftRegIn**](structSYN__ShiftRegIn.md)



_Input Shift Register Context._ 

* `#include <syn_shiftreg.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**buffer**](#variable-buffer)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**clock\_pin**](#variable-clock_pin)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**data\_pin**](#variable-data_pin)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**load\_pin**](#variable-load_pin)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**msb\_first**](#variable-msb_first)  <br> |
|  uint8\_t | [**num\_chips**](#variable-num_chips)  <br> |












































## Public Attributes Documentation




### variable buffer 

```C++
uint8_t SYN_ShiftRegIn::buffer[SYN_SHIFTREG_MAX_CHIPS];
```



Sampled input state buffer bytes 


        

<hr>



### variable clock\_pin 

```C++
SYN_GPIO_Pin SYN_ShiftRegIn::clock_pin;
```



Shift Clock pin (CLK) 


        

<hr>



### variable data\_pin 

```C++
SYN_GPIO_Pin SYN_ShiftRegIn::data_pin;
```



Serial Data In pin (Q7 / SO) 


        

<hr>



### variable load\_pin 

```C++
SYN_GPIO_Pin SYN_ShiftRegIn::load_pin;
```



Parallel Load pin (PL / LD) 


        

<hr>



### variable msb\_first 

```C++
bool SYN_ShiftRegIn::msb_first;
```



True if shifting MSB first 


        

<hr>



### variable num\_chips 

```C++
uint8_t SYN_ShiftRegIn::num_chips;
```



Number of cascaded 8-bit chips (1 to 8) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_shiftreg.h`

