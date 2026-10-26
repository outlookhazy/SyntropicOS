

# Struct SYN\_Actuator



[**ClassList**](annotated.md) **>** [**SYN\_Actuator**](structSYN__Actuator.md)



_Linear actuator instance — motor controller + stroke mapping._ 

* `#include <syn_actuator.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) | [**ctrl**](#variable-ctrl)  <br> |
|  int16\_t | [**current\_pct**](#variable-current_pct)  <br> |
|  int32\_t | [**stroke\_max**](#variable-stroke_max)  <br> |
|  int32\_t | [**stroke\_min**](#variable-stroke_min)  <br> |
|  int32\_t | [**stroke\_range**](#variable-stroke_range)  <br> |
|  int16\_t | [**target\_pct**](#variable-target_pct)  <br> |












































## Public Attributes Documentation




### variable ctrl 

```C++
SYN_MotorCtrl SYN_Actuator::ctrl;
```



Underlying motor controller 
 


        

<hr>



### variable current\_pct 

```C++
int16_t SYN_Actuator::current_pct;
```



Current position 0-1000 
 


        

<hr>



### variable stroke\_max 

```C++
int32_t SYN_Actuator::stroke_max;
```



ADC at extended 
 


        

<hr>



### variable stroke\_min 

```C++
int32_t SYN_Actuator::stroke_min;
```



ADC at retracted 
 


        

<hr>



### variable stroke\_range 

```C++
int32_t SYN_Actuator::stroke_range;
```



max - min (precomputed) 
 


        

<hr>



### variable target\_pct 

```C++
int16_t SYN_Actuator::target_pct;
```



Target position 0-1000 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_actuator.h`

