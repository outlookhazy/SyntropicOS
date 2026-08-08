

# Struct SYN\_Kalman



[**ClassList**](annotated.md) **>** [**SYN\_Kalman**](structSYN__Kalman.md)



_Kalman filter instance._ [More...](#detailed-description)

* `#include <syn_kalman.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const [**SYN\_Kalman\_Config**](structSYN__Kalman__Config.md) \* | [**cfg**](#variable-cfg)  <br> |
|  [**SYN\_Matrix**](structSYN__Matrix.md) \* | [**temp\_m1**](#variable-temp_m1)  <br> |
|  [**SYN\_Matrix**](structSYN__Matrix.md) \* | [**temp\_mm**](#variable-temp_mm)  <br> |
|  [**SYN\_Matrix**](structSYN__Matrix.md) \* | [**temp\_mm\_2**](#variable-temp_mm_2)  <br> |
|  [**SYN\_Matrix**](structSYN__Matrix.md) \* | [**temp\_mn**](#variable-temp_mn)  <br> |
|  [**SYN\_Matrix**](structSYN__Matrix.md) \* | [**temp\_n1**](#variable-temp_n1)  <br> |
|  [**SYN\_Matrix**](structSYN__Matrix.md) \* | [**temp\_nm**](#variable-temp_nm)  <br> |
|  [**SYN\_Matrix**](structSYN__Matrix.md) \* | [**temp\_nn\_1**](#variable-temp_nn_1)  <br> |
|  [**SYN\_Matrix**](structSYN__Matrix.md) \* | [**temp\_nn\_2**](#variable-temp_nn_2)  <br> |












































## Detailed Description


Holds internal scratch matrices for intermediate calculations. The caller must allocate the scratch space and pass it to syn\_kalman\_init via the work buffer. 


    
## Public Attributes Documentation




### variable cfg 

```C++
const SYN_Kalman_Config* SYN_Kalman::cfg;
```



Configuration (caller-owned) 
 


        

<hr>



### variable temp\_m1 

```C++
SYN_Matrix* SYN_Kalman::temp_m1;
```



Scratch n\_meas × 1 (innovation) 
 


        

<hr>



### variable temp\_mm 

```C++
SYN_Matrix* SYN_Kalman::temp_mm;
```



Scratch n\_meas × n\_meas 
 


        

<hr>



### variable temp\_mm\_2 

```C++
SYN_Matrix* SYN_Kalman::temp_mm_2;
```



Scratch n\_meas × n\_meas (for inverse) 
 


        

<hr>



### variable temp\_mn 

```C++
SYN_Matrix* SYN_Kalman::temp_mn;
```



Scratch n\_meas × n\_state 
 


        

<hr>



### variable temp\_n1 

```C++
SYN_Matrix* SYN_Kalman::temp_n1;
```



Scratch n\_state × 1 
 


        

<hr>



### variable temp\_nm 

```C++
SYN_Matrix* SYN_Kalman::temp_nm;
```



Scratch n\_state × n\_meas 
 


        

<hr>



### variable temp\_nn\_1 

```C++
SYN_Matrix* SYN_Kalman::temp_nn_1;
```



Scratch n\_state × n\_state 
 


        

<hr>



### variable temp\_nn\_2 

```C++
SYN_Matrix* SYN_Kalman::temp_nn_2;
```



Scratch n\_state × n\_state 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_kalman.h`

