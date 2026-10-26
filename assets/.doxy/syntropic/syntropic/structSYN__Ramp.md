

# Struct SYN\_Ramp



[**ClassList**](annotated.md) **>** [**SYN\_Ramp**](structSYN__Ramp.md)



_Ramp generator instance — current value, target, rate._ 

* `#include <syn_ramp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int32\_t | [**accel**](#variable-accel)  <br> |
|  int32\_t | [**current**](#variable-current)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**done**](#variable-done)  <br> |
|  int32\_t | [**frac\_accum**](#variable-frac_accum)  <br> |
|  uint8\_t | [**frac\_bits**](#variable-frac_bits)  <br> |
|  uint8\_t | [**mode**](#variable-mode)  <br> |
|  int32\_t | [**rate**](#variable-rate)  <br> |
|  int32\_t | [**target**](#variable-target)  <br> |
|  int32\_t | [**velocity**](#variable-velocity)  <br> |












































## Public Attributes Documentation




### variable accel 

```C++
int32_t SYN_Ramp::accel;
```



Acceleration per tick (in Q`frac_bits`) 
 


        

<hr>



### variable current 

```C++
int32_t SYN_Ramp::current;
```



Current output value 
 


        

<hr>



### variable done 

```C++
bool SYN_Ramp::done;
```



true when current == target 
 


        

<hr>



### variable frac\_accum 

```C++
int32_t SYN_Ramp::frac_accum;
```



Fractional position accumulator 
 


        

<hr>



### variable frac\_bits 

```C++
uint8_t SYN_Ramp::frac_bits;
```



Fixed-point bits (0 = integer, 8 = Q8) 
 


        

<hr>



### variable mode 

```C++
uint8_t SYN_Ramp::mode;
```



SYN\_RampMode 
 


        

<hr>



### variable rate 

```C++
int32_t SYN_Ramp::rate;
```



Max velocity per update (in Q`frac_bits`) 
 


        

<hr>



### variable target 

```C++
int32_t SYN_Ramp::target;
```



Desired final value 
 


        

<hr>



### variable velocity 

```C++
int32_t SYN_Ramp::velocity;
```



Current velocity (in Q`frac_bits`) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_ramp.h`

