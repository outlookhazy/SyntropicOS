

# Struct SYN\_PID\_Config



[**ClassList**](annotated.md) **>** [**SYN\_PID\_Config**](structSYN__PID__Config.md)



_PID controller configuration._ 

* `#include <syn_pid.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**d\_filter\_alpha**](#variable-d_filter_alpha)  <br> |
|  int32\_t | [**integral\_max**](#variable-integral_max)  <br> |
|  int32\_t | [**kd**](#variable-kd)  <br> |
|  int32\_t | [**ki**](#variable-ki)  <br> |
|  int32\_t | [**kp**](#variable-kp)  <br> |
|  int32\_t | [**out\_max**](#variable-out_max)  <br> |
|  int32\_t | [**out\_min**](#variable-out_min)  <br> |
|  int32\_t | [**scale**](#variable-scale)  <br> |












































## Public Attributes Documentation




### variable d\_filter\_alpha 

```C++
uint8_t SYN_PID_Config::d_filter_alpha;
```



EMA alpha for D-term (0=off, 255=no filter) 


        

<hr>



### variable integral\_max 

```C++
int32_t SYN_PID_Config::integral_max;
```



Max integral accumulator (0 = auto) 
 


        

<hr>



### variable kd 

```C++
int32_t SYN_PID_Config::kd;
```



Derivative gain (÷ scale) 
 


        

<hr>



### variable ki 

```C++
int32_t SYN_PID_Config::ki;
```



Integral gain (÷ scale × 1000) 
 


        

<hr>



### variable kp 

```C++
int32_t SYN_PID_Config::kp;
```



Proportional gain (÷ scale) 
 


        

<hr>



### variable out\_max 

```C++
int32_t SYN_PID_Config::out_max;
```



Maximum output value 
 


        

<hr>



### variable out\_min 

```C++
int32_t SYN_PID_Config::out_min;
```



Minimum output value 
 


        

<hr>



### variable scale 

```C++
int32_t SYN_PID_Config::scale;
```



Gain divisor (e.g., 256 for 8-bit) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_pid.h`

