

# File syn\_oled.c



[**FileList**](files.md) **>** [**display**](dir_572b98f20fe1ab62e971e24ddc7f0bdb.md) **>** [**syn\_oled.c**](syn__oled_8c.md)

[Go to the source code of this file](syn__oled_8c_source.md)

_Generic Monochrome OLED Display Driver (SSD1306, SH1106, SSD1309 over I2C)._ 

* `#include "syn_oled.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_oled\_canvas\_flush\_cb**](#function-syn_oled_canvas_flush_cb) (const uint8\_t \* buf, size\_t len, void \* ctx) <br>_Canvas flush callback matching SYN\_Canvas\_FlushFn signature._  |
|  void | [**syn\_oled\_flush\_canvas**](#function-syn_oled_flush_canvas) ([**SYN\_OLED**](structSYN__OLED.md) \* oled, const [**SYN\_Canvas**](structSYN__Canvas.md) \* c) <br>_Flush canvas pixel buffer to physical OLED display screen._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_oled\_init**](#function-syn_oled_init) ([**SYN\_OLED**](structSYN__OLED.md) \* oled, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) scl, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) sda, uint8\_t i2c\_addr, uint16\_t w, uint16\_t h, [**SYN\_OLEDType**](syn__oled_8h.md#enum-syn_oledtype) type) <br>_Initialize Monochrome OLED display context._  |
|  void | [**syn\_oled\_set\_contrast**](#function-syn_oled_set_contrast) ([**SYN\_OLED**](structSYN__OLED.md) \* oled, uint8\_t contrast) <br>_Set display contrast level._  |
|  void | [**syn\_oled\_set\_display\_on**](#function-syn_oled_set_display_on) ([**SYN\_OLED**](structSYN__OLED.md) \* oled, [**bool**](syn__defs_8h.md#enum-bool) on) <br>_Turn display ON or OFF (power save mode)._  |
|  void | [**syn\_oled\_set\_invert**](#function-syn_oled_set_invert) ([**SYN\_OLED**](structSYN__OLED.md) \* oled, [**bool**](syn__defs_8h.md#enum-bool) invert) <br>_Set display color inversion._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**get\_canvas\_pixel**](#function-get_canvas_pixel) (const [**SYN\_Canvas**](structSYN__Canvas.md) \* c, uint16\_t x, uint16\_t y) <br>_Get pixel state from monochrome canvas._  |
|  void | [**send\_command**](#function-send_command) ([**SYN\_OLED**](structSYN__OLED.md) \* oled, uint8\_t cmd) <br>_Send command byte to OLED controller._  |
|  void | [**write\_i2c\_reg**](#function-write_i2c_reg) ([**SYN\_SoftI2C**](structSYN__SoftI2C.md) \* i2c, uint8\_t dev\_addr, uint8\_t reg, uint8\_t val) <br>_Write register byte over software I2C._  |


























## Public Functions Documentation




### function syn\_oled\_canvas\_flush\_cb 

_Canvas flush callback matching SYN\_Canvas\_FlushFn signature._ 
```C++
void syn_oled_canvas_flush_cb (
    const uint8_t * buf,
    size_t len,
    void * ctx
) 
```





**Parameters:**


* `buf` Framebuffer byte array. 
* `len` Byte array length. 
* `ctx` Context pointer ([**SYN\_OLED**](structSYN__OLED.md) instance). 




        

<hr>



### function syn\_oled\_flush\_canvas 

_Flush canvas pixel buffer to physical OLED display screen._ 
```C++
void syn_oled_flush_canvas (
    SYN_OLED * oled,
    const SYN_Canvas * c
) 
```





**Parameters:**


* `oled` OLED context. 
* `c` Source Canvas context. 




        

<hr>



### function syn\_oled\_init 

_Initialize Monochrome OLED display context._ 
```C++
SYN_Status syn_oled_init (
    SYN_OLED * oled,
    SYN_GPIO_Pin scl,
    SYN_GPIO_Pin sda,
    uint8_t i2c_addr,
    uint16_t w,
    uint16_t h,
    SYN_OLEDType type
) 
```





**Parameters:**


* `oled` OLED context. 
* `scl` I2C SCL GPIO pin. 
* `sda` I2C SDA GPIO pin. 
* `i2c_addr` I2C slave address (e.g. 0x3C or 0x3D). 
* `w` Width in pixels (e.g. 128). 
* `h` Height in pixels (e.g. 64 or 32). 
* `type` OLED IC Controller type (SSD1306, SH1106, SSD1309). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_oled\_set\_contrast 

_Set display contrast level._ 
```C++
void syn_oled_set_contrast (
    SYN_OLED * oled,
    uint8_t contrast
) 
```





**Parameters:**


* `oled` OLED context. 
* `contrast` Contrast level (0 to 255). 




        

<hr>



### function syn\_oled\_set\_display\_on 

_Turn display ON or OFF (power save mode)._ 
```C++
void syn_oled_set_display_on (
    SYN_OLED * oled,
    bool on
) 
```





**Parameters:**


* `oled` OLED context. 
* `on` True for display ON. 




        

<hr>



### function syn\_oled\_set\_invert 

_Set display color inversion._ 
```C++
void syn_oled_set_invert (
    SYN_OLED * oled,
    bool invert
) 
```





**Parameters:**


* `oled` OLED context. 
* `invert` True for inverted display (black on white). 




        

<hr>
## Public Static Functions Documentation




### function get\_canvas\_pixel 

_Get pixel state from monochrome canvas._ 
```C++
static bool get_canvas_pixel (
    const SYN_Canvas * c,
    uint16_t x,
    uint16_t y
) 
```





**Parameters:**


* `c` Pointer to canvas. 
* `x` X coordinate. 
* `y` Y coordinate. 



**Returns:**

True if pixel set, false if clear. 





        

<hr>



### function send\_command 

_Send command byte to OLED controller._ 
```C++
static void send_command (
    SYN_OLED * oled,
    uint8_t cmd
) 
```





**Parameters:**


* `oled` Pointer to OLED instance. 
* `cmd` Command byte. 




        

<hr>



### function write\_i2c\_reg 

_Write register byte over software I2C._ 
```C++
static void write_i2c_reg (
    SYN_SoftI2C * i2c,
    uint8_t dev_addr,
    uint8_t reg,
    uint8_t val
) 
```





**Parameters:**


* `i2c` Pointer to I2C interface. 
* `dev_addr` 7-bit device I2C address. 
* `reg` Register address. 
* `val` Register value. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/display/syn_oled.c`

