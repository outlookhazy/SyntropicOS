

# Struct SYN\_X25519\_Ctx



[**ClassList**](annotated.md) **>** [**SYN\_X25519\_Ctx**](structSYN__X25519__Ctx.md)



_Resumable X25519 scalar multiplication working state (caller-owned, zero-heap)._ 

* `#include <syn_x25519.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int32\_t | [**a**](#variable-a)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  int32\_t | [**b**](#variable-b)  <br> |
|  int32\_t | [**c**](#variable-c)  <br> |
|  int32\_t | [**d**](#variable-d)  <br> |
|  uint16\_t | [**idx**](#variable-idx)  <br> |
|  [**SYN\_PT**](structSYN__PT.md) | [**sub\_pt**](#variable-sub_pt)  <br> |
|  int32\_t | [**x**](#variable-x)  <br> |
|  uint8\_t | [**z**](#variable-z)  <br> |












































## Public Attributes Documentation




### variable a 

```C++
int32_t SYN_X25519_Ctx::a[16];
```



Montgomery ladder X\_2 / final numerator 


        

<hr>



### variable active 

```C++
bool SYN_X25519_Ctx::active;
```



true while initialized and awaiting completion 


        

<hr>



### variable b 

```C++
int32_t SYN_X25519_Ctx::b[16];
```



Montgomery ladder X\_3 


        

<hr>



### variable c 

```C++
int32_t SYN_X25519_Ctx::c[16];
```



Montgomery ladder Z\_2 / field inversion accumulator 


        

<hr>



### variable d 

```C++
int32_t SYN_X25519_Ctx::d[16];
```



Montgomery ladder Z\_3 / field inversion base Z\_2 


        

<hr>



### variable idx 

```C++
uint16_t SYN_X25519_Ctx::idx;
```



Loop step index within active child protothread 


        

<hr>



### variable sub\_pt 

```C++
SYN_PT SYN_X25519_Ctx::sub_pt;
```



Child protothread continuation for ladder and inversion loops 


        

<hr>



### variable x 

```C++
int32_t SYN_X25519_Ctx::x[16];
```



Unpacked base point 


        

<hr>



### variable z 

```C++
uint8_t SYN_X25519_Ctx::z[32];
```



Clamped private scalar bytes 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_x25519.h`

