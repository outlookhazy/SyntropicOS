

# Struct SYN\_Kalman\_Config



[**ClassList**](annotated.md) **>** [**SYN\_Kalman\_Config**](structSYN__Kalman__Config.md)



_Kalman filter configuration — all matrices are caller-owned._ [More...](#detailed-description)

* `#include <syn_kalman.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_Matrix**](structSYN__Matrix.md) \* | [**F**](#variable-f)  <br> |
|  [**SYN\_Matrix**](structSYN__Matrix.md) \* | [**H**](#variable-h)  <br> |
|  [**SYN\_Matrix**](structSYN__Matrix.md) \* | [**P**](#variable-p)  <br> |
|  [**SYN\_Matrix**](structSYN__Matrix.md) \* | [**Q**](#variable-q)  <br> |
|  [**SYN\_Matrix**](structSYN__Matrix.md) \* | [**R**](#variable-r)  <br> |
|  uint8\_t | [**n\_meas**](#variable-n_meas)  <br> |
|  uint8\_t | [**n\_state**](#variable-n_state)  <br> |
|  [**SYN\_Matrix**](structSYN__Matrix.md) \* | [**x**](#variable-x)  <br> |












































## Detailed Description


The caller allocates all matrices via SYN\_MAT\_DECL and passes pointers in this config struct. The Kalman filter modifies x and P in-place during predict/update. 


    
## Public Attributes Documentation




### variable F 

```C++
SYN_Matrix* SYN_Kalman_Config::F;
```



State transition model (n\_state × n\_state) 
 


        

<hr>



### variable H 

```C++
SYN_Matrix* SYN_Kalman_Config::H;
```



Measurement model (n\_meas × n\_state) 
 


        

<hr>



### variable P 

```C++
SYN_Matrix* SYN_Kalman_Config::P;
```



Error covariance (n\_state × n\_state) 
 


        

<hr>



### variable Q 

```C++
SYN_Matrix* SYN_Kalman_Config::Q;
```



Process noise covariance (n\_state × n\_state) 


        

<hr>



### variable R 

```C++
SYN_Matrix* SYN_Kalman_Config::R;
```



Measurement noise covariance (n\_meas × n\_meas) 


        

<hr>



### variable n\_meas 

```C++
uint8_t SYN_Kalman_Config::n_meas;
```



Measurement dimension 
 


        

<hr>



### variable n\_state 

```C++
uint8_t SYN_Kalman_Config::n_state;
```



State dimension 
 


        

<hr>



### variable x 

```C++
SYN_Matrix* SYN_Kalman_Config::x;
```



State vector (n\_state × 1) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_kalman.h`

