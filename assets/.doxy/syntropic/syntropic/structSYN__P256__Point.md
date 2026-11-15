

# Struct SYN\_P256\_Point



[**ClassList**](annotated.md) **>** [**SYN\_P256\_Point**](structSYN__P256__Point.md)



_Jacobian projective point used inside resumable P-256 contexts._ 

* `#include <syn_p256.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**infinity**](#variable-infinity)  <br> |
|  uint32\_t | [**x**](#variable-x)  <br> |
|  uint32\_t | [**y**](#variable-y)  <br> |
|  uint32\_t | [**z**](#variable-z)  <br> |












































## Public Attributes Documentation




### variable infinity 

```C++
bool SYN_P256_Point::infinity;
```



True if point is at infinity 


        

<hr>



### variable x 

```C++
uint32_t SYN_P256_Point::x[8];
```



Projective coordinate X 


        

<hr>



### variable y 

```C++
uint32_t SYN_P256_Point::y[8];
```



Projective coordinate Y 


        

<hr>



### variable z 

```C++
uint32_t SYN_P256_Point::z[8];
```



Projective coordinate Z 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_p256.h`

