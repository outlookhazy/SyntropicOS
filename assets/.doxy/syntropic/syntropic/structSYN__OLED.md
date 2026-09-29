

# Struct SYN\_OLED



[**ClassList**](annotated.md) **>** [**SYN\_OLED**](structSYN__OLED.md)



_Generic OLED Display Instance Context._ [More...](#detailed-description)

* `#include <syn_oled.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**col\_offset**](#variable-col_offset)  <br> |
|  uint8\_t | [**contrast**](#variable-contrast)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**display\_on**](#variable-display_on)  <br> |
|  uint16\_t | [**height**](#variable-height)  <br> |
|  [**SYN\_SoftI2C**](structSYN__SoftI2C.md) | [**i2c**](#variable-i2c)  <br> |
|  uint8\_t | [**i2c\_addr**](#variable-i2c_addr)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**inverted**](#variable-inverted)  <br> |
|  [**SYN\_OLEDType**](syn__oled_8h.md#enum-syn_oledtype) | [**type**](#variable-type)  <br> |
|  uint16\_t | [**width**](#variable-width)  <br> |












































## Detailed Description


Generic OLED Display Instance Context. 


    
## Public Attributes Documentation




### variable col\_offset 

```C++
uint8_t SYN_OLED::col_offset;
```



Column start offset (0 for SSD1306, 2 for SH1106) 


        

<hr>



### variable contrast 

```C++
uint8_t SYN_OLED::contrast;
```



Contrast setting (0 to 255) 


        

<hr>



### variable display\_on 

```C++
bool SYN_OLED::display_on;
```



True if display powered on 


        

<hr>



### variable height 

```C++
uint16_t SYN_OLED::height;
```



Display height in pixels 


        

<hr>



### variable i2c 

```C++
SYN_SoftI2C SYN_OLED::i2c;
```



Software I2C bus context 


        

<hr>



### variable i2c\_addr 

```C++
uint8_t SYN_OLED::i2c_addr;
```



7-bit I2C device address 


        

<hr>



### variable inverted 

```C++
bool SYN_OLED::inverted;
```



True if display colors inverted 


        

<hr>



### variable type 

```C++
SYN_OLEDType SYN_OLED::type;
```



Controller IC type enum 


        

<hr>



### variable width 

```C++
uint16_t SYN_OLED::width;
```



Display width in pixels 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/display/syn_oled.h`

