

# File syn\_distance.h



[**FileList**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_distance.h**](syn__distance_8h.md)

[Go to the source code of this file](syn__distance_8h_source.md)

_Generic Distance & Proximity Sensor Driver (HC-SR04, VL53L0X, Sharp IR)._ 

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_gpio.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Distance**](structSYN__Distance.md) <br>_Generic Distance Sensor Context._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_DistanceType**](#enum-syn_distancetype)  <br>_Distance Sensor Type._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_distance\_feed\_pulse**](#function-syn_distance_feed_pulse) ([**SYN\_Distance**](structSYN__Distance.md) \* sensor, uint32\_t pulse\_us) <br>_Feed measured pulse duration in microseconds (for Ultrasonic) or raw mm (for TOF)._  |
|  uint32\_t | [**syn\_distance\_get\_cm**](#function-syn_distance_get_cm) (const [**SYN\_Distance**](structSYN__Distance.md) \* sensor) <br>_Get calculated distance in centimeters._  |
|  uint32\_t | [**syn\_distance\_get\_mm**](#function-syn_distance_get_mm) (const [**SYN\_Distance**](structSYN__Distance.md) \* sensor) <br>_Get calculated distance in millimeters._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_distance\_init**](#function-syn_distance_init) ([**SYN\_Distance**](structSYN__Distance.md) \* sensor, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) trig\_pin, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) echo\_pin, uint32\_t min\_mm, uint32\_t max\_mm, [**SYN\_DistanceType**](syn__distance_8h.md#enum-syn_distancetype) type) <br>_Initialize Distance Sensor._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_distance\_is\_obstacle\_detected**](#function-syn_distance_is_obstacle_detected) (const [**SYN\_Distance**](structSYN__Distance.md) \* sensor) <br>_Check if an object is within proximity threshold._  |
|  void | [**syn\_distance\_set\_proximity\_threshold**](#function-syn_distance_set_proximity_threshold) ([**SYN\_Distance**](structSYN__Distance.md) \* sensor, uint32\_t thresh\_mm) <br>_Set proximity alarm threshold in mm._  |




























## Public Types Documentation




### enum SYN\_DistanceType 

_Distance Sensor Type._ 
```C++
enum SYN_DistanceType {
    SYN_DISTANCE_ULTRASONIC = 0,
    SYN_DISTANCE_TOF_LASER = 1,
    SYN_DISTANCE_INFRARED = 2
};
```




<hr>
## Public Functions Documentation




### function syn\_distance\_feed\_pulse 

_Feed measured pulse duration in microseconds (for Ultrasonic) or raw mm (for TOF)._ 
```C++
void syn_distance_feed_pulse (
    SYN_Distance * sensor,
    uint32_t pulse_us
) 
```





**Parameters:**


* `sensor` Distance sensor context. 
* `pulse_us` Echo pulse width in microseconds (or raw mm). 




        

<hr>



### function syn\_distance\_get\_cm 

_Get calculated distance in centimeters._ 
```C++
uint32_t syn_distance_get_cm (
    const SYN_Distance * sensor
) 
```





**Parameters:**


* `sensor` Distance sensor context. 



**Returns:**

Distance in cm. 





        

<hr>



### function syn\_distance\_get\_mm 

_Get calculated distance in millimeters._ 
```C++
uint32_t syn_distance_get_mm (
    const SYN_Distance * sensor
) 
```





**Parameters:**


* `sensor` Distance sensor context. 



**Returns:**

Distance in mm. 





        

<hr>



### function syn\_distance\_init 

_Initialize Distance Sensor._ 
```C++
SYN_Status syn_distance_init (
    SYN_Distance * sensor,
    SYN_GPIO_Pin trig_pin,
    SYN_GPIO_Pin echo_pin,
    uint32_t min_mm,
    uint32_t max_mm,
    SYN_DistanceType type
) 
```





**Parameters:**


* `sensor` Distance sensor context. 
* `trig_pin` Trigger GPIO pin (or SCL for I2C TOF). 
* `echo_pin` Echo GPIO pin (or SDA for I2C TOF). 
* `min_mm` Min valid distance in mm (e.g. 20mm). 
* `max_mm` Max valid distance in mm (e.g. 4000mm). 
* `type` Sensor type. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_distance\_is\_obstacle\_detected 

_Check if an object is within proximity threshold._ 
```C++
bool syn_distance_is_obstacle_detected (
    const SYN_Distance * sensor
) 
```





**Parameters:**


* `sensor` Distance sensor context. 



**Returns:**

True if obstacle detected within threshold. 





        

<hr>



### function syn\_distance\_set\_proximity\_threshold 

_Set proximity alarm threshold in mm._ 
```C++
void syn_distance_set_proximity_threshold (
    SYN_Distance * sensor,
    uint32_t thresh_mm
) 
```





**Parameters:**


* `sensor` Distance sensor context. 
* `thresh_mm` Proximity threshold in mm. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sensor/syn_distance.h`

