

# File syn\_p256.h



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_p256.h**](syn__p256_8h.md)

[Go to the source code of this file](syn__p256_8h_source.md)

_NIST P-256 (secp256r1 / prime256v1) Elliptic Curve Cryptography._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`





































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



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_P256\_BYTE\_LEN**](syn__p256_8h.md#define-syn_p256_byte_len)  `32U`<br>_NIST P-256 coordinate / scalar byte size (32 bytes)._  |

## Detailed Description


Provides zero-allocation, cleanroom NIST P-256 scalar multiplication, ECDH shared secret generation, and FIPS 186-4 ECDSA signature verification. 


    
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
## Macro Definition Documentation





### define SYN\_P256\_BYTE\_LEN 

_NIST P-256 coordinate / scalar byte size (32 bytes)._ 
```C++
#define SYN_P256_BYTE_LEN `32U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_p256.h`

