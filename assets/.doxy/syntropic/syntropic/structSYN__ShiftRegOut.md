

# Struct SYN\_ShiftRegOut



[**ClassList**](annotated.md) **>** [**SYN\_ShiftRegOut**](structSYN__ShiftRegOut.md)



_Output Shift Register Context._ 

* `#include <syn_shiftreg.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**buffer**](#variable-buffer)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**clock\_pin**](#variable-clock_pin)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**data\_pin**](#variable-data_pin)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**latch\_pin**](#variable-latch_pin)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**msb\_first**](#variable-msb_first)  <br> |
|  uint8\_t | [**num\_chips**](#variable-num_chips)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**oe\_pin**](#variable-oe_pin)  <br> |












































## Public Attributes Documentation




### variable buffer 

```C++
uint8_t SYN_ShiftRegOut::buffer[SYN_SHIFTREG_MAX_CHIPS];
```



Output state buffer bytes 


        

<hr>



### variable clock\_pin 

```C++
SYN_GPIO_Pin SYN_ShiftRegOut::clock_pin;
```



Shift Clock pin (SH\_CP / SRCLK) 


        

<hr>



### variable data\_pin 

```C++
SYN_GPIO_Pin SYN_ShiftRegOut::data_pin;
```



Serial Data Out pin (DS / SER) 


        

<hr>



### variable latch\_pin 

```C++
SYN_GPIO_Pin SYN_ShiftRegOut::latch_pin;
```



Storage Latch Clock pin (ST\_CP / RCLK) 


        

<hr>



### variable msb\_first 

```C++
bool SYN_ShiftRegOut::msb_first;
```



True if shifting MSB first 


        

<hr>



### variable num\_chips 

```C++
uint8_t SYN_ShiftRegOut::num_chips;
```



Number of cascaded 8-bit chips (1 to 8) 


        

<hr>



### variable oe\_pin 

```C++
SYN_GPIO_Pin SYN_ShiftRegOut::oe_pin;
```



Optional Output Enable pin (0 = unused) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_shiftreg.h`

