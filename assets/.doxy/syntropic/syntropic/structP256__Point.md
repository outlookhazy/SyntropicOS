

# Struct P256\_Point



[**ClassList**](annotated.md) **>** [**P256\_Point**](structP256__Point.md)



_Point representation in Jacobian projective coordinates (X, Y, Z)._ 






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**infinity**](#variable-infinity)  <br> |
|  [**bignum256**](syn__p256_8c.md#typedef-bignum256) | [**x**](#variable-x)  <br> |
|  [**bignum256**](syn__p256_8c.md#typedef-bignum256) | [**y**](#variable-y)  <br> |
|  [**bignum256**](syn__p256_8c.md#typedef-bignum256) | [**z**](#variable-z)  <br> |












































## Public Attributes Documentation




### variable infinity 

```C++
bool P256_Point::infinity;
```



True if point is at infinity 


        

<hr>



### variable x 

```C++
bignum256 P256_Point::x;
```



Projective coordinate X 


        

<hr>



### variable y 

```C++
bignum256 P256_Point::y;
```



Projective coordinate Y 


        

<hr>



### variable z 

```C++
bignum256 P256_Point::z;
```



Projective coordinate Z 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_p256.c`

