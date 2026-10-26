

# Struct SYN\_Sensor



[**ClassList**](annotated.md) **>** [**SYN\_Sensor**](structSYN__Sensor.md)



_Sensor descriptor — owns the full read→filter→threshold pipeline._ 

* `#include <syn_sensor.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**enabled**](#variable-enabled)  <br> |
|  void \* | [**filter**](#variable-filter)  <br> |
|  uint8\_t | [**filter\_type**](#variable-filter_type)  <br> |
|  int16\_t | [**filtered**](#variable-filtered)  <br> |
|  [**SYN\_Hysteresis**](structSYN__Hysteresis.md) | [**hyst**](#variable-hyst)  <br> |
|  uint32\_t | [**interval\_ms**](#variable-interval_ms)  <br> |
|  uint32\_t | [**last\_poll\_tick**](#variable-last_poll_tick)  <br> |
|  const char \* | [**name**](#variable-name)  <br> |
|  [**SYN\_SensorThresholdCallback**](syn__sensor_8h.md#typedef-syn_sensorthresholdcallback) | [**on\_high**](#variable-on_high)  <br> |
|  [**SYN\_SensorThresholdCallback**](syn__sensor_8h.md#typedef-syn_sensorthresholdcallback) | [**on\_low**](#variable-on_low)  <br> |
|  int16\_t | [**raw**](#variable-raw)  <br> |
|  void \* | [**read\_ctx**](#variable-read_ctx)  <br> |
|  [**SYN\_SensorReadFunc**](syn__sensor_8h.md#typedef-syn_sensorreadfunc) | [**read\_func**](#variable-read_func)  <br> |
|  [**SYN\_Signal**](structSYN__Signal.md) \* | [**stats**](#variable-stats)  <br> |
|  void \* | [**threshold\_ctx**](#variable-threshold_ctx)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**threshold\_enabled**](#variable-threshold_enabled)  <br> |












































## Public Attributes Documentation




### variable enabled 

```C++
bool SYN_Sensor::enabled;
```



Polling enabled flag 
 


        

<hr>



### variable filter 

```C++
void* SYN_Sensor::filter;
```



Pointer to filter instance 
 


        

<hr>



### variable filter\_type 

```C++
uint8_t SYN_Sensor::filter_type;
```



SYN\_SensorFilterType 
 


        

<hr>



### variable filtered 

```C++
int16_t SYN_Sensor::filtered;
```



Last filtered reading 
 


        

<hr>



### variable hyst 

```C++
SYN_Hysteresis SYN_Sensor::hyst;
```



Hysteresis state 
 


        

<hr>



### variable interval\_ms 

```C++
uint32_t SYN_Sensor::interval_ms;
```



Poll interval in ms 
 


        

<hr>



### variable last\_poll\_tick 

```C++
uint32_t SYN_Sensor::last_poll_tick;
```



Tick of last poll 
 


        

<hr>



### variable name 

```C++
const char* SYN_Sensor::name;
```



Human-readable name 
 


        

<hr>



### variable on\_high 

```C++
SYN_SensorThresholdCallback SYN_Sensor::on_high;
```



Above-threshold callback 
 


        

<hr>



### variable on\_low 

```C++
SYN_SensorThresholdCallback SYN_Sensor::on_low;
```



Below-threshold callback 
 


        

<hr>



### variable raw 

```C++
int16_t SYN_Sensor::raw;
```



Last raw reading 
 


        

<hr>



### variable read\_ctx 

```C++
void* SYN_Sensor::read_ctx;
```



Context for read callback 
 


        

<hr>



### variable read\_func 

```C++
SYN_SensorReadFunc SYN_Sensor::read_func;
```



Hardware read callback 
 


        

<hr>



### variable stats 

```C++
SYN_Signal* SYN_Sensor::stats;
```



Sliding window stats (if set) 


        

<hr>



### variable threshold\_ctx 

```C++
void* SYN_Sensor::threshold_ctx;
```



Context for threshold cbs 
 


        

<hr>



### variable threshold\_enabled 

```C++
bool SYN_Sensor::threshold_enabled;
```



Threshold monitoring on 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sensor/syn_sensor.h`

