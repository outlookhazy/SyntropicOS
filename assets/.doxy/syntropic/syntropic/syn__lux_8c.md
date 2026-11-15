

# File syn\_lux.c



[**FileList**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_lux.c**](syn__lux_8c.md)

[Go to the source code of this file](syn__lux_8c_source.md)

_Generic Ambient Light & RGB Color Sensor Driver (BH1750, TSL2561, TCS34725)._ 

* `#include "syn_lux.h"`
* `#include "../util/syn_assert.h"`
* `#include <math.h>`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_lux\_feed\_lux**](#function-syn_lux_feed_lux) ([**SYN\_Lux**](structSYN__Lux.md) \* sensor, float lux) <br>_Feed raw illuminance reading in Lux (for BH1750/TSL2561)._  |
|  void | [**syn\_lux\_feed\_rgbc**](#function-syn_lux_feed_rgbc) ([**SYN\_Lux**](structSYN__Lux.md) \* sensor, uint16\_t r, uint16\_t g, uint16\_t b, uint16\_t c) <br>_Feed raw RGBC channels (for TCS34725)._  |
|  uint16\_t | [**syn\_lux\_get\_color\_temp\_k**](#function-syn_lux_get_color_temp_k) (const [**SYN\_Lux**](structSYN__Lux.md) \* sensor) <br>_Get Correlated Color Temperature in Kelvin._  |
|  float | [**syn\_lux\_get\_lux**](#function-syn_lux_get_lux) (const [**SYN\_Lux**](structSYN__Lux.md) \* sensor) <br>_Get ambient illuminance in Lux._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_lux\_init**](#function-syn_lux_init) ([**SYN\_Lux**](structSYN__Lux.md) \* sensor, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) scl, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) sda, uint8\_t i2c\_addr, [**SYN\_LuxType**](syn__lux_8h.md#enum-syn_luxtype) type) <br>_Initialize Light & Color Sensor._  |




























## Public Functions Documentation




### function syn\_lux\_feed\_lux 

_Feed raw illuminance reading in Lux (for BH1750/TSL2561)._ 
```C++
void syn_lux_feed_lux (
    SYN_Lux * sensor,
    float lux
) 
```





**Parameters:**


* `sensor` Lux sensor context. 
* `lux` Ambient light value in Lux. 




        

<hr>



### function syn\_lux\_feed\_rgbc 

_Feed raw RGBC channels (for TCS34725)._ 
```C++
void syn_lux_feed_rgbc (
    SYN_Lux * sensor,
    uint16_t r,
    uint16_t g,
    uint16_t b,
    uint16_t c
) 
```





**Parameters:**


* `sensor` Lux sensor context. 
* `r` Red channel raw ADC. 
* `g` Green channel raw ADC. 
* `b` Blue channel raw ADC. 
* `c` Clear channel raw ADC. 




        

<hr>



### function syn\_lux\_get\_color\_temp\_k 

_Get Correlated Color Temperature in Kelvin._ 
```C++
uint16_t syn_lux_get_color_temp_k (
    const SYN_Lux * sensor
) 
```





**Parameters:**


* `sensor` Lux sensor context. 



**Returns:**

Color Temp in K. 





        

<hr>



### function syn\_lux\_get\_lux 

_Get ambient illuminance in Lux._ 
```C++
float syn_lux_get_lux (
    const SYN_Lux * sensor
) 
```





**Parameters:**


* `sensor` Lux sensor context. 



**Returns:**

Lux value. 





        

<hr>



### function syn\_lux\_init 

_Initialize Light & Color Sensor._ 
```C++
SYN_Status syn_lux_init (
    SYN_Lux * sensor,
    SYN_GPIO_Pin scl,
    SYN_GPIO_Pin sda,
    uint8_t i2c_addr,
    SYN_LuxType type
) 
```





**Parameters:**


* `sensor` Lux sensor context. 
* `scl` I2C SCL GPIO pin. 
* `sda` I2C SDA GPIO pin. 
* `i2c_addr` I2C slave address (e.g. 0x23 or 0x29). 
* `type` Sensor type (BH1750, TSL2561, TCS34725). 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sensor/syn_lux.c`

