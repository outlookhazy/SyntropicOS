

# Struct SYN\_PID



[**ClassList**](annotated.md) **>** [**SYN\_PID**](structSYN__PID.md)



_PID controller instance — config + accumulated state._ 

* `#include <syn_pid.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_PID\_Config**](structSYN__PID__Config.md) | [**cfg**](#variable-cfg)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**first**](#variable-first)  <br> |
|  int32\_t | [**integral**](#variable-integral)  <br> |
|  int32\_t | [**output**](#variable-output)  <br> |
|  int32\_t | [**prev\_d\_filtered**](#variable-prev_d_filtered)  <br> |
|  int32\_t | [**prev\_error**](#variable-prev_error)  <br> |












































## Public Attributes Documentation




### variable cfg 

```C++
SYN_PID_Config SYN_PID::cfg;
```



Configuration snapshot 
 


        

<hr>



### variable first 

```C++
bool SYN_PID::first;
```



True before first update 
 


        

<hr>



### variable integral 

```C++
int32_t SYN_PID::integral;
```



Accumulated integral 
 


        

<hr>



### variable output 

```C++
int32_t SYN_PID::output;
```



Last computed output 
 


        

<hr>



### variable prev\_d\_filtered 

```C++
int32_t SYN_PID::prev_d_filtered;
```



Filtered derivative (Q0) 
 


        

<hr>



### variable prev\_error 

```C++
int32_t SYN_PID::prev_error;
```



Previous error for derivative 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_pid.h`

