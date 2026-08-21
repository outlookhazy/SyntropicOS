

# Struct SYN\_LwM2M\_SensorContext



[**ClassList**](annotated.md) **>** [**SYN\_LwM2M\_SensorContext**](structSYN__LwM2M__SensorContext.md)



_Generic Sensor / Temperature Object (Object ID 3303) context._ 

* `#include <syn_lwm2m.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  double | [**max\_measured\_val**](#variable-max_measured_val)  <br> |
|  double | [**min\_measured\_val**](#variable-min_measured_val)  <br> |
|  double | [**sensor\_value**](#variable-sensor_value)  <br> |
|  const char \* | [**unit**](#variable-unit)  <br> |












































## Public Attributes Documentation




### variable max\_measured\_val 

```C++
double SYN_LwM2M_SensorContext::max_measured_val;
```



Res 5602: Max Measured Value 


        

<hr>



### variable min\_measured\_val 

```C++
double SYN_LwM2M_SensorContext::min_measured_val;
```



Res 5601: Min Measured Value 


        

<hr>



### variable sensor\_value 

```C++
double SYN_LwM2M_SensorContext::sensor_value;
```



Res 5700: Sensor Value 


        

<hr>



### variable unit 

```C++
const char* SYN_LwM2M_SensorContext::unit;
```



Res 5701: Sensor Unit (e.g. "Cel") 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lwm2m.h`

