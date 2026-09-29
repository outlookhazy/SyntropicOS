

# Struct SYN\_GCode\_Controller



[**ClassList**](annotated.md) **>** [**SYN\_GCode\_Controller**](structSYN__GCode__Controller.md)



_G-Code interpreter and controller instance._ 

* `#include <syn_gcode.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**active\_tool**](#variable-active_tool)  <br> |
|  [**SYN\_GCode\_Config**](structSYN__GCode__Config.md) | [**cfg**](#variable-cfg)  <br> |
|  [**SYN\_GCode\_CoolantMode**](syn__gcode_8h.md#enum-syn_gcode_coolantmode) | [**coolant**](#variable-coolant)  <br> |
|  [**SYN\_Vector3F**](structSYN__Vector3F.md) | [**current\_pos**](#variable-current_pos)  <br> |
|  [**SYN\_GCode\_DistanceMode**](syn__gcode_8h.md#enum-syn_gcode_distancemode) | [**dist\_mode**](#variable-dist_mode)  <br> |
|  float | [**feedrate**](#variable-feedrate)  <br> |
|  [**SYN\_GCode\_MotionMode**](syn__gcode_8h.md#enum-syn_gcode_motionmode) | [**motion\_mode**](#variable-motion_mode)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**move\_in\_progress**](#variable-move_in_progress)  <br> |
|  [**SYN\_Vector3F**](structSYN__Vector3F.md) | [**offset\_pos**](#variable-offset_pos)  <br> |
|  [**SYN\_GCode\_Plane**](syn__gcode_8h.md#enum-syn_gcode_plane) | [**plane**](#variable-plane)  <br> |
|  [**SYN\_PT**](structSYN__PT.md) | [**pt**](#variable-pt)  <br> |
|  [**SYN\_GCode\_SpindleMode**](syn__gcode_8h.md#enum-syn_gcode_spindlemode) | [**spindle**](#variable-spindle)  <br> |
|  float | [**spindle\_speed**](#variable-spindle_speed)  <br> |
|  [**SYN\_GCode\_Units**](syn__gcode_8h.md#enum-syn_gcode_units) | [**units**](#variable-units)  <br> |












































## Public Attributes Documentation




### variable active\_tool 

```C++
uint32_t SYN_GCode_Controller::active_tool;
```



Active tool number 


        

<hr>



### variable cfg 

```C++
SYN_GCode_Config SYN_GCode_Controller::cfg;
```



Controller configuration 


        

<hr>



### variable coolant 

```C++
SYN_GCode_CoolantMode SYN_GCode_Controller::coolant;
```



Active coolant state 


        

<hr>



### variable current\_pos 

```C++
SYN_Vector3F SYN_GCode_Controller::current_pos;
```



Current machine coordinate position 


        

<hr>



### variable dist\_mode 

```C++
SYN_GCode_DistanceMode SYN_GCode_Controller::dist_mode;
```



Active distance mode (G90/G91) 


        

<hr>



### variable feedrate 

```C++
float SYN_GCode_Controller::feedrate;
```



Active feedrate (units/sec) 


        

<hr>



### variable motion\_mode 

```C++
SYN_GCode_MotionMode SYN_GCode_Controller::motion_mode;
```



Active motion mode 


        

<hr>



### variable move\_in\_progress 

```C++
bool SYN_GCode_Controller::move_in_progress;
```



Motion move currently in execution 


        

<hr>



### variable offset\_pos 

```C++
SYN_Vector3F SYN_GCode_Controller::offset_pos;
```



G92 Coordinate system offset 


        

<hr>



### variable plane 

```C++
SYN_GCode_Plane SYN_GCode_Controller::plane;
```



Active plane selection 


        

<hr>



### variable pt 

```C++
SYN_PT SYN_GCode_Controller::pt;
```



Protothread state 


        

<hr>



### variable spindle 

```C++
SYN_GCode_SpindleMode SYN_GCode_Controller::spindle;
```



Active spindle state 


        

<hr>



### variable spindle\_speed 

```C++
float SYN_GCode_Controller::spindle_speed;
```



Active spindle speed (RPM) 


        

<hr>



### variable units 

```C++
SYN_GCode_Units SYN_GCode_Controller::units;
```



Active unit system 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_gcode.h`

