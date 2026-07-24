

# File syn\_sensor.h



[**FileList**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_sensor.h**](syn__sensor_8h.md)

[Go to the source code of this file](syn__sensor_8h_source.md)

_Sensor polling framework — periodic read → filter → threshold → event._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_system.h"`
* `#include "../dsp/syn_filter.h"`
* `#include "../dsp/syn_signal.h"`
* `#include "../util/syn_hysteresis.h"`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Sensor**](structSYN__Sensor.md) <br>_Sensor descriptor — owns the full read→filter→threshold pipeline._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_SensorFilterType**](#enum-syn_sensorfiltertype)  <br>_Filter type selector for sensor pipeline._  |
| typedef int16\_t(\* | [**SYN\_SensorReadFunc**](#typedef-syn_sensorreadfunc)  <br>_Read function — returns the raw sensor value._  |
| typedef void(\* | [**SYN\_SensorThresholdCallback**](#typedef-syn_sensorthresholdcallback)  <br>_Threshold callback — called when threshold is crossed._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_sensor\_clear\_filter**](#function-syn_sensor_clear_filter) ([**SYN\_Sensor**](structSYN__Sensor.md) \* sensor) <br>_Remove any attached filter._  |
|  void | [**syn\_sensor\_clear\_threshold**](#function-syn_sensor_clear_threshold) ([**SYN\_Sensor**](structSYN__Sensor.md) \* sensor) <br>_Disable threshold monitoring._  |
|  void | [**syn\_sensor\_enable**](#function-syn_sensor_enable) ([**SYN\_Sensor**](structSYN__Sensor.md) \* sensor, [**bool**](syn__defs_8h.md#enum-bool) enable) <br>_Enable or disable polling._  |
|  void | [**syn\_sensor\_init**](#function-syn_sensor_init) ([**SYN\_Sensor**](structSYN__Sensor.md) \* sensor, const char \* name, [**SYN\_SensorReadFunc**](syn__sensor_8h.md#typedef-syn_sensorreadfunc) read\_func, void \* ctx) <br>_Initialize a sensor._  |
|  int16\_t | [**syn\_sensor\_read\_now**](#function-syn_sensor_read_now) ([**SYN\_Sensor**](structSYN__Sensor.md) \* sensor) <br>_Force an immediate reading (ignoring interval)._  |
|  void | [**syn\_sensor\_service**](#function-syn_sensor_service) ([**SYN\_Sensor**](structSYN__Sensor.md) \* sensors, size\_t count) <br>_Service an array of sensors (calls syn\_sensor\_update on each)._  |
|  void | [**syn\_sensor\_set\_filter\_ema**](#function-syn_sensor_set_filter_ema) ([**SYN\_Sensor**](structSYN__Sensor.md) \* sensor, [**SYN\_FilterEMA**](structSYN__FilterEMA.md) \* f) <br>_Attach an exponential moving-average filter._  |
|  void | [**syn\_sensor\_set\_filter\_ma**](#function-syn_sensor_set_filter_ma) ([**SYN\_Sensor**](structSYN__Sensor.md) \* sensor, [**SYN\_FilterMA**](structSYN__FilterMA.md) \* f) <br>_Attach a moving-average filter._  |
|  void | [**syn\_sensor\_set\_filter\_median**](#function-syn_sensor_set_filter_median) ([**SYN\_Sensor**](structSYN__Sensor.md) \* sensor, [**SYN\_FilterMedian**](structSYN__FilterMedian.md) \* f) <br>_Attach a median filter._  |
|  void | [**syn\_sensor\_set\_interval**](#function-syn_sensor_set_interval) ([**SYN\_Sensor**](structSYN__Sensor.md) \* sensor, uint32\_t interval\_ms) <br>_Set polling interval in milliseconds._  |
|  void | [**syn\_sensor\_set\_stats**](#function-syn_sensor_set_stats) ([**SYN\_Sensor**](structSYN__Sensor.md) \* sensor, [**SYN\_Signal**](structSYN__Signal.md) \* stats) <br>_Attach a signal statistics window._  |
|  void | [**syn\_sensor\_set\_threshold**](#function-syn_sensor_set_threshold) ([**SYN\_Sensor**](structSYN__Sensor.md) \* sensor, int32\_t threshold, int32\_t band, [**SYN\_SensorThresholdCallback**](syn__sensor_8h.md#typedef-syn_sensorthresholdcallback) on\_high, [**SYN\_SensorThresholdCallback**](syn__sensor_8h.md#typedef-syn_sensorthresholdcallback) on\_low, void \* ctx) <br>_Set a threshold with hysteresis and callbacks._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_sensor\_update**](#function-syn_sensor_update) ([**SYN\_Sensor**](structSYN__Sensor.md) \* sensor) <br>_Update the sensor — poll if interval elapsed, filter, check threshold._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  int16\_t | [**syn\_sensor\_raw**](#function-syn_sensor_raw) (const [**SYN\_Sensor**](structSYN__Sensor.md) \* sensor) <br>_Get the last raw value._  |
|  int16\_t | [**syn\_sensor\_value**](#function-syn_sensor_value) (const [**SYN\_Sensor**](structSYN__Sensor.md) \* sensor) <br>_Get the last filtered value._  |


























## Detailed Description


Wires up the full sensor pipeline: poll at an interval, filter the reading, compare against thresholds (with hysteresis), and fire callbacks or set event flags.




**
**


```C++
static SYN_Sensor temp_sensor;
static SYN_FilterEMA temp_filter;
syn_filter_ema_init(&temp_filter, 64);

syn_sensor_init(&temp_sensor, "temp", read_temperature, NULL);
syn_sensor_set_interval(&temp_sensor, 1000);      // poll every 1s
syn_sensor_set_filter_ema(&temp_sensor, &temp_filter);
syn_sensor_set_threshold(&temp_sensor, 8000, 500,  // 80.00°C ± 5°C
                           on_temp_high, on_temp_low, NULL);

// In main loop:
syn_sensor_update(&temp_sensor);
```
 





    
## Public Types Documentation




### enum SYN\_SensorFilterType 

_Filter type selector for sensor pipeline._ 
```C++
enum SYN_SensorFilterType {
    SYN_SENSOR_FILTER_NONE = 0,
    SYN_SENSOR_FILTER_MA = 1,
    SYN_SENSOR_FILTER_EMA = 2,
    SYN_SENSOR_FILTER_MEDIAN = 3
};
```




<hr>



### typedef SYN\_SensorReadFunc 

_Read function — returns the raw sensor value._ 
```C++
typedef int16_t(* SYN_SensorReadFunc) (void *ctx);
```





**Parameters:**


* `ctx` User context pointer. 



**Returns:**

Raw sensor reading. 





        

<hr>



### typedef SYN\_SensorThresholdCallback 

_Threshold callback — called when threshold is crossed._ 
```C++
typedef void(* SYN_SensorThresholdCallback) (struct SYN_Sensor *sensor, int16_t value, void *ctx);
```





**Parameters:**


* `sensor` The sensor that fired. 
* `value` Current filtered value. 
* `ctx` User context pointer. 




        

<hr>
## Public Functions Documentation




### function syn\_sensor\_clear\_filter 

_Remove any attached filter._ 
```C++
void syn_sensor_clear_filter (
    SYN_Sensor * sensor
) 
```





**Parameters:**


* `sensor` Sensor. 




        

<hr>



### function syn\_sensor\_clear\_threshold 

_Disable threshold monitoring._ 
```C++
void syn_sensor_clear_threshold (
    SYN_Sensor * sensor
) 
```





**Parameters:**


* `sensor` Sensor. 




        

<hr>



### function syn\_sensor\_enable 

_Enable or disable polling._ 
```C++
void syn_sensor_enable (
    SYN_Sensor * sensor,
    bool enable
) 
```





**Parameters:**


* `sensor` Sensor. 
* `enable` true to enable, false to disable. 




        

<hr>



### function syn\_sensor\_init 

_Initialize a sensor._ 
```C++
void syn_sensor_init (
    SYN_Sensor * sensor,
    const char * name,
    SYN_SensorReadFunc read_func,
    void * ctx
) 
```





**Parameters:**


* `sensor` Sensor instance. 
* `name` Human-readable name (for logging). 
* `read_func` Function that reads the sensor hardware. 
* `ctx` Context passed to read\_func. 




        

<hr>



### function syn\_sensor\_read\_now 

_Force an immediate reading (ignoring interval)._ 
```C++
int16_t syn_sensor_read_now (
    SYN_Sensor * sensor
) 
```





**Parameters:**


* `sensor` Sensor. 



**Returns:**

The filtered value. 





        

<hr>



### function syn\_sensor\_service 

_Service an array of sensors (calls syn\_sensor\_update on each)._ 
```C++
void syn_sensor_service (
    SYN_Sensor * sensors,
    size_t count
) 
```





**Parameters:**


* `sensors` Array of sensors. 
* `count` Number of sensors. 




        

<hr>



### function syn\_sensor\_set\_filter\_ema 

_Attach an exponential moving-average filter._ 
```C++
void syn_sensor_set_filter_ema (
    SYN_Sensor * sensor,
    SYN_FilterEMA * f
) 
```





**Parameters:**


* `sensor` Sensor. 
* `f` Initialized EMA filter. 




        

<hr>



### function syn\_sensor\_set\_filter\_ma 

_Attach a moving-average filter._ 
```C++
void syn_sensor_set_filter_ma (
    SYN_Sensor * sensor,
    SYN_FilterMA * f
) 
```





**Parameters:**


* `sensor` Sensor. 
* `f` Initialized MA filter. 




        

<hr>



### function syn\_sensor\_set\_filter\_median 

_Attach a median filter._ 
```C++
void syn_sensor_set_filter_median (
    SYN_Sensor * sensor,
    SYN_FilterMedian * f
) 
```





**Parameters:**


* `sensor` Sensor. 
* `f` Initialized median filter. 




        

<hr>



### function syn\_sensor\_set\_interval 

_Set polling interval in milliseconds._ 
```C++
void syn_sensor_set_interval (
    SYN_Sensor * sensor,
    uint32_t interval_ms
) 
```





**Parameters:**


* `sensor` Sensor. 
* `interval_ms` Poll interval. 




        

<hr>



### function syn\_sensor\_set\_stats 

_Attach a signal statistics window._ 
```C++
void syn_sensor_set_stats (
    SYN_Sensor * sensor,
    SYN_Signal * stats
) 
```



Each update pushes the filtered value into the signal window, giving you min/max/mean/variance/delta automatically.




**Parameters:**


* `sensor` Sensor. 
* `stats` Initialized [**SYN\_Signal**](structSYN__Signal.md) instance, or NULL to detach. 




        

<hr>



### function syn\_sensor\_set\_threshold 

_Set a threshold with hysteresis and callbacks._ 
```C++
void syn_sensor_set_threshold (
    SYN_Sensor * sensor,
    int32_t threshold,
    int32_t band,
    SYN_SensorThresholdCallback on_high,
    SYN_SensorThresholdCallback on_low,
    void * ctx
) 
```





**Parameters:**


* `sensor` Sensor. 
* `threshold` Center threshold value. 
* `band` Hysteresis half-width. 
* `on_high` Called when value crosses above (threshold + band). 
* `on_low` Called when value drops below (threshold - band). 
* `ctx` Context for callbacks. 




        

<hr>



### function syn\_sensor\_update 

_Update the sensor — poll if interval elapsed, filter, check threshold._ 
```C++
bool syn_sensor_update (
    SYN_Sensor * sensor
) 
```



Call from your main loop or scheduler task.




**Parameters:**


* `sensor` Sensor. 



**Returns:**

true if a new reading was taken this call. 





        

<hr>
## Public Static Functions Documentation




### function syn\_sensor\_raw 

_Get the last raw value._ 
```C++
static inline int16_t syn_sensor_raw (
    const SYN_Sensor * sensor
) 
```





**Parameters:**


* `sensor` Sensor. 



**Returns:**

Raw reading. 





        

<hr>



### function syn\_sensor\_value 

_Get the last filtered value._ 
```C++
static inline int16_t syn_sensor_value (
    const SYN_Sensor * sensor
) 
```





**Parameters:**


* `sensor` Sensor. 



**Returns:**

Filtered reading. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sensor/syn_sensor.h`

