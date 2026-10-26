

# Struct SYN\_Hysteresis



[**ClassList**](annotated.md) **>** [**SYN\_Hysteresis**](structSYN__Hysteresis.md)



_Hysteresis comparator with configurable dead-band._ 

* `#include <syn_hysteresis.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int32\_t | [**band**](#variable-band)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**state**](#variable-state)  <br> |
|  int32\_t | [**threshold**](#variable-threshold)  <br> |












































## Public Attributes Documentation




### variable band 

```C++
int32_t SYN_Hysteresis::band;
```



Half-width of the deadband 
 


        

<hr>



### variable state 

```C++
bool SYN_Hysteresis::state;
```



Current output state (high/low) 
 


        

<hr>



### variable threshold 

```C++
int32_t SYN_Hysteresis::threshold;
```



Center threshold value 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_hysteresis.h`

