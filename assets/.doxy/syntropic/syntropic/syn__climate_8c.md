

# File syn\_climate.c



[**FileList**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_climate.c**](syn__climate_8c.md)

[Go to the source code of this file](syn__climate_8c_source.md)

_Generic Climate & Environmental Sensor Driver (SHT3x, BME280, HTU21D, DHT22)._ 

* `#include "syn_climate.h"`
* `#include "../util/syn_assert.h"`
* `#include <math.h>`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_climate\_feed\_sample**](#function-syn_climate_feed_sample) ([**SYN\_Climate**](structSYN__Climate.md) \* sensor, float temp\_c, float humidity\_rh, float press\_hpa) <br>_Feed raw sensor readings (temperature, humidity, pressure)._  |
|  float | [**syn\_climate\_get\_dew\_point**](#function-syn_climate_get_dew_point) (const [**SYN\_Climate**](structSYN__Climate.md) \* sensor) <br>_Get calculated dew point in Celsius._  |
|  float | [**syn\_climate\_get\_humidity**](#function-syn_climate_get_humidity) (const [**SYN\_Climate**](structSYN__Climate.md) \* sensor) <br>_Get relative humidity %._  |
|  float | [**syn\_climate\_get\_temp\_c**](#function-syn_climate_get_temp_c) (const [**SYN\_Climate**](structSYN__Climate.md) \* sensor) <br>_Get temperature in Celsius._  |
|  float | [**syn\_climate\_get\_temp\_f**](#function-syn_climate_get_temp_f) (const [**SYN\_Climate**](structSYN__Climate.md) \* sensor) <br>_Get temperature in Fahrenheit._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_climate\_init**](#function-syn_climate_init) ([**SYN\_Climate**](structSYN__Climate.md) \* sensor, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) scl, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) sda, uint8\_t i2c\_addr, [**SYN\_ClimateType**](syn__climate_8h.md#enum-syn_climatetype) type) <br>_Initialize Climate Sensor._  |




























## Public Functions Documentation




### function syn\_climate\_feed\_sample 

_Feed raw sensor readings (temperature, humidity, pressure)._ 
```C++
void syn_climate_feed_sample (
    SYN_Climate * sensor,
    float temp_c,
    float humidity_rh,
    float press_hpa
) 
```





**Parameters:**


* `sensor` Climate sensor context. 
* `temp_c` Temperature in Celsius. 
* `humidity_rh` Relative humidity percentage (0 to 100%). 
* `press_hpa` Barometric pressure in hPa (e.g. 1013.25f). 




        

<hr>



### function syn\_climate\_get\_dew\_point 

_Get calculated dew point in Celsius._ 
```C++
float syn_climate_get_dew_point (
    const SYN_Climate * sensor
) 
```





**Parameters:**


* `sensor` Climate sensor context. 



**Returns:**

Dew point in °C. 





        

<hr>



### function syn\_climate\_get\_humidity 

_Get relative humidity %._ 
```C++
float syn_climate_get_humidity (
    const SYN_Climate * sensor
) 
```





**Parameters:**


* `sensor` Climate sensor context. 



**Returns:**

Humidity RH. 





        

<hr>



### function syn\_climate\_get\_temp\_c 

_Get temperature in Celsius._ 
```C++
float syn_climate_get_temp_c (
    const SYN_Climate * sensor
) 
```





**Parameters:**


* `sensor` Climate sensor context. 



**Returns:**

Temperature in °C. 





        

<hr>



### function syn\_climate\_get\_temp\_f 

_Get temperature in Fahrenheit._ 
```C++
float syn_climate_get_temp_f (
    const SYN_Climate * sensor
) 
```





**Parameters:**


* `sensor` Climate sensor context. 



**Returns:**

Temperature in °F. 





        

<hr>



### function syn\_climate\_init 

_Initialize Climate Sensor._ 
```C++
SYN_Status syn_climate_init (
    SYN_Climate * sensor,
    SYN_GPIO_Pin scl,
    SYN_GPIO_Pin sda,
    uint8_t i2c_addr,
    SYN_ClimateType type
) 
```





**Parameters:**


* `sensor` Climate sensor context. 
* `scl` I2C SCL GPIO pin. 
* `sda` I2C SDA GPIO pin. 
* `i2c_addr` I2C slave address (e.g. 0x44 or 0x76). 
* `type` Sensor type (SHT3X, BME280, HTU21D). 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sensor/syn_climate.c`

