

# Struct SYN\_AutoTune\_Result



[**ClassList**](annotated.md) **>** [**SYN\_AutoTune\_Result**](structSYN__AutoTune__Result.md)



_Auto-tune results (valid when state == DONE)._ 

* `#include <syn_autotune.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int32\_t | [**Ku**](#variable-ku)  <br> |
|  uint32\_t | [**Tu\_ms**](#variable-tu_ms)  <br> |
|  int32\_t | [**ff\_ka**](#variable-ff_ka)  <br> |
|  int32\_t | [**ff\_kv**](#variable-ff_kv)  <br> |
|  uint8\_t | [**ff\_scale**](#variable-ff_scale)  <br> |
|  int32\_t | [**kd**](#variable-kd)  <br> |
|  int32\_t | [**ki**](#variable-ki)  <br> |
|  int32\_t | [**kp**](#variable-kp)  <br> |
|  uint8\_t | [**pid\_scale**](#variable-pid_scale)  <br> |
|  int32\_t | [**steady\_velocity\_1**](#variable-steady_velocity_1)  <br> |
|  int32\_t | [**steady\_velocity\_2**](#variable-steady_velocity_2)  <br> |












































## Public Attributes Documentation




### variable Ku 

```C++
int32_t SYN_AutoTune_Result::Ku;
```



Ultimate gain (scaled by pid\_scale) 
 


        

<hr>



### variable Tu\_ms 

```C++
uint32_t SYN_AutoTune_Result::Tu_ms;
```



Ultimate period (ms) 
 


        

<hr>



### variable ff\_ka 

```C++
int32_t SYN_AutoTune_Result::ff_ka;
```



Computed inertia feedforward gain 
 


        

<hr>



### variable ff\_kv 

```C++
int32_t SYN_AutoTune_Result::ff_kv;
```



Computed velocity feedforward gain 
 


        

<hr>



### variable ff\_scale 

```C++
uint8_t SYN_AutoTune_Result::ff_scale;
```



Feedforward scale (same as ctrl cfg) 
 


        

<hr>



### variable kd 

```C++
int32_t SYN_AutoTune_Result::kd;
```



Computed derivative gain 
 


        

<hr>



### variable ki 

```C++
int32_t SYN_AutoTune_Result::ki;
```



Computed integral gain 
 


        

<hr>



### variable kp 

```C++
int32_t SYN_AutoTune_Result::kp;
```



Computed proportional gain 
 


        

<hr>



### variable pid\_scale 

```C++
uint8_t SYN_AutoTune_Result::pid_scale;
```



PID gain scale 
 


        

<hr>



### variable steady\_velocity\_1 

```C++
int32_t SYN_AutoTune_Result::steady_velocity_1;
```



Measured velocity at point 1 
 


        

<hr>



### variable steady\_velocity\_2 

```C++
int32_t SYN_AutoTune_Result::steady_velocity_2;
```



Measured velocity at point 2 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_autotune.h`

