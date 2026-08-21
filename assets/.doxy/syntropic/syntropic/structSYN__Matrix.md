

# Struct SYN\_Matrix



[**ClassList**](annotated.md) **>** [**SYN\_Matrix**](structSYN__Matrix.md)



_Fixed-point matrix descriptor (non-owning view)._ [More...](#detailed-description)

* `#include <syn_matrix.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**cols**](#variable-cols)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* | [**data**](#variable-data)  <br> |
|  uint8\_t | [**rows**](#variable-rows)  <br> |












































## Detailed Description


Points to caller-owned flat row-major Q16.16 storage. Dimensions are typically compile-time constants, enabling the compiler to constant-fold loop bounds and unroll. 


    
## Public Attributes Documentation




### variable cols 

```C++
uint8_t SYN_Matrix::cols;
```



Number of columns 
 


        

<hr>



### variable data 

```C++
q16_t* SYN_Matrix::data;
```



Flat row-major storage (caller-owned) 
 


        

<hr>



### variable rows 

```C++
uint8_t SYN_Matrix::rows;
```



Number of rows 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_matrix.h`

