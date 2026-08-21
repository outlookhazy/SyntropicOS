

# Struct SYN\_SCurve



[**ClassList**](annotated.md) **>** [**SYN\_SCurve**](structSYN__SCurve.md)



_S-curve trajectory generator state._ 

* `#include <syn_scurve.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int32\_t | [**a**](#variable-a)  <br> |
|  int32\_t | [**a\_max**](#variable-a_max)  <br> |
|  int32\_t | [**current\_phase**](#variable-current_phase)  <br> |
|  int32\_t | [**dir**](#variable-dir)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**done**](#variable-done)  <br> |
|  int32\_t | [**j**](#variable-j)  <br> |
|  int32\_t | [**j\_max**](#variable-j_max)  <br> |
|  int32\_t | [**p**](#variable-p)  <br> |
|  int32\_t | [**phase\_ticks**](#variable-phase_ticks)  <br> |
|  int32\_t | [**target\_p**](#variable-target_p)  <br> |
|  int32\_t | [**ticks\_in\_phase**](#variable-ticks_in_phase)  <br> |
|  int32\_t | [**v**](#variable-v)  <br> |
|  int32\_t | [**v\_max**](#variable-v_max)  <br> |












































## Public Attributes Documentation




### variable a 

```C++
int32_t SYN_SCurve::a;
```



Current acceleration 


        

<hr>



### variable a\_max 

```C++
int32_t SYN_SCurve::a_max;
```



Maximum acceleration magnitude 


        

<hr>



### variable current\_phase 

```C++
int32_t SYN_SCurve::current_phase;
```



Current phase (0-6), 7 = done 


        

<hr>



### variable dir 

```C++
int32_t SYN_SCurve::dir;
```



Direction of motion (1 or -1) 


        

<hr>



### variable done 

```C++
bool SYN_SCurve::done;
```



True if target reached 


        

<hr>



### variable j 

```C++
int32_t SYN_SCurve::j;
```



Current jerk 


        

<hr>



### variable j\_max 

```C++
int32_t SYN_SCurve::j_max;
```



Maximum jerk magnitude 


        

<hr>



### variable p 

```C++
int32_t SYN_SCurve::p;
```



Current position 


        

<hr>



### variable phase\_ticks 

```C++
int32_t SYN_SCurve::phase_ticks[7];
```



Ticks to spend in each of the 7 phases 


        

<hr>



### variable target\_p 

```C++
int32_t SYN_SCurve::target_p;
```



Target position 


        

<hr>



### variable ticks\_in\_phase 

```C++
int32_t SYN_SCurve::ticks_in_phase;
```



Ticks spent in current phase 


        

<hr>



### variable v 

```C++
int32_t SYN_SCurve::v;
```



Current velocity 


        

<hr>



### variable v\_max 

```C++
int32_t SYN_SCurve::v_max;
```



Maximum velocity magnitude 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_scurve.h`

