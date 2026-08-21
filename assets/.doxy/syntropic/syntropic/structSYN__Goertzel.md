

# Struct SYN\_Goertzel



[**ClassList**](annotated.md) **>** [**SYN\_Goertzel**](structSYN__Goertzel.md)



_Goertzel Filter Instance State._ 

* `#include <syn_goertzel.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int32\_t | [**coeff\_q15**](#variable-coeff_q15)  <br> |
|  size\_t | [**n\_count**](#variable-n_count)  <br> |
|  size\_t | [**n\_total**](#variable-n_total)  <br> |
|  int32\_t | [**s1**](#variable-s1)  <br> |
|  int32\_t | [**s2**](#variable-s2)  <br> |












































## Public Attributes Documentation




### variable coeff\_q15 

```C++
int32_t SYN_Goertzel::coeff_q15;
```



Precomputed 2 \* cos(2 \* pi \* k / N) in Q15 format 


        

<hr>



### variable n\_count 

```C++
size_t SYN_Goertzel::n_count;
```



Accumulated sample count in current block 
 


        

<hr>



### variable n\_total 

```C++
size_t SYN_Goertzel::n_total;
```



Target block size N 
 


        

<hr>



### variable s1 

```C++
int32_t SYN_Goertzel::s1;
```



Filter delay element s[n - 1] 
 


        

<hr>



### variable s2 

```C++
int32_t SYN_Goertzel::s2;
```



Filter delay element s[n - 2] 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_goertzel.h`

