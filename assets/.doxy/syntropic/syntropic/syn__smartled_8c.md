

# File syn\_smartled.c



[**FileList**](files.md) **>** [**output**](dir_1e1a1e0352188cb1bf4d84e529f1e02b.md) **>** [**syn\_smartled.c**](syn__smartled_8c.md)

[Go to the source code of this file](syn__smartled_8c_source.md)

_Generic Addressable RGB/RGBW Smart LED Strip Driver (WS2812B, SK6812, APA102, SK9822)._ 

* `#include "syn_smartled.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_smartled\_clear**](#function-syn_smartled_clear) ([**SYN\_SmartLED**](structSYN__SmartLED.md) \* led) <br>_Clear all pixels in strip (turn off all LEDs)._  |
|  void | [**syn\_smartled\_fill\_rgb**](#function-syn_smartled_fill_rgb) ([**SYN\_SmartLED**](structSYN__SmartLED.md) \* led, uint8\_t r, uint8\_t g, uint8\_t b) <br>_Fill entire strip with a single RGB color._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_smartled\_init**](#function-syn_smartled_init) ([**SYN\_SmartLED**](structSYN__SmartLED.md) \* led, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) data\_pin, uint16\_t num\_leds, [**SYN\_SmartLEDColor**](structSYN__SmartLEDColor.md) \* pixel\_buf, [**SYN\_SmartLEDOrder**](syn__smartled_8h.md#enum-syn_smartledorder) order) <br>_Initialize a Smart LED Strip context._  |
|  void | [**syn\_smartled\_set\_brightness**](#function-syn_smartled_set_brightness) ([**SYN\_SmartLED**](structSYN__SmartLED.md) \* led, uint8\_t brightness) <br>_Set global brightness scale (0 = off, 255 = 100% full brightness)._  |
|  void | [**syn\_smartled\_set\_pixel\_hsv**](#function-syn_smartled_set_pixel_hsv) ([**SYN\_SmartLED**](structSYN__SmartLED.md) \* led, uint16\_t index, uint8\_t h, uint8\_t s, uint8\_t v) <br>_Set HSV color of a specific LED pixel (0-255 hue, sat, val)._  |
|  void | [**syn\_smartled\_set\_pixel\_rgb**](#function-syn_smartled_set_pixel_rgb) ([**SYN\_SmartLED**](structSYN__SmartLED.md) \* led, uint16\_t index, uint8\_t r, uint8\_t g, uint8\_t b) <br>_Set RGB color of a specific LED pixel._  |




























## Public Functions Documentation




### function syn\_smartled\_clear 

_Clear all pixels in strip (turn off all LEDs)._ 
```C++
void syn_smartled_clear (
    SYN_SmartLED * led
) 
```





**Parameters:**


* `led` Smart LED context. 




        

<hr>



### function syn\_smartled\_fill\_rgb 

_Fill entire strip with a single RGB color._ 
```C++
void syn_smartled_fill_rgb (
    SYN_SmartLED * led,
    uint8_t r,
    uint8_t g,
    uint8_t b
) 
```





**Parameters:**


* `led` Smart LED context. 
* `r` Red channel. 
* `g` Green channel. 
* `b` Blue channel. 




        

<hr>



### function syn\_smartled\_init 

_Initialize a Smart LED Strip context._ 
```C++
SYN_Status syn_smartled_init (
    SYN_SmartLED * led,
    SYN_GPIO_Pin data_pin,
    uint16_t num_leds,
    SYN_SmartLEDColor * pixel_buf,
    SYN_SmartLEDOrder order
) 
```





**Parameters:**


* `led` Smart LED context. 
* `data_pin` GPIO data pin. 
* `num_leds` Total LED count in strip. 
* `pixel_buf` Array of [**SYN\_SmartLEDColor**](structSYN__SmartLEDColor.md) structs (length = num\_leds). 
* `order` Color channel order (GRB, RGB, RGBW). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_smartled\_set\_brightness 

_Set global brightness scale (0 = off, 255 = 100% full brightness)._ 
```C++
void syn_smartled_set_brightness (
    SYN_SmartLED * led,
    uint8_t brightness
) 
```





**Parameters:**


* `led` Smart LED context. 
* `brightness` Brightness scale (0 to 255). 




        

<hr>



### function syn\_smartled\_set\_pixel\_hsv 

_Set HSV color of a specific LED pixel (0-255 hue, sat, val)._ 
```C++
void syn_smartled_set_pixel_hsv (
    SYN_SmartLED * led,
    uint16_t index,
    uint8_t h,
    uint8_t s,
    uint8_t v
) 
```





**Parameters:**


* `led` Smart LED context. 
* `index` Pixel index (0 to num\_leds-1). 
* `h` Hue (0 to 255). 
* `s` Saturation (0 to 255). 
* `v` Value / Intensity (0 to 255). 




        

<hr>



### function syn\_smartled\_set\_pixel\_rgb 

_Set RGB color of a specific LED pixel._ 
```C++
void syn_smartled_set_pixel_rgb (
    SYN_SmartLED * led,
    uint16_t index,
    uint8_t r,
    uint8_t g,
    uint8_t b
) 
```





**Parameters:**


* `led` Smart LED context. 
* `index` Pixel index (0 to num\_leds-1). 
* `r` Red channel (0 to 255). 
* `g` Green channel (0 to 255). 
* `b` Blue channel (0 to 255). 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/output/syn_smartled.c`

