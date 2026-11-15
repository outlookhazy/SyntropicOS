

# Struct SYN\_RCCurve\_Config



[**ClassList**](annotated.md) **>** [**SYN\_RCCurve\_Config**](structSYN__RCCurve__Config.md)



[More...](#detailed-description)

* `#include <syn_rc_curve.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**deadband\_us**](#variable-deadband_us)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**dual\_rate**](#variable-dual_rate)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**expo**](#variable-expo)  <br> |












































## Detailed Description


RC Curve Configuration. 


    
## Public Attributes Documentation




### variable deadband\_us 

```C++
uint16_t SYN_RCCurve_Config::deadband_us;
```



Center deadband width in us (e.g., 10 us -&gt; [1490, 1510] deadband). 


        

<hr>



### variable dual\_rate 

```C++
q16_t SYN_RCCurve_Config::dual_rate;
```



Dual rate scaling factor in Q16.16 (1.0 = 100% full travel). 


        

<hr>



### variable expo 

```C++
q16_t SYN_RCCurve_Config::expo;
```



Exponential factor in Q16.16 (0.0 = linear, 1.0 = full cubic). 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_rc_curve.h`

