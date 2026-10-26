

# Struct SYN\_Interpolator



[**ClassList**](annotated.md) **>** [**SYN\_Interpolator**](structSYN__Interpolator.md)



_Multi-Axis Interpolator configuration and state machine._ 

* `#include <syn_interpolator.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_Vector3F**](structSYN__Vector3F.md) | [**center\_offset**](#variable-center_offset)  <br> |
|  float | [**current\_dist**](#variable-current_dist)  <br> |
|  [**SYN\_Vector3F**](structSYN__Vector3F.md) | [**current\_pos**](#variable-current_pos)  <br> |
|  [**SYN\_Interp\_Mode**](syn__interpolator_8h.md#enum-syn_interp_mode) | [**mode**](#variable-mode)  <br> |
|  float | [**radius**](#variable-radius)  <br> |
|  [**SYN\_SCurve**](structSYN__SCurve.md) | [**scurve**](#variable-scurve)  <br> |
|  float | [**start\_angle**](#variable-start_angle)  <br> |
|  [**SYN\_Vector3F**](structSYN__Vector3F.md) | [**start\_pos**](#variable-start_pos)  <br> |
|  uint32\_t | [**step\_index**](#variable-step_index)  <br> |
|  float | [**sweep\_angle**](#variable-sweep_angle)  <br> |
|  float | [**target\_feedrate**](#variable-target_feedrate)  <br> |
|  [**SYN\_Vector3F**](structSYN__Vector3F.md) | [**target\_pos**](#variable-target_pos)  <br> |
|  float | [**total\_length**](#variable-total_length)  <br> |
|  uint32\_t | [**total\_steps**](#variable-total_steps)  <br> |












































## Public Attributes Documentation




### variable center\_offset 

```C++
SYN_Vector3F SYN_Interpolator::center_offset;
```



Circle center offset relative to start (I, J, K) 


        

<hr>



### variable current\_dist 

```C++
float SYN_Interpolator::current_dist;
```



Current traveled distance along path 


        

<hr>



### variable current\_pos 

```C++
SYN_Vector3F SYN_Interpolator::current_pos;
```



Calculated current position 


        

<hr>



### variable mode 

```C++
SYN_Interp_Mode SYN_Interpolator::mode;
```



Active interpolation mode 


        

<hr>



### variable radius 

```C++
float SYN_Interpolator::radius;
```



Arc radius (for circular mode) 


        

<hr>



### variable scurve 

```C++
SYN_SCurve SYN_Interpolator::scurve;
```



S-Curve velocity profile generator 


        

<hr>



### variable start\_angle 

```C++
float SYN_Interpolator::start_angle;
```



Arc start angle in radians 


        

<hr>



### variable start\_pos 

```C++
SYN_Vector3F SYN_Interpolator::start_pos;
```



Segment start point (X, Y, Z) 


        

<hr>



### variable step\_index 

```C++
uint32_t SYN_Interpolator::step_index;
```



Current step index (0 .. total\_steps) 


        

<hr>



### variable sweep\_angle 

```C++
float SYN_Interpolator::sweep_angle;
```



Arc total sweep angle in radians 


        

<hr>



### variable target\_feedrate 

```C++
float SYN_Interpolator::target_feedrate;
```



Max velocity along path (units/sec) 


        

<hr>



### variable target\_pos 

```C++
SYN_Vector3F SYN_Interpolator::target_pos;
```



Segment target point (X, Y, Z) 


        

<hr>



### variable total\_length 

```C++
float SYN_Interpolator::total_length;
```



Total path length in units 


        

<hr>



### variable total\_steps 

```C++
uint32_t SYN_Interpolator::total_steps;
```



Number of discrete steps in move 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_interpolator.h`

