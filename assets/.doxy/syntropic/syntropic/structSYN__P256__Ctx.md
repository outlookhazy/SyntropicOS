

# Struct SYN\_P256\_Ctx



[**ClassList**](annotated.md) **>** [**SYN\_P256\_Ctx**](structSYN__P256__Ctx.md)



_Resumable NIST P-256 working context (caller-owned, zero-heap)._ 

* `#include <syn_p256.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_P256\_Point**](structSYN__P256__Point.md) | [**acc**](#variable-acc)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  uint8\_t | [**attempts**](#variable-attempts)  <br> |
|  uint32\_t | [**aux**](#variable-aux)  <br> |
|  [**SYN\_HMAC\_DRBG**](structSYN__HMAC__DRBG.md) | [**drbg**](#variable-drbg)  <br> |
|  uint16\_t | [**idx**](#variable-idx)  <br> |
|  uint32\_t | [**inv\_base**](#variable-inv_base)  <br> |
|  uint32\_t | [**inv\_res**](#variable-inv_res)  <br> |
|  uint32\_t | [**k1**](#variable-k1)  <br> |
|  uint32\_t | [**k2**](#variable-k2)  <br> |
|  uint8\_t | [**max\_attempts**](#variable-max_attempts)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ok**](#variable-ok)  <br> |
|  uint8\_t | [**op**](#variable-op)  <br> |
|  uint32\_t | [**px**](#variable-px)  <br> |
|  uint32\_t | [**py**](#variable-py)  <br> |
|  [**SYN\_PT**](structSYN__PT.md) | [**sub\_pt**](#variable-sub_pt)  <br> |
|  [**SYN\_P256\_Point**](structSYN__P256__Point.md) | [**table**](#variable-table)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**use\_drbg**](#variable-use_drbg)  <br> |












































## Public Attributes Documentation




### variable acc 

```C++
SYN_P256_Point SYN_P256_Ctx::acc;
```



Active Jacobian point accumulator 


        

<hr>



### variable active 

```C++
bool SYN_P256_Ctx::active;
```



True while initialized and awaiting completion 


        

<hr>



### variable attempts 

```C++
uint8_t SYN_P256_Ctx::attempts;
```



Sign nonce attempts executed 


        

<hr>



### variable aux 

```C++
uint32_t SYN_P256_Ctx::aux[8];
```



Reduced hash e (sign) or signature r (verify) 


        

<hr>



### variable drbg 

```C++
SYN_HMAC_DRBG SYN_P256_Ctx::drbg;
```



RFC 6979 deterministic nonce generator 


        

<hr>



### variable idx 

```C++
uint16_t SYN_P256_Ctx::idx;
```



Sub-step index within active child protothread 


        

<hr>



### variable inv\_base 

```C++
uint32_t SYN_P256_Ctx::inv_base[8];
```



Modular inversion base 


        

<hr>



### variable inv\_res 

```C++
uint32_t SYN_P256_Ctx::inv_res[8];
```



Modular inversion accumulator 


        

<hr>



### variable k1 

```C++
uint32_t SYN_P256_Ctx::k1[8];
```



Primary scalar (k, nonce\_k, or u1) 


        

<hr>



### variable k2 

```C++
uint32_t SYN_P256_Ctx::k2[8];
```



Secondary scalar (private key d or u2) 


        

<hr>



### variable max\_attempts 

```C++
uint8_t SYN_P256_Ctx::max_attempts;
```



Max sign nonce attempts allowed 


        

<hr>



### variable ok 

```C++
bool SYN_P256_Ctx::ok;
```



Latched signing outcome across context zeroization 


        

<hr>



### variable op 

```C++
uint8_t SYN_P256_Ctx::op;
```



Active operation kind 


        

<hr>



### variable px 

```C++
uint32_t SYN_P256_Ctx::px[8];
```



Base point X coordinate 


        

<hr>



### variable py 

```C++
uint32_t SYN_P256_Ctx::py[8];
```



Base point Y coordinate 


        

<hr>



### variable sub\_pt 

```C++
SYN_PT SYN_P256_Ctx::sub_pt;
```



Child protothread continuation for inner curve/inversion loops 


        

<hr>



### variable table 

```C++
SYN_P256_Point SYN_P256_Ctx::table[3];
```



Windowed / Shamir precomputed points 


        

<hr>



### variable use\_drbg 

```C++
bool SYN_P256_Ctx::use_drbg;
```



True when drawing k from RFC 6979 DRBG 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_p256.h`

