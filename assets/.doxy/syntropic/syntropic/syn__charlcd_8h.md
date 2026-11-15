

# File syn\_charlcd.h



[**FileList**](files.md) **>** [**display**](dir_572b98f20fe1ab62e971e24ddc7f0bdb.md) **>** [**syn\_charlcd.h**](syn__charlcd_8h.md)

[Go to the source code of this file](syn__charlcd_8h_source.md)

_Generic Character LCD Driver (HD44780, ST7066, KS0066 over I2C PCF8574 or 4-Bit Parallel GPIO)._ 

* `#include "../common/syn_defs.h"`
* `#include "../drivers/syn_soft_i2c.h"`
* `#include "../port/syn_port_gpio.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_CharLCD**](structSYN__CharLCD.md) <br>_Generic Character LCD Context._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_CharLCDMode**](#enum-syn_charlcdmode)  <br>_Character LCD Interface Mode._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_charlcd\_clear**](#function-syn_charlcd_clear) ([**SYN\_CharLCD**](structSYN__CharLCD.md) \* lcd) <br>_Clear LCD screen and reset cursor to home (0,0)._  |
|  void | [**syn\_charlcd\_create\_char**](#function-syn_charlcd_create_char) ([**SYN\_CharLCD**](structSYN__CharLCD.md) \* lcd, uint8\_t slot, const uint8\_t charmap) <br>_Load a custom 5x8 pixel character into CGRAM._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_charlcd\_init\_gpio**](#function-syn_charlcd_init_gpio) ([**SYN\_CharLCD**](structSYN__CharLCD.md) \* lcd, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) rs, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) en, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) d4, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) d5, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) d6, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) d7, uint8\_t cols, uint8\_t rows) <br>_Initialize Character LCD in Direct 4-Bit Parallel GPIO mode._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_charlcd\_init\_i2c**](#function-syn_charlcd_init_i2c) ([**SYN\_CharLCD**](structSYN__CharLCD.md) \* lcd, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) scl, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) sda, uint8\_t i2c\_addr, uint8\_t cols, uint8\_t rows) <br>_Initialize Character LCD in I2C PCF8574 Backpack mode._  |
|  void | [**syn\_charlcd\_print**](#function-syn_charlcd_print) ([**SYN\_CharLCD**](structSYN__CharLCD.md) \* lcd, const char \* str) <br>_Print an ASCII string at current cursor position._  |
|  void | [**syn\_charlcd\_set\_backlight**](#function-syn_charlcd_set_backlight) ([**SYN\_CharLCD**](structSYN__CharLCD.md) \* lcd, [**bool**](syn__defs_8h.md#enum-bool) enable) <br>_Turn display backlight ON or OFF (I2C backpack mode)._  |
|  void | [**syn\_charlcd\_set\_cursor**](#function-syn_charlcd_set_cursor) ([**SYN\_CharLCD**](structSYN__CharLCD.md) \* lcd, uint8\_t col, uint8\_t row) <br>_Set cursor position._  |




























## Public Types Documentation




### enum SYN\_CharLCDMode 

_Character LCD Interface Mode._ 
```C++
enum SYN_CharLCDMode {
    SYN_CHARLCD_MODE_I2C = 0,
    SYN_CHARLCD_MODE_GPIO_4BIT
};
```




<hr>
## Public Functions Documentation




### function syn\_charlcd\_clear 

_Clear LCD screen and reset cursor to home (0,0)._ 
```C++
void syn_charlcd_clear (
    SYN_CharLCD * lcd
) 
```





**Parameters:**


* `lcd` LCD context. 




        

<hr>



### function syn\_charlcd\_create\_char 

_Load a custom 5x8 pixel character into CGRAM._ 
```C++
void syn_charlcd_create_char (
    SYN_CharLCD * lcd,
    uint8_t slot,
    const uint8_t charmap
) 
```





**Parameters:**


* `lcd` LCD context. 
* `slot` CGRAM slot (0 to 7). 
* `charmap` Array of 8 bytes (each byte represents a 5-bit pixel row). 




        

<hr>



### function syn\_charlcd\_init\_gpio 

_Initialize Character LCD in Direct 4-Bit Parallel GPIO mode._ 
```C++
SYN_Status syn_charlcd_init_gpio (
    SYN_CharLCD * lcd,
    SYN_GPIO_Pin rs,
    SYN_GPIO_Pin en,
    SYN_GPIO_Pin d4,
    SYN_GPIO_Pin d5,
    SYN_GPIO_Pin d6,
    SYN_GPIO_Pin d7,
    uint8_t cols,
    uint8_t rows
) 
```





**Parameters:**


* `lcd` LCD context. 
* `rs` Register Select GPIO pin. 
* `en` Enable GPIO pin. 
* `d4` Data Bit 4 GPIO pin. 
* `d5` Data Bit 5 GPIO pin. 
* `d6` Data Bit 6 GPIO pin. 
* `d7` Data Bit 7 GPIO pin. 
* `cols` Columns (e.g. 16 or 20). 
* `rows` Rows (e.g. 2 or 4). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_charlcd\_init\_i2c 

_Initialize Character LCD in I2C PCF8574 Backpack mode._ 
```C++
SYN_Status syn_charlcd_init_i2c (
    SYN_CharLCD * lcd,
    SYN_GPIO_Pin scl,
    SYN_GPIO_Pin sda,
    uint8_t i2c_addr,
    uint8_t cols,
    uint8_t rows
) 
```





**Parameters:**


* `lcd` LCD context. 
* `scl` I2C SCL GPIO pin. 
* `sda` I2C SDA GPIO pin. 
* `i2c_addr` PCF8574 I2C address (e.g. 0x27 or 0x3F). 
* `cols` Columns (e.g. 16 or 20). 
* `rows` Rows (e.g. 2 or 4). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_charlcd\_print 

_Print an ASCII string at current cursor position._ 
```C++
void syn_charlcd_print (
    SYN_CharLCD * lcd,
    const char * str
) 
```





**Parameters:**


* `lcd` LCD context. 
* `str` Null-terminated string. 




        

<hr>



### function syn\_charlcd\_set\_backlight 

_Turn display backlight ON or OFF (I2C backpack mode)._ 
```C++
void syn_charlcd_set_backlight (
    SYN_CharLCD * lcd,
    bool enable
) 
```





**Parameters:**


* `lcd` LCD context. 
* `enable` True for backlight ON. 




        

<hr>



### function syn\_charlcd\_set\_cursor 

_Set cursor position._ 
```C++
void syn_charlcd_set_cursor (
    SYN_CharLCD * lcd,
    uint8_t col,
    uint8_t row
) 
```





**Parameters:**


* `lcd` LCD context. 
* `col` Column (0 to cols-1). 
* `row` Row (0 to rows-1). 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/display/syn_charlcd.h`

