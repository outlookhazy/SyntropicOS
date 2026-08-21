

# Struct SYN\_GCode\_Config



[**ClassList**](annotated.md) **>** [**SYN\_GCode\_Config**](structSYN__GCode__Config.md)



_G-Code controller configuration._ 

* `#include <syn_gcode.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  float | [**default\_feedrate**](#variable-default_feedrate)  <br> |
|  [**SYN\_Interpolator**](structSYN__Interpolator.md) \* | [**interpolator**](#variable-interpolator)  <br> |
|  float | [**max\_acceleration**](#variable-max_acceleration)  <br> |
|  float | [**max\_jerk**](#variable-max_jerk)  <br> |
|  [**SYN\_GCode\_CoolantCb**](syn__gcode_8h.md#typedef-syn_gcode_coolantcb) | [**on\_coolant**](#variable-on_coolant)  <br> |
|  [**SYN\_GCode\_SpindleCb**](syn__gcode_8h.md#typedef-syn_gcode_spindlecb) | [**on\_spindle**](#variable-on_spindle)  <br> |
|  [**SYN\_GCode\_ToolCb**](syn__gcode_8h.md#typedef-syn_gcode_toolcb) | [**on\_tool**](#variable-on_tool)  <br> |
|  float | [**step\_resolution**](#variable-step_resolution)  <br> |
|  void \* | [**user\_data**](#variable-user_data)  <br> |












































## Public Attributes Documentation




### variable default\_feedrate 

```C++
float SYN_GCode_Config::default_feedrate;
```



Initial feedrate (units/sec) 


        

<hr>



### variable interpolator 

```C++
SYN_Interpolator* SYN_GCode_Config::interpolator;
```



Associated motion interpolator instance 


        

<hr>



### variable max\_acceleration 

```C++
float SYN_GCode_Config::max_acceleration;
```



Max trajectory acceleration (units/sec^2) 


        

<hr>



### variable max\_jerk 

```C++
float SYN_GCode_Config::max_jerk;
```



Max trajectory jerk (units/sec^3) 


        

<hr>



### variable on\_coolant 

```C++
SYN_GCode_CoolantCb SYN_GCode_Config::on_coolant;
```



Coolant callback 


        

<hr>



### variable on\_spindle 

```C++
SYN_GCode_SpindleCb SYN_GCode_Config::on_spindle;
```



Spindle callback 


        

<hr>



### variable on\_tool 

```C++
SYN_GCode_ToolCb SYN_GCode_Config::on_tool;
```



Tool change callback 


        

<hr>



### variable step\_resolution 

```C++
float SYN_GCode_Config::step_resolution;
```



Step spatial resolution (e.g. 0.001) 


        

<hr>



### variable user\_data 

```C++
void* SYN_GCode_Config::user_data;
```



User context pointer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_gcode.h`

