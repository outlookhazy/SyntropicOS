

# Struct SYN\_FilterEMA



[**ClassList**](annotated.md) **>** [**SYN\_FilterEMA**](structSYN__FilterEMA.md)



_Exponential moving average filter state._ 

* `#include <syn_filter.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**alpha**](#variable-alpha)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**primed**](#variable-primed)  <br> |
|  int32\_t | [**value**](#variable-value)  <br> |












































## Public Attributes Documentation




### variable alpha 

```C++
uint8_t SYN_FilterEMA::alpha;
```



Smoothing factor (0–255), higher = faster 
 


        

<hr>



### variable primed 

```C++
bool SYN_FilterEMA::primed;
```



Has received at least one sample 
 


        

<hr>



### variable value 

```C++
int32_t SYN_FilterEMA::value;
```



Current filtered value (Q8 fixed-point) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_filter.h`

