

# Struct SYN\_FOCObserverConfig



[**ClassList**](annotated.md) **>** [**SYN\_FOCObserverConfig**](structSYN__FOCObserverConfig.md)



_Sliding Mode Observer configuration parameters._ 

* `#include <syn_foc_observer.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**G**](#variable-g)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**Ki\_pll**](#variable-ki_pll)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**Kp\_pll**](#variable-kp_pll)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**L**](#variable-l)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**R**](#variable-r)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**dt**](#variable-dt)  <br> |












































## Public Attributes Documentation




### variable G 

```C++
q16_t SYN_FOCObserverConfig::G;
```



Sliding mode observer gain (Q16.16) 


        

<hr>



### variable Ki\_pll 

```C++
q16_t SYN_FOCObserverConfig::Ki_pll;
```



Software PLL integral gain (Q16.16) 


        

<hr>



### variable Kp\_pll 

```C++
q16_t SYN_FOCObserverConfig::Kp_pll;
```



Software PLL proportional gain (Q16.16) 


        

<hr>



### variable L 

```C++
q16_t SYN_FOCObserverConfig::L;
```



Stator phase inductance in Henries (Q16.16) 


        

<hr>



### variable R 

```C++
q16_t SYN_FOCObserverConfig::R;
```



Stator phase resistance in Ohms (Q16.16) 


        

<hr>



### variable dt 

```C++
q16_t SYN_FOCObserverConfig::dt;
```



Control loop sample period in seconds (Q16.16) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_foc_observer.h`

