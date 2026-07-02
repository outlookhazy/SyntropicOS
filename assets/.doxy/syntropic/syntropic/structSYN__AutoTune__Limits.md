

# Struct SYN\_AutoTune\_Limits



[**ClassList**](annotated.md) **>** [**SYN\_AutoTune\_Limits**](structSYN__AutoTune__Limits.md)



_Physical constraints for the one-call auto-tune._ 

* `#include <syn_autotune.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int32\_t | [**max\_velocity**](#variable-max_velocity)  <br> |
|  int32\_t | [**position\_max**](#variable-position_max)  <br> |
|  int32\_t | [**position\_min**](#variable-position_min)  <br> |
|  uint32\_t | [**watchdog\_ms**](#variable-watchdog_ms)  <br> |












































## Public Attributes Documentation




### variable max\_velocity 

```C++
int32_t SYN_AutoTune_Limits::max_velocity;
```



Max safe velocity (counts/sec). 0=none. 


        

<hr>



### variable position\_max 

```C++
int32_t SYN_AutoTune_Limits::position_max;
```



Max safe position (encoder counts). 
 


        

<hr>



### variable position\_min 

```C++
int32_t SYN_AutoTune_Limits::position_min;
```



Min safe position (encoder counts). 
 


        

<hr>



### variable watchdog\_ms 

```C++
uint32_t SYN_AutoTune_Limits::watchdog_ms;
```



Abort if update() gap exceeds this. 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_autotune.h`

