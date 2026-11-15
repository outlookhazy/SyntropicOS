

# Struct SYN\_Ed25519\_Ctx



[**ClassList**](annotated.md) **>** [**SYN\_Ed25519\_Ctx**](structSYN__Ed25519__Ctx.md)



_Resumable Ed25519 working context (caller-owned, zero-heap)._ 

* `#include <syn_ed25519.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  int32\_t | [**aux**](#variable-aux)  <br> |
|  uint8\_t | [**d**](#variable-d)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_pk**](#variable-has_pk)  <br> |
|  uint16\_t | [**idx**](#variable-idx)  <br> |
|  const uint8\_t \* | [**msg**](#variable-msg)  <br> |
|  size\_t | [**msg\_len**](#variable-msg_len)  <br> |
|  size\_t | [**msg\_off**](#variable-msg_off)  <br> |
|  uint8\_t | [**op**](#variable-op)  <br> |
|  int32\_t | [**p**](#variable-p)  <br> |
|  uint8\_t | [**pk**](#variable-pk)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**prehashed**](#variable-prehashed)  <br> |
|  int32\_t | [**q**](#variable-q)  <br> |
|  uint8\_t | [**r\_pt**](#variable-r_pt)  <br> |
|  uint8\_t | [**scalar**](#variable-scalar)  <br> |
|  [**SYN\_SHA512**](structSYN__SHA512.md) | [**sha**](#variable-sha)  <br> |
|  [**SYN\_PT**](structSYN__PT.md) | [**sub2\_pt**](#variable-sub2_pt)  <br> |
|  [**SYN\_PT**](structSYN__PT.md) | [**sub\_pt**](#variable-sub_pt)  <br> |
|  union [**SYN\_Ed25519\_Ctx**](structSYN__Ed25519__Ctx.md) | [**u**](#variable-u)  <br> |












































## Public Attributes Documentation




### variable active 

```C++
bool SYN_Ed25519_Ctx::active;
```



True while initialized and awaiting completion 


        

<hr>



### variable aux 

```C++
int32_t SYN_Ed25519_Ctx::aux[4][16];
```



Inversion/pow2523 temporaries or Shamir sum point 


        

<hr>



### variable d 

```C++
uint8_t SYN_Ed25519_Ctx::d[64];
```



Expanded secret scalar / prefix or reduced challenge h 


        

<hr>



### variable has\_pk 

```C++
bool SYN_Ed25519_Ctx::has_pk;
```



True when sign context received caller-supplied public key 


        

<hr>



### variable idx 

```C++
uint16_t SYN_Ed25519_Ctx::idx;
```



Step index within active child protothread 


        

<hr>



### variable msg 

```C++
const uint8_t* SYN_Ed25519_Ctx::msg;
```



Caller message buffer (valid until sign/verify completes) 


        

<hr>



### variable msg\_len 

```C++
size_t SYN_Ed25519_Ctx::msg_len;
```



Message length in bytes 


        

<hr>



### variable msg\_off 

```C++
size_t SYN_Ed25519_Ctx::msg_off;
```



Bytes hashed so far in active SHA-512 phase 


        

<hr>



### variable op 

```C++
uint8_t SYN_Ed25519_Ctx::op;
```



Active operation kind 


        

<hr>



### variable p 

```C++
int32_t SYN_Ed25519_Ctx::p[4][16];
```



Extended Edwards25519 point accumulator 


        

<hr>



### variable pk 

```C++
uint8_t SYN_Ed25519_Ctx::pk[32];
```



Compressed 32-byte public key / scratch packed point 


        

<hr>



### variable prehashed 

```C++
bool SYN_Ed25519_Ctx::prehashed;
```



True when verify context initialized from precomputed H(R\|\|A\|\|M) 


        

<hr>



### variable q 

```C++
int32_t SYN_Ed25519_Ctx::q[4][16];
```



Extended Edwards25519 base / unpacked peer point 


        

<hr>



### variable r\_pt 

```C++
uint8_t SYN_Ed25519_Ctx::r_pt[32];
```



Compressed 32-byte signature R point 


        

<hr>



### variable scalar 

```C++
uint8_t SYN_Ed25519_Ctx::scalar[32];
```



Reduced nonce r (sign) or signature scalar S (verify) 


        

<hr>



### variable sha 

```C++
SYN_SHA512 SYN_Ed25519_Ctx::sha;
```



Streaming SHA-512 context for nonce/challenge digests 


        

<hr>



### variable sub2\_pt 

```C++
SYN_PT SYN_Ed25519_Ctx::sub2_pt;
```



Nested child protothread continuation 


        

<hr>



### variable sub\_pt 

```C++
SYN_PT SYN_Ed25519_Ctx::sub_pt;
```



First-level child protothread continuation 


        

<hr>



### variable u 

```C++
union SYN_Ed25519_Ctx SYN_Ed25519_Ctx::u;
```



Phase-disjoint scratch storage 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_ed25519.h`

