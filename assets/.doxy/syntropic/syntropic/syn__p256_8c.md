

# File syn\_p256.c



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_p256.c**](syn__p256_8c.md)

[Go to the source code of this file](syn__p256_8c_source.md)

_NIST P-256 (secp256r1 / prime256v1) Elliptic Curve Cryptography implementation._ 

* `#include "syn_p256.h"`
* `#include "syn_hmac_drbg.h"`
* `#include <string.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**P256\_Point**](structP256__Point.md) <br>_Point representation in Jacobian projective coordinates (X, Y, Z)._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef uint32\_t | [**bignum256**](#typedef-bignum256)  <br>_256-bit unsigned big integer representation in eight 32-bit limbs._  |






## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const uint32\_t | [**BN\_ONE**](#variable-bn_one)   = `{1U, 0U, 0U, 0U, 0U, 0U, 0U, 0U}`<br>_Big-number 1._  |
|  const uint32\_t | [**P256\_B**](#variable-p256_b)   = `/* multi line expression */`<br>_Curve coefficient b._  |
|  const uint32\_t | [**P256\_GX**](#variable-p256_gx)   = `/* multi line expression */`<br>_Base Generator G x-coordinate._  |
|  const uint32\_t | [**P256\_GY**](#variable-p256_gy)   = `/* multi line expression */`<br>_Base Generator G y-coordinate._  |
|  const uint32\_t | [**P256\_N**](#variable-p256_n)   = `/* multi line expression */`<br>_Group order n._  |
|  const uint32\_t | [**P256\_P**](#variable-p256_p)   = `/* multi line expression */`<br>_Prime p = 2^256 - 2^224 + 2^192 + 2^96 - 1._  |














## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_p256\_base\_mul**](#function-syn_p256_base_mul) (const uint8\_t scalar, uint8\_t pub\_x, uint8\_t pub\_y) <br>_Multiply the P-256 base generator G by a 32-byte scalar._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_p256\_ecdh**](#function-syn_p256_ecdh) (const uint8\_t priv\_key, const uint8\_t peer\_pub\_x, const uint8\_t peer\_pub\_y, uint8\_t shared\_secret) <br>_Perform ECDH Key Agreement._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_p256\_is\_on\_curve**](#function-syn_p256_is_on_curve) (const uint8\_t px, const uint8\_t py) <br>_Validate if a point (x, y) lies on the NIST P-256 curve: y^2 = x^3 - 3x + b (mod p)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_p256\_point\_mul**](#function-syn_p256_point_mul) (const uint8\_t scalar, const uint8\_t px, const uint8\_t py, uint8\_t rx, uint8\_t ry) <br>_Multiply an arbitrary P-256 point by a 32-byte scalar._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_p256\_sign\_ecdsa**](#function-syn_p256_sign_ecdsa) (const uint8\_t priv\_key, const uint8\_t nonce\_k, const uint8\_t hash, uint8\_t r\_out, uint8\_t s\_out) <br>_Generate a NIST P-256 ECDSA signature (FIPS 186-4)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_p256\_sign\_ecdsa\_deterministic**](#function-syn_p256_sign_ecdsa_deterministic) (const uint8\_t priv\_key, const uint8\_t hash, uint8\_t r\_out, uint8\_t s\_out) <br>_Generate a deterministic NIST P-256 ECDSA signature (RFC 6979 / FIPS 186-4)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_p256\_verify\_ecdsa**](#function-syn_p256_verify_ecdsa) (const uint8\_t hash, const uint8\_t r, const uint8\_t s, const uint8\_t pub\_x, const uint8\_t pub\_y) <br>_Verify a NIST P-256 ECDSA signature (FIPS 186-4)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**bn\_add\_raw**](#function-bn_add_raw) ([**bignum256**](syn__p256_8c.md#typedef-bignum256) r, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) a, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) b) <br>_Raw 256-bit addition with carry out._  |
|  int | [**bn\_cmp**](#function-bn_cmp) (const [**bignum256**](syn__p256_8c.md#typedef-bignum256) a, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) b) <br>_Compare two 256-bit big numbers._  |
|  void | [**bn\_copy**](#function-bn_copy) ([**bignum256**](syn__p256_8c.md#typedef-bignum256) r, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) a) <br>_Copy bignum256._  |
|  void | [**bn\_div2\_mod\_n**](#function-bn_div2_mod_n) ([**bignum256**](syn__p256_8c.md#typedef-bignum256) r, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) a) <br>_Halving modulo n: computes (a / 2) mod n._  |
|  void | [**bn\_from\_bytes**](#function-bn_from_bytes) ([**bignum256**](syn__p256_8c.md#typedef-bignum256) r, const uint8\_t bytes) <br>_Load bignum256 from 32 big-endian bytes._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**bn\_is\_zero**](#function-bn_is_zero) (const [**bignum256**](syn__p256_8c.md#typedef-bignum256) a) <br>_Check if bignum256 is zero._  |
|  void | [**bn\_mul\_raw**](#function-bn_mul_raw) (uint32\_t t, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) a, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) b) <br>_256x256-bit raw integer multiplication producing a 512-bit product._  |
|  void | [**bn\_rshift1**](#function-bn_rshift1) ([**bignum256**](syn__p256_8c.md#typedef-bignum256) r, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) a) <br>_Shift 256-bit bignum right by 1 bit._  |
|  uint32\_t | [**bn\_sub\_raw**](#function-bn_sub_raw) ([**bignum256**](syn__p256_8c.md#typedef-bignum256) r, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) a, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) b) <br>_Raw 256-bit subtraction with borrow out._  |
|  void | [**bn\_to\_bytes**](#function-bn_to_bytes) (uint8\_t bytes, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) a) <br>_Export bignum256 to 32 big-endian bytes._  |
|  void | [**p256\_mod\_n\_inv**](#function-p256_mod_n_inv) ([**bignum256**](syn__p256_8c.md#typedef-bignum256) r, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) a) <br>_Group order inversion modulo n via Binary Extended Euclidean Algorithm: a^(-1) mod n._  |
|  void | [**p256\_mod\_n\_mul**](#function-p256_mod_n_mul) ([**bignum256**](syn__p256_8c.md#typedef-bignum256) r, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) a, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) b) <br>_Group order multiplication modulo n._  |
|  void | [**p256\_mod\_n\_sub**](#function-p256_mod_n_sub) ([**bignum256**](syn__p256_8c.md#typedef-bignum256) r, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) a, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) b) <br>_Subtraction modulo n._  |
|  void | [**p256\_mod\_p\_add**](#function-p256_mod_p_add) ([**bignum256**](syn__p256_8c.md#typedef-bignum256) r, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) a, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) b) <br>_Field addition modulo p._  |
|  void | [**p256\_mod\_p\_inv**](#function-p256_mod_p_inv) ([**bignum256**](syn__p256_8c.md#typedef-bignum256) r, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) a) <br>_Field inversion modulo p via Fermat's Little Theorem: a^(p-2) mod p._  |
|  void | [**p256\_mod\_p\_mul**](#function-p256_mod_p_mul) ([**bignum256**](syn__p256_8c.md#typedef-bignum256) r, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) a, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) b) <br>_Field multiplication modulo p._  |
|  void | [**p256\_mod\_p\_reduce**](#function-p256_mod_p_reduce) ([**bignum256**](syn__p256_8c.md#typedef-bignum256) r, const uint32\_t c) <br>_512-bit integer fast reduction modulo p using Solinas algorithm for NIST P-256._  |
|  void | [**p256\_mod\_p\_sqr**](#function-p256_mod_p_sqr) ([**bignum256**](syn__p256_8c.md#typedef-bignum256) r, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) a) <br>_Field squaring modulo p._  |
|  void | [**p256\_mod\_p\_sub**](#function-p256_mod_p_sub) ([**bignum256**](syn__p256_8c.md#typedef-bignum256) r, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) a, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) b) <br>_Field subtraction modulo p._  |
|  void | [**point\_add**](#function-point_add) ([**P256\_Point**](structP256__Point.md) \* r, const [**P256\_Point**](structP256__Point.md) \* p, const [**P256\_Point**](structP256__Point.md) \* q) <br>_Point Addition in Jacobian coordinates: P + Q._  |
|  void | [**point\_add\_mixed**](#function-point_add_mixed) ([**P256\_Point**](structP256__Point.md) \* r, const [**P256\_Point**](structP256__Point.md) \* p, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) qx, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) qy) <br>_Mixed Jacobian + Affine Addition: P (Jacobian) + Q (Affine)._  |
|  void | [**point\_cmov**](#function-point_cmov) ([**P256\_Point**](structP256__Point.md) \* dst, const [**P256\_Point**](structP256__Point.md) \* src, uint32\_t mask) <br>_Constant-time conditional point move: dst = (mask != 0) ? src : dst._  |
|  void | [**point\_double**](#function-point_double) ([**P256\_Point**](structP256__Point.md) \* r, const [**P256\_Point**](structP256__Point.md) \* p) <br>_Point Doubling in Jacobian coordinates: 2P._  |
|  void | [**point\_scalar\_mul\_affine**](#function-point_scalar_mul_affine) ([**P256\_Point**](structP256__Point.md) \* r, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) k, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) px, const [**bignum256**](syn__p256_8c.md#typedef-bignum256) py) <br>_Constant-time Windowed Scalar Multiplication: k \* P (where P is affine (px, py))._  |
|  void | [**point\_set\_infinity**](#function-point_set_infinity) ([**P256\_Point**](structP256__Point.md) \* p) <br>_Set Jacobian point to infinity._  |
|  void | [**point\_to\_affine**](#function-point_to_affine) (const [**P256\_Point**](structP256__Point.md) \* p, [**bignum256**](syn__p256_8c.md#typedef-bignum256) x\_out, [**bignum256**](syn__p256_8c.md#typedef-bignum256) y\_out) <br>_Convert Jacobian coordinates (X, Y, Z) to Affine coordinates (x, y)._  |


























## Public Types Documentation




### typedef bignum256 

_256-bit unsigned big integer representation in eight 32-bit limbs._ 
```C++
typedef uint32_t bignum256[8];
```




<hr>
## Public Static Attributes Documentation




### variable BN\_ONE 

_Big-number 1._ 
```C++
const uint32_t BN_ONE[8];
```




<hr>



### variable P256\_B 

_Curve coefficient b._ 
```C++
const uint32_t P256_B[8];
```




<hr>



### variable P256\_GX 

_Base Generator G x-coordinate._ 
```C++
const uint32_t P256_GX[8];
```




<hr>



### variable P256\_GY 

_Base Generator G y-coordinate._ 
```C++
const uint32_t P256_GY[8];
```




<hr>



### variable P256\_N 

_Group order n._ 
```C++
const uint32_t P256_N[8];
```




<hr>



### variable P256\_P 

_Prime p = 2^256 - 2^224 + 2^192 + 2^96 - 1._ 
```C++
const uint32_t P256_P[8];
```




<hr>
## Public Functions Documentation




### function syn\_p256\_base\_mul 

_Multiply the P-256 base generator G by a 32-byte scalar._ 
```C++
bool syn_p256_base_mul (
    const uint8_t scalar,
    uint8_t pub_x,
    uint8_t pub_y
) 
```



Computes Q = scalar \* G.




**Parameters:**


* `scalar` 32-byte scalar in big-endian format. 
* `pub_x` [out] 32-byte X-coordinate of resulting point. 
* `pub_y` [out] 32-byte Y-coordinate of resulting point. 



**Returns:**

true on success, false if scalar is 0 or &gt;= curve order. 





        

<hr>



### function syn\_p256\_ecdh 

_Perform ECDH Key Agreement._ 
```C++
bool syn_p256_ecdh (
    const uint8_t priv_key,
    const uint8_t peer_pub_x,
    const uint8_t peer_pub_y,
    uint8_t shared_secret
) 
```



Computes shared\_secret = priv\_key \* peer\_pub\_point.




**Parameters:**


* `priv_key` 32-byte private key scalar. 
* `peer_pub_x` 32-byte X-coordinate of peer's public key. 
* `peer_pub_y` 32-byte Y-coordinate of peer's public key. 
* `shared_secret` [out] 32-byte shared secret (X-coordinate of product point). 



**Returns:**

true on success, false on invalid point or scalar. 





        

<hr>



### function syn\_p256\_is\_on\_curve 

_Validate if a point (x, y) lies on the NIST P-256 curve: y^2 = x^3 - 3x + b (mod p)._ 
```C++
bool syn_p256_is_on_curve (
    const uint8_t px,
    const uint8_t py
) 
```





**Parameters:**


* `px` 32-byte X-coordinate. 
* `py` 32-byte Y-coordinate. 



**Returns:**

true if point is on curve, false otherwise. 





        

<hr>



### function syn\_p256\_point\_mul 

_Multiply an arbitrary P-256 point by a 32-byte scalar._ 
```C++
bool syn_p256_point_mul (
    const uint8_t scalar,
    const uint8_t px,
    const uint8_t py,
    uint8_t rx,
    uint8_t ry
) 
```



Computes R = scalar \* P.




**Parameters:**


* `scalar` 32-byte scalar in big-endian format. 
* `px` 32-byte X-coordinate of input point P. 
* `py` 32-byte Y-coordinate of input point P. 
* `rx` [out] 32-byte X-coordinate of resulting point R. 
* `ry` [out] 32-byte Y-coordinate of resulting point R. 



**Returns:**

true on success, false if point is invalid or scalar out of range. 





        

<hr>



### function syn\_p256\_sign\_ecdsa 

_Generate a NIST P-256 ECDSA signature (FIPS 186-4)._ 
```C++
bool syn_p256_sign_ecdsa (
    const uint8_t priv_key,
    const uint8_t nonce_k,
    const uint8_t hash,
    uint8_t r_out,
    uint8_t s_out
) 
```



Computes signature (r, s) for a given message hash using a private key and nonce k.




**Parameters:**


* `priv_key` 32-byte private key scalar. 
* `nonce_k` 32-byte ephemeral private nonce k (must be in [1, n-1]). 
* `hash` 32-byte message hash (typically SHA-256). 
* `r_out` [out] 32-byte signature component r. 
* `s_out` [out] 32-byte signature component s. 



**Returns:**

true on success, false on invalid parameters. 





        

<hr>



### function syn\_p256\_sign\_ecdsa\_deterministic 

_Generate a deterministic NIST P-256 ECDSA signature (RFC 6979 / FIPS 186-4)._ 
```C++
bool syn_p256_sign_ecdsa_deterministic (
    const uint8_t priv_key,
    const uint8_t hash,
    uint8_t r_out,
    uint8_t s_out
) 
```



Derives the ephemeral nonce k deterministically using NIST SP 800-90A HMAC-DRBG (SHA-256) keyed with the private key and message hash.




**Parameters:**


* `priv_key` 32-byte private key scalar. 
* `hash` 32-byte message hash (typically SHA-256). 
* `r_out` [out] 32-byte signature component r. 
* `s_out` [out] 32-byte signature component s. 



**Returns:**

true on success, false on invalid parameters. 





        

<hr>



### function syn\_p256\_verify\_ecdsa 

_Verify a NIST P-256 ECDSA signature (FIPS 186-4)._ 
```C++
bool syn_p256_verify_ecdsa (
    const uint8_t hash,
    const uint8_t r,
    const uint8_t s,
    const uint8_t pub_x,
    const uint8_t pub_y
) 
```





**Parameters:**


* `hash` 32-byte message hash (typically SHA-256). 
* `r` 32-byte signature component r. 
* `s` 32-byte signature component s. 
* `pub_x` 32-byte public key X-coordinate. 
* `pub_y` 32-byte public key Y-coordinate. 



**Returns:**

true if signature is mathematically valid, false otherwise. 





        

<hr>
## Public Static Functions Documentation




### function bn\_add\_raw 

_Raw 256-bit addition with carry out._ 
```C++
static uint32_t bn_add_raw (
    bignum256 r,
    const bignum256 a,
    const bignum256 b
) 
```





**Parameters:**


* `r` Sum result. 
* `a` First term. 
* `b` Second term. 



**Returns:**

Carry out bit (0 or 1). 





        

<hr>



### function bn\_cmp 

_Compare two 256-bit big numbers._ 
```C++
static int bn_cmp (
    const bignum256 a,
    const bignum256 b
) 
```





**Parameters:**


* `a` First big number. 
* `b` Second big number. 



**Returns:**

1 if a &gt; b, -1 if a &lt; b, 0 if equal. 





        

<hr>



### function bn\_copy 

_Copy bignum256._ 
```C++
static void bn_copy (
    bignum256 r,
    const bignum256 a
) 
```





**Parameters:**


* `r` Destination. 
* `a` Source. 




        

<hr>



### function bn\_div2\_mod\_n 

_Halving modulo n: computes (a / 2) mod n._ 
```C++
static void bn_div2_mod_n (
    bignum256 r,
    const bignum256 a
) 
```





**Parameters:**


* `r` Result. 
* `a` Input bignum. 




        

<hr>



### function bn\_from\_bytes 

_Load bignum256 from 32 big-endian bytes._ 
```C++
static void bn_from_bytes (
    bignum256 r,
    const uint8_t bytes
) 
```





**Parameters:**


* `r` Output big-number. 
* `bytes` 32-byte big-endian input array. 




        

<hr>



### function bn\_is\_zero 

_Check if bignum256 is zero._ 
```C++
static bool bn_is_zero (
    const bignum256 a
) 
```





**Parameters:**


* `a` Input big number. 



**Returns:**

True if zero, false otherwise. 





        

<hr>



### function bn\_mul\_raw 

_256x256-bit raw integer multiplication producing a 512-bit product._ 
```C++
static void bn_mul_raw (
    uint32_t t,
    const bignum256 a,
    const bignum256 b
) 
```





**Parameters:**


* `t` 512-bit product (16 limbs). 
* `a` First 256-bit factor. 
* `b` Second 256-bit factor. 




        

<hr>



### function bn\_rshift1 

_Shift 256-bit bignum right by 1 bit._ 
```C++
static void bn_rshift1 (
    bignum256 r,
    const bignum256 a
) 
```





**Parameters:**


* `r` Result. 
* `a` Input bignum. 




        

<hr>



### function bn\_sub\_raw 

_Raw 256-bit subtraction with borrow out._ 
```C++
static uint32_t bn_sub_raw (
    bignum256 r,
    const bignum256 a,
    const bignum256 b
) 
```





**Parameters:**


* `r` Difference result. 
* `a` Minuend. 
* `b` Subtrahend. 



**Returns:**

Borrow out bit (0 or 1). 





        

<hr>



### function bn\_to\_bytes 

_Export bignum256 to 32 big-endian bytes._ 
```C++
static void bn_to_bytes (
    uint8_t bytes,
    const bignum256 a
) 
```





**Parameters:**


* `bytes` 32-byte big-endian output array. 
* `a` Input big-number. 




        

<hr>



### function p256\_mod\_n\_inv 

_Group order inversion modulo n via Binary Extended Euclidean Algorithm: a^(-1) mod n._ 
```C++
static void p256_mod_n_inv (
    bignum256 r,
    const bignum256 a
) 
```





**Parameters:**


* `r` Inverted value. 
* `a` Base to invert. 




        

<hr>



### function p256\_mod\_n\_mul 

_Group order multiplication modulo n._ 
```C++
static void p256_mod_n_mul (
    bignum256 r,
    const bignum256 a,
    const bignum256 b
) 
```





**Parameters:**


* `r` Product modulo n. 
* `a` First factor. 
* `b` Second factor. 




        

<hr>



### function p256\_mod\_n\_sub 

_Subtraction modulo n._ 
```C++
static void p256_mod_n_sub (
    bignum256 r,
    const bignum256 a,
    const bignum256 b
) 
```





**Parameters:**


* `r` Difference mod n. 
* `a` Minuend. 
* `b` Subtrahend. 




        

<hr>



### function p256\_mod\_p\_add 

_Field addition modulo p._ 
```C++
static void p256_mod_p_add (
    bignum256 r,
    const bignum256 a,
    const bignum256 b
) 
```





**Parameters:**


* `r` Sum modulo p. 
* `a` First operand. 
* `b` Second operand. 




        

<hr>



### function p256\_mod\_p\_inv 

_Field inversion modulo p via Fermat's Little Theorem: a^(p-2) mod p._ 
```C++
static void p256_mod_p_inv (
    bignum256 r,
    const bignum256 a
) 
```





**Parameters:**


* `r` Inverted value. 
* `a` Base to invert. 




        

<hr>



### function p256\_mod\_p\_mul 

_Field multiplication modulo p._ 
```C++
static void p256_mod_p_mul (
    bignum256 r,
    const bignum256 a,
    const bignum256 b
) 
```





**Parameters:**


* `r` Product modulo p. 
* `a` First factor. 
* `b` Second factor. 




        

<hr>



### function p256\_mod\_p\_reduce 

_512-bit integer fast reduction modulo p using Solinas algorithm for NIST P-256._ 
```C++
static void p256_mod_p_reduce (
    bignum256 r,
    const uint32_t c
) 
```





**Parameters:**


* `r` Reduced 256-bit value modulo p. 
* `c` 512-bit integer in 16 32-bit limbs. 




        

<hr>



### function p256\_mod\_p\_sqr 

_Field squaring modulo p._ 
```C++
static void p256_mod_p_sqr (
    bignum256 r,
    const bignum256 a
) 
```





**Parameters:**


* `r` Result a^2 mod p. 
* `a` Base. 




        

<hr>



### function p256\_mod\_p\_sub 

_Field subtraction modulo p._ 
```C++
static void p256_mod_p_sub (
    bignum256 r,
    const bignum256 a,
    const bignum256 b
) 
```





**Parameters:**


* `r` Difference modulo p. 
* `a` Minuend. 
* `b` Subtrahend. 




        

<hr>



### function point\_add 

_Point Addition in Jacobian coordinates: P + Q._ 
```C++
static void point_add (
    P256_Point * r,
    const P256_Point * p,
    const P256_Point * q
) 
```





**Parameters:**


* `r` Output point P + Q. 
* `p` First Jacobian point. 
* `q` Second Jacobian point. 




        

<hr>



### function point\_add\_mixed 

_Mixed Jacobian + Affine Addition: P (Jacobian) + Q (Affine)._ 
```C++
static void point_add_mixed (
    P256_Point * r,
    const P256_Point * p,
    const bignum256 qx,
    const bignum256 qy
) 
```





**Parameters:**


* `r` Output point P + Q. 
* `p` Input Jacobian point. 
* `qx` Input Affine point X coordinate. 
* `qy` Input Affine point Y coordinate. 




        

<hr>



### function point\_cmov 

_Constant-time conditional point move: dst = (mask != 0) ? src : dst._ 
```C++
static void point_cmov (
    P256_Point * dst,
    const P256_Point * src,
    uint32_t mask
) 
```





**Parameters:**


* `dst` Destination point. 
* `src` Source point. 
* `mask` Bitmask (0x00000000 or 0xFFFFFFFF). 




        

<hr>



### function point\_double 

_Point Doubling in Jacobian coordinates: 2P._ 
```C++
static void point_double (
    P256_Point * r,
    const P256_Point * p
) 
```





**Parameters:**


* `r` Output point 2P. 
* `p` Input point P. 




        

<hr>



### function point\_scalar\_mul\_affine 

_Constant-time Windowed Scalar Multiplication: k \* P (where P is affine (px, py))._ 
```C++
static void point_scalar_mul_affine (
    P256_Point * r,
    const bignum256 k,
    const bignum256 px,
    const bignum256 py
) 
```





**Parameters:**


* `r` Output Jacobian point. 
* `k` Scalar factor. 
* `px` Affine base point X coordinate. 
* `py` Affine base point Y coordinate. 




        

<hr>



### function point\_set\_infinity 

_Set Jacobian point to infinity._ 
```C++
static void point_set_infinity (
    P256_Point * p
) 
```





**Parameters:**


* `p` Point structure to initialize to infinity. 




        

<hr>



### function point\_to\_affine 

_Convert Jacobian coordinates (X, Y, Z) to Affine coordinates (x, y)._ 
```C++
static void point_to_affine (
    const P256_Point * p,
    bignum256 x_out,
    bignum256 y_out
) 
```





**Parameters:**


* `p` Input Jacobian point. 
* `x_out` Output affine X coordinate. 
* `y_out` Output affine Y coordinate. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_p256.c`

