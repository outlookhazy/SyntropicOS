

# Struct SYN\_Distance



[**ClassList**](annotated.md) **>** [**SYN\_Distance**](structSYN__Distance.md)



_Generic Distance Sensor Context._ 

* `#include <syn_distance.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**echo\_pin**](#variable-echo_pin)  <br> |
|  uint32\_t | [**last\_distance\_mm**](#variable-last_distance_mm)  <br> |
|  uint32\_t | [**max\_range\_mm**](#variable-max_range_mm)  <br> |
|  uint32\_t | [**min\_range\_mm**](#variable-min_range_mm)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**obstacle\_detected**](#variable-obstacle_detected)  <br> |
|  uint32\_t | [**proximity\_thresh\_mm**](#variable-proximity_thresh_mm)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**trig\_pin**](#variable-trig_pin)  <br> |
|  [**SYN\_DistanceType**](syn__distance_8h.md#enum-syn_distancetype) | [**type**](#variable-type)  <br> |












































## Public Attributes Documentation




### variable echo\_pin 

```C++
SYN_GPIO_Pin SYN_Distance::echo_pin;
```



Echo input / SDA GPIO pin 


        

<hr>



### variable last\_distance\_mm 

```C++
uint32_t SYN_Distance::last_distance_mm;
```



Calculated distance in millimeters 


        

<hr>



### variable max\_range\_mm 

```C++
uint32_t SYN_Distance::max_range_mm;
```



Max sensor range in mm 


        

<hr>



### variable min\_range\_mm 

```C++
uint32_t SYN_Distance::min_range_mm;
```



Min sensor range in mm 


        

<hr>



### variable obstacle\_detected 

```C++
bool SYN_Distance::obstacle_detected;
```



True if object within proximity threshold 


        

<hr>



### variable proximity\_thresh\_mm 

```C++
uint32_t SYN_Distance::proximity_thresh_mm;
```



Proximity alarm threshold in mm 


        

<hr>



### variable trig\_pin 

```C++
SYN_GPIO_Pin SYN_Distance::trig_pin;
```



Trigger pulse / SCL GPIO pin 


        

<hr>



### variable type 

```C++
SYN_DistanceType SYN_Distance::type;
```



Distance sensor model type 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sensor/syn_distance.h`

