

# File syn\_sha256.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_sha256.c**](syn__sha256_8c.md)

[Go to the source code of this file](syn__sha256_8c_source.md)

_SHA-256 implementation — FIPS 180-4 compliant, pure C99._ [More...](#detailed-description)

* `#include "syn_sha256.h"`
* `#include "syn_assert.h"`
* `#include <string.h>`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const uint32\_t | [**H\_INIT**](#variable-h_init)   = `/* multi line expression */`<br> |
|  const uint32\_t | [**K**](#variable-k)   = `/* multi line expression */`<br> |














## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_sha256\_final**](#function-syn_sha256_final) ([**SYN\_SHA256**](structSYN__SHA256.md) \* ctx, uint8\_t hash) <br>_Finalize the hash and produce the 32-byte digest._  |
|  void | [**syn\_sha256\_init**](#function-syn_sha256_init) ([**SYN\_SHA256**](structSYN__SHA256.md) \* ctx) <br>_Initialize a SHA-256 context._  |
|  void | [**syn\_sha256\_update**](#function-syn_sha256_update) ([**SYN\_SHA256**](structSYN__SHA256.md) \* ctx, const void \* data, size\_t len) <br>_Feed data into the hash._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**ch**](#function-ch) (uint32\_t x, uint32\_t y, uint32\_t z) <br>_Choice function: Ch(x,y,z) = (x ∧ y) ⊕ (¬x ∧ z)._  |
|  uint32\_t | [**gamma0**](#function-gamma0) (uint32\_t x) <br>_Small sigma-0: σ₀(x) = ROTR⁷(x) ⊕ ROTR¹⁸(x) ⊕ SHR³(x)._  |
|  uint32\_t | [**gamma1**](#function-gamma1) (uint32\_t x) <br>_Small sigma-1: σ₁(x) = ROTR¹⁷(x) ⊕ ROTR¹⁹(x) ⊕ SHR¹⁰(x)._  |
|  uint32\_t | [**maj**](#function-maj) (uint32\_t x, uint32\_t y, uint32\_t z) <br>_Majority function: Maj(x,y,z) = (x ∧ y) ⊕ (x ∧ z) ⊕ (y ∧ z)._  |
|  uint32\_t | [**rotr**](#function-rotr) (uint32\_t x, unsigned n) <br>_Circular right rotate._  |
|  void | [**sha256\_transform**](#function-sha256_transform) (uint32\_t state, const uint8\_t block) <br>_Process a single 64-byte block._  |
|  uint32\_t | [**sigma0**](#function-sigma0) (uint32\_t x) <br>_Big Sigma-0: Σ₀(x) = ROTR²(x) ⊕ ROTR¹³(x) ⊕ ROTR²²(x)._  |
|  uint32\_t | [**sigma1**](#function-sigma1) (uint32\_t x) <br>_Big Sigma-1: Σ₁(x) = ROTR⁶(x) ⊕ ROTR¹¹(x) ⊕ ROTR²⁵(x)._  |


























## Detailed Description


No heap allocation, no floating point, no external dependencies. Static K[64] round constants table consumes ~256 bytes of ROM. 


    
## Public Static Attributes Documentation




### variable H\_INIT 

```C++
const uint32_t H_INIT[8];
```



FIPS 180-4 initial hash values (H0–H7). 


        

<hr>



### variable K 

```C++
const uint32_t K[64];
```



FIPS 180-4 round constants (K0–K63). 


        

<hr>
## Public Functions Documentation




### function syn\_sha256\_final 

_Finalize the hash and produce the 32-byte digest._ 
```C++
void syn_sha256_final (
    SYN_SHA256 * ctx,
    uint8_t hash
) 
```



After calling this, the context must be re-initialized before reuse.




**Parameters:**


* `ctx` SHA-256 context. 
* `hash` Output buffer (must be at least 32 bytes). 




        

<hr>



### function syn\_sha256\_init 

_Initialize a SHA-256 context._ 
```C++
void syn_sha256_init (
    SYN_SHA256 * ctx
) 
```





**Parameters:**


* `ctx` Context to initialize. 




        

<hr>



### function syn\_sha256\_update 

_Feed data into the hash._ 
```C++
void syn_sha256_update (
    SYN_SHA256 * ctx,
    const void * data,
    size_t len
) 
```



Can be called repeatedly with arbitrary chunk sizes.




**Parameters:**


* `ctx` SHA-256 context. 
* `data` Data to hash. 
* `len` Length in bytes. 




        

<hr>
## Public Static Functions Documentation




### function ch 

_Choice function: Ch(x,y,z) = (x ∧ y) ⊕ (¬x ∧ z)._ 
```C++
static inline uint32_t ch (
    uint32_t x,
    uint32_t y,
    uint32_t z
) 
```





**Parameters:**


* `x` First input. 
* `y` Second input. 
* `z` Third input. 



**Returns:**

Ch(x, y, z). 





        

<hr>



### function gamma0 

_Small sigma-0: σ₀(x) = ROTR⁷(x) ⊕ ROTR¹⁸(x) ⊕ SHR³(x)._ 
```C++
static inline uint32_t gamma0 (
    uint32_t x
) 
```





**Parameters:**


* `x` Input word. 



**Returns:**

σ₀(x). 





        

<hr>



### function gamma1 

_Small sigma-1: σ₁(x) = ROTR¹⁷(x) ⊕ ROTR¹⁹(x) ⊕ SHR¹⁰(x)._ 
```C++
static inline uint32_t gamma1 (
    uint32_t x
) 
```





**Parameters:**


* `x` Input word. 



**Returns:**

σ₁(x). 





        

<hr>



### function maj 

_Majority function: Maj(x,y,z) = (x ∧ y) ⊕ (x ∧ z) ⊕ (y ∧ z)._ 
```C++
static inline uint32_t maj (
    uint32_t x,
    uint32_t y,
    uint32_t z
) 
```





**Parameters:**


* `x` First input. 
* `y` Second input. 
* `z` Third input. 



**Returns:**

Maj(x, y, z). 





        

<hr>



### function rotr 

_Circular right rotate._ 
```C++
static inline uint32_t rotr (
    uint32_t x,
    unsigned n
) 
```





**Parameters:**


* `x` Value to rotate. 
* `n` Number of bits to rotate. 



**Returns:**

Rotated value. 





        

<hr>



### function sha256\_transform 

_Process a single 64-byte block._ 
```C++
static void sha256_transform (
    uint32_t state,
    const uint8_t block
) 
```





**Parameters:**


* `state` Running hash state (8 × uint32\_t). 
* `block` 64-byte input block. 




        

<hr>



### function sigma0 

_Big Sigma-0: Σ₀(x) = ROTR²(x) ⊕ ROTR¹³(x) ⊕ ROTR²²(x)._ 
```C++
static inline uint32_t sigma0 (
    uint32_t x
) 
```





**Parameters:**


* `x` Input word. 



**Returns:**

Σ₀(x). 





        

<hr>



### function sigma1 

_Big Sigma-1: Σ₁(x) = ROTR⁶(x) ⊕ ROTR¹¹(x) ⊕ ROTR²⁵(x)._ 
```C++
static inline uint32_t sigma1 (
    uint32_t x
) 
```





**Parameters:**


* `x` Input word. 



**Returns:**

Σ₁(x). 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_sha256.c`

