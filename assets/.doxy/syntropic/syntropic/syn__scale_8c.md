

# File syn\_scale.c



[**FileList**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_scale.c**](syn__scale_8c.md)

[Go to the source code of this file](syn__scale_8c_source.md)

_Generic Weight Scale & Load Cell Driver (HX711 24-bit ADC, NAU7802)._ 

* `#include "syn_scale.h"`
* `#include "../util/syn_assert.h"`
* `#include <math.h>`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_scale\_feed\_adc**](#function-syn_scale_feed_adc) ([**SYN\_Scale**](structSYN__Scale.md) \* scale, int32\_t raw\_adc24) <br>_Feed raw 24-bit ADC reading from load cell chip._  |
|  float | [**syn\_scale\_get\_grams**](#function-syn_scale_get_grams) (const [**SYN\_Scale**](structSYN__Scale.md) \* scale) <br>_Get calculated weight in grams._  |
|  float | [**syn\_scale\_get\_kg**](#function-syn_scale_get_kg) (const [**SYN\_Scale**](structSYN__Scale.md) \* scale) <br>_Get calculated weight in kilograms._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_scale\_init**](#function-syn_scale_init) ([**SYN\_Scale**](structSYN__Scale.md) \* scale, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) dout\_pin, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) sck\_pin, [**SYN\_ScaleType**](syn__scale_8h.md#enum-syn_scaletype) type) <br>_Initialize Weight Scale context._  |
|  void | [**syn\_scale\_set\_calibration\_factor**](#function-syn_scale_set_calibration_factor) ([**SYN\_Scale**](structSYN__Scale.md) \* scale, float factor) <br>_Set calibration scale factor (counts per gram)._  |
|  void | [**syn\_scale\_tare**](#function-syn_scale_tare) ([**SYN\_Scale**](structSYN__Scale.md) \* scale, int32\_t tare\_offset) <br>_Set zero-point Tare offset._  |




























## Public Functions Documentation




### function syn\_scale\_feed\_adc 

_Feed raw 24-bit ADC reading from load cell chip._ 
```C++
void syn_scale_feed_adc (
    SYN_Scale * scale,
    int32_t raw_adc24
) 
```





**Parameters:**


* `scale` Scale context. 
* `raw_adc24` Signed 24-bit raw ADC reading. 




        

<hr>



### function syn\_scale\_get\_grams 

_Get calculated weight in grams._ 
```C++
float syn_scale_get_grams (
    const SYN_Scale * scale
) 
```





**Parameters:**


* `scale` Scale context. 



**Returns:**

Weight in grams. 





        

<hr>



### function syn\_scale\_get\_kg 

_Get calculated weight in kilograms._ 
```C++
float syn_scale_get_kg (
    const SYN_Scale * scale
) 
```





**Parameters:**


* `scale` Scale context. 



**Returns:**

Weight in kg. 





        

<hr>



### function syn\_scale\_init 

_Initialize Weight Scale context._ 
```C++
SYN_Status syn_scale_init (
    SYN_Scale * scale,
    SYN_GPIO_Pin dout_pin,
    SYN_GPIO_Pin sck_pin,
    SYN_ScaleType type
) 
```





**Parameters:**


* `scale` Scale context. 
* `dout_pin` Data GPIO pin (or SDA for I2C). 
* `sck_pin` Clock GPIO pin (or SCL for I2C). 
* `type` Scale ADC type (HX711 or NAU7802). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_scale\_set\_calibration\_factor 

_Set calibration scale factor (counts per gram)._ 
```C++
void syn_scale_set_calibration_factor (
    SYN_Scale * scale,
    float factor
) 
```





**Parameters:**


* `scale` Scale context. 
* `factor` Calibration factor (e.g. 420.0f counts/gram). 




        

<hr>



### function syn\_scale\_tare 

_Set zero-point Tare offset._ 
```C++
void syn_scale_tare (
    SYN_Scale * scale,
    int32_t tare_offset
) 
```





**Parameters:**


* `scale` Scale context. 
* `tare_offset` Zero reading value. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sensor/syn_scale.c`

