

# Struct Poly1305\_Ctx



[**ClassList**](annotated.md) **>** [**Poly1305\_Ctx**](structPoly1305__Ctx.md)



_Poly1305 context._ 






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**h**](#variable-h)  <br> |
|  uint32\_t | [**pad**](#variable-pad)  <br> |
|  uint32\_t | [**r**](#variable-r)  <br> |












































## Public Attributes Documentation




### variable h 

```C++
uint32_t Poly1305_Ctx::h[5];
```



Accumulator (26-bit limbs) 
 


        

<hr>



### variable pad 

```C++
uint32_t Poly1305_Ctx::pad[4];
```



Key s = second 16 bytes 
 


        

<hr>



### variable r 

```C++
uint32_t Poly1305_Ctx::r[5];
```



Clamped key r (26-bit limbs) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_chacha20poly1305.c`

