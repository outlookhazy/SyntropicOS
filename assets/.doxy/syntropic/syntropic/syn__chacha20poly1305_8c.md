

# File syn\_chacha20poly1305.c



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_chacha20poly1305.c**](syn__chacha20poly1305_8c.md)

[Go to the source code of this file](syn__chacha20poly1305_8c_source.md)

_ChaCha20-Poly1305 AEAD — RFC 8439._ [More...](#detailed-description)

* `#include "syn_chacha20poly1305.h"`
* `#include <string.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**Poly1305\_Ctx**](structPoly1305__Ctx.md) <br>_Poly1305 context._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_aead\_decrypt**](#function-syn_aead_decrypt) (const uint8\_t key, const uint8\_t nonce, const uint8\_t \* aad, size\_t aad\_len, const uint8\_t \* ciphertext, size\_t ct\_len, const uint8\_t tag, uint8\_t \* plaintext) <br>_Decrypt and verify (AEAD)._  |
|  void | [**syn\_aead\_encrypt**](#function-syn_aead_encrypt) (const uint8\_t key, const uint8\_t nonce, const uint8\_t \* aad, size\_t aad\_len, const uint8\_t \* plaintext, size\_t pt\_len, uint8\_t \* ciphertext, uint8\_t tag) <br>_Encrypt and authenticate (AEAD)._  |
|  void | [**syn\_chacha20\_block**](#function-syn_chacha20_block) (const uint8\_t key, const uint8\_t nonce, uint32\_t counter, uint8\_t out) <br>_Generate ChaCha20 keystream block (no XOR)._  |
|  void | [**syn\_chacha20\_xor**](#function-syn_chacha20_xor) (const uint8\_t key, const uint8\_t nonce, uint32\_t counter, const uint8\_t \* in, size\_t len, uint8\_t \* out) <br>_XOR data with ChaCha20 keystream._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**aead\_mac**](#function-aead_mac) (const uint8\_t poly\_key, const uint8\_t \* aad, size\_t aad\_len, const uint8\_t \* ct, size\_t ct\_len, uint8\_t tag) <br>_Compute Poly1305 MAC over (aad \|\| pad \|\| ct \|\| pad \|\| len(aad) \|\| len(ct))._  |
|  void | [**chacha20\_block\_core**](#function-chacha20_block_core) (uint32\_t out, const uint32\_t in) <br>_Compute one ChaCha20 block into_ `out` _(16 uint32\_t)._ |
|  void | [**chacha20\_init**](#function-chacha20_init) (uint32\_t state, const uint8\_t key, const uint8\_t nonce, uint32\_t counter) <br>_Set up the ChaCha20 initial state._  |
|  uint32\_t | [**load32\_le**](#function-load32_le) (const uint8\_t \* p) <br>_Load 32-bit little-endian word._  |
|  void | [**poly1305\_blocks**](#function-poly1305_blocks) ([**Poly1305\_Ctx**](structPoly1305__Ctx.md) \* ctx, const uint8\_t \* data, size\_t len, uint32\_t hibit) <br>_Absorb full 16-byte blocks into Poly1305 accumulator._  |
|  void | [**poly1305\_finish**](#function-poly1305_finish) ([**Poly1305\_Ctx**](structPoly1305__Ctx.md) \* ctx, uint8\_t mac) <br>_Finalise Poly1305 and write the 16-byte MAC to_ `mac` _._ |
|  void | [**poly1305\_init**](#function-poly1305_init) ([**Poly1305\_Ctx**](structPoly1305__Ctx.md) \* ctx, const uint8\_t key) <br>_Initialise Poly1305 from a 32-byte one-time key._  |
|  uint32\_t | [**rotl32**](#function-rotl32) (uint32\_t x, unsigned n) <br>_32-bit left rotation._  |
|  void | [**store32\_le**](#function-store32_le) (uint8\_t \* p, uint32\_t v) <br>_Store 32-bit little-endian word._  |
|  void | [**store64\_le**](#function-store64_le) (uint8\_t \* p, uint64\_t v) <br>_Store 64-bit little-endian word._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**QR**](syn__chacha20poly1305_8c.md#define-qr) (a, b, c, d) `/* multi line expression */`<br>_ChaCha20 Quarter Round._  |

## Detailed Description


ChaCha20: add-rotate-xor stream cipher (pure 32-bit integer ops). Poly1305: one-time MAC using 130-bit arithmetic with 5 × 26-bit limbs. AEAD: combined construction per RFC 8439 §2.8. 


    
## Public Functions Documentation




### function syn\_aead\_decrypt 

_Decrypt and verify (AEAD)._ 
```C++
bool syn_aead_decrypt (
    const uint8_t key,
    const uint8_t nonce,
    const uint8_t * aad,
    size_t aad_len,
    const uint8_t * ciphertext,
    size_t ct_len,
    const uint8_t tag,
    uint8_t * plaintext
) 
```





**Parameters:**


* `key` 256-bit key (32 bytes). 
* `nonce` 96-bit nonce (12 bytes). 
* `aad` Additional authenticated data (or NULL). 
* `aad_len` AAD length. 
* `ciphertext` Data to decrypt. 
* `ct_len` Ciphertext length. 
* `tag` Expected 128-bit authentication tag (16 bytes). 
* `plaintext` Output plaintext (same length as ciphertext). 



**Returns:**

true if tag is valid and decryption succeeded. 





        

<hr>



### function syn\_aead\_encrypt 

_Encrypt and authenticate (AEAD)._ 
```C++
void syn_aead_encrypt (
    const uint8_t key,
    const uint8_t nonce,
    const uint8_t * aad,
    size_t aad_len,
    const uint8_t * plaintext,
    size_t pt_len,
    uint8_t * ciphertext,
    uint8_t tag
) 
```





**Parameters:**


* `key` 256-bit key (32 bytes). 
* `nonce` 96-bit nonce (12 bytes). 
* `aad` Additional authenticated data (or NULL). 
* `aad_len` AAD length. 
* `plaintext` Data to encrypt. 
* `pt_len` Plaintext length. 
* `ciphertext` Output ciphertext (same length as plaintext). 
* `tag` Output 128-bit authentication tag (16 bytes). 




        

<hr>



### function syn\_chacha20\_block 

_Generate ChaCha20 keystream block (no XOR)._ 
```C++
void syn_chacha20_block (
    const uint8_t key,
    const uint8_t nonce,
    uint32_t counter,
    uint8_t out
) 
```



Produces exactly 64 bytes of keystream for the given counter value.




**Parameters:**


* `key` 256-bit key (32 bytes). 
* `nonce` 96-bit nonce (12 bytes). 
* `counter` Block counter. 
* `out` Output buffer (exactly 64 bytes). 




        

<hr>



### function syn\_chacha20\_xor 

_XOR data with ChaCha20 keystream._ 
```C++
void syn_chacha20_xor (
    const uint8_t key,
    const uint8_t nonce,
    uint32_t counter,
    const uint8_t * in,
    size_t len,
    uint8_t * out
) 
```



Encrypts (or decrypts — same operation) by XOR-ing with the ChaCha20 keystream starting at the given block counter.




**Parameters:**


* `key` 256-bit key (32 bytes). 
* `nonce` 96-bit nonce (12 bytes). 
* `counter` Initial block counter (usually 0 or 1). 
* `in` Input data. 
* `len` Data length in bytes. 
* `out` Output buffer (may alias in). 




        

<hr>
## Public Static Functions Documentation




### function aead\_mac 

_Compute Poly1305 MAC over (aad \|\| pad \|\| ct \|\| pad \|\| len(aad) \|\| len(ct))._ 
```C++
static void aead_mac (
    const uint8_t poly_key,
    const uint8_t * aad,
    size_t aad_len,
    const uint8_t * ct,
    size_t ct_len,
    uint8_t tag
) 
```





**Parameters:**


* `poly_key` One-time Poly1305 key (32 bytes). 
* `aad` Additional authenticated data. 
* `aad_len` Length of `aad`. 
* `ct` Ciphertext. 
* `ct_len` Length of `ct`. 
* `tag` Output 16-byte tag. 




        

<hr>



### function chacha20\_block\_core 

_Compute one ChaCha20 block into_ `out` _(16 uint32\_t)._
```C++
static void chacha20_block_core (
    uint32_t out,
    const uint32_t in
) 
```





**Parameters:**


* `out` Output buffer (16 words). 
* `in` Initial state (16 words). 




        

<hr>



### function chacha20\_init 

_Set up the ChaCha20 initial state._ 
```C++
static void chacha20_init (
    uint32_t state,
    const uint8_t key,
    const uint8_t nonce,
    uint32_t counter
) 
```





**Parameters:**


* `state` Initial state buffer (16 words). 
* `key` 256-bit key. 
* `nonce` 96-bit nonce. 
* `counter` Initial block counter. 




        

<hr>



### function load32\_le 

_Load 32-bit little-endian word._ 
```C++
static inline uint32_t load32_le (
    const uint8_t * p
) 
```





**Parameters:**


* `p` Source bytes. 



**Returns:**

32-bit value. 





        

<hr>



### function poly1305\_blocks 

_Absorb full 16-byte blocks into Poly1305 accumulator._ 
```C++
static void poly1305_blocks (
    Poly1305_Ctx * ctx,
    const uint8_t * data,
    size_t len,
    uint32_t hibit
) 
```





**Parameters:**


* `ctx` Poly1305 context. 
* `data` Input data. 
* `len` Length in bytes (must be multiple of 16). 
* `hibit` High bit to add (1&lt;&lt;24 for message, 0 for special uses). 




        

<hr>



### function poly1305\_finish 

_Finalise Poly1305 and write the 16-byte MAC to_ `mac` _._
```C++
static void poly1305_finish (
    Poly1305_Ctx * ctx,
    uint8_t mac
) 
```





**Parameters:**


* `ctx` Poly1305 context. 
* `mac` Output buffer (16 bytes). 




        

<hr>



### function poly1305\_init 

_Initialise Poly1305 from a 32-byte one-time key._ 
```C++
static void poly1305_init (
    Poly1305_Ctx * ctx,
    const uint8_t key
) 
```





**Parameters:**


* `ctx` Context to initialize. 
* `key` 32-byte key. 




        

<hr>



### function rotl32 

_32-bit left rotation._ 
```C++
static inline uint32_t rotl32 (
    uint32_t x,
    unsigned n
) 
```





**Parameters:**


* `x` Value to rotate. 
* `n` Bits to shift. 



**Returns:**

Rotated value. 





        

<hr>



### function store32\_le 

_Store 32-bit little-endian word._ 
```C++
static inline void store32_le (
    uint8_t * p,
    uint32_t v
) 
```





**Parameters:**


* `p` Destination bytes. 
* `v` Value to store. 




        

<hr>



### function store64\_le 

_Store 64-bit little-endian word._ 
```C++
static inline void store64_le (
    uint8_t * p,
    uint64_t v
) 
```





**Parameters:**


* `p` Destination bytes. 
* `v` Value to store. 




        

<hr>
## Macro Definition Documentation





### define QR 

_ChaCha20 Quarter Round._ 
```C++
#define QR (
    a,
    b,
    c,
    d
) `/* multi line expression */`
```





**Parameters:**


* `a` Indices into the state vector. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_chacha20poly1305.c`

