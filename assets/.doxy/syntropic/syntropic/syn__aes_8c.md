

# File syn\_aes.c



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_aes.c**](syn__aes_8c.md)

[Go to the source code of this file](syn__aes_8c_source.md)

_Unified AES block cipher & AEAD implementation (128/192/256-bit keys, ECB, CBC, CTR, GCM)._ 

* `#include "syn_aes.h"`
* `#include <string.h>`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const uint8\_t | [**rcon**](#variable-rcon)   = `{0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36}`<br> |
|  const uint8\_t | [**rsbox**](#variable-rsbox)   = `/* multi line expression */`<br> |
|  const uint8\_t | [**sbox**](#variable-sbox)   = `/* multi line expression */`<br> |














## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes\_cbc\_decrypt**](#function-syn_aes_cbc_decrypt) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, const uint8\_t iv, const uint8\_t \* in, size\_t in\_len, uint8\_t \* out, size\_t out\_capacity, size\_t \* out\_len) <br>_Decrypt data using AES-CBC with PKCS#7 unpadding._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes\_cbc\_encrypt**](#function-syn_aes_cbc_encrypt) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, const uint8\_t iv, const uint8\_t \* in, size\_t in\_len, uint8\_t \* out, size\_t out\_capacity, size\_t \* out\_len) <br>_Encrypt data using AES-CBC with PKCS#7 padding._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes\_ccm\_decrypt**](#function-syn_aes_ccm_decrypt) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, const uint8\_t \* nonce, size\_t nonce\_len, const uint8\_t \* aad, size\_t aad\_len, const uint8\_t \* in, size\_t in\_len, const uint8\_t \* tag, size\_t tag\_len, uint8\_t \* out) <br>_AES-CCM authenticated decryption and tag verification (NIST SP 800-38C / RFC 3610)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes\_ccm\_encrypt**](#function-syn_aes_ccm_encrypt) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, const uint8\_t \* nonce, size\_t nonce\_len, const uint8\_t \* aad, size\_t aad\_len, const uint8\_t \* in, size\_t in\_len, uint8\_t \* out, uint8\_t \* tag, size\_t tag\_len) <br>_AES-CCM authenticated encryption (NIST SP 800-38C / RFC 3610)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes\_ctr**](#function-syn_aes_ctr) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, const uint8\_t nonce, const uint8\_t \* in, size\_t len, uint8\_t \* out) <br>_Encrypt/decrypt arbitrary length data using AES-CTR stream mode (NIST SP 800-38A)._  |
|  void | [**syn\_aes\_decrypt\_block**](#function-syn_aes_decrypt_block) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, const uint8\_t in, uint8\_t out) <br>_Decrypt a single 16-byte block (ECB mode)._  |
|  void | [**syn\_aes\_encrypt\_block**](#function-syn_aes_encrypt_block) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, const uint8\_t in, uint8\_t out) <br>_Encrypt a single 16-byte block (ECB mode)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes\_gcm\_decrypt**](#function-syn_aes_gcm_decrypt) (const [**SYN\_AES\_GCM\_Context**](structSYN__AES__GCM__Context.md) \* ctx, const uint8\_t \* nonce, size\_t nonce\_len, const uint8\_t \* aad, size\_t aad\_len, const uint8\_t \* in, size\_t in\_len, uint8\_t \* out, const uint8\_t tag) <br>_AES-GCM authenticated decryption and tag verification (NIST SP 800-38D)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes\_gcm\_encrypt**](#function-syn_aes_gcm_encrypt) (const [**SYN\_AES\_GCM\_Context**](structSYN__AES__GCM__Context.md) \* ctx, const uint8\_t \* nonce, size\_t nonce\_len, const uint8\_t \* aad, size\_t aad\_len, const uint8\_t \* in, size\_t in\_len, uint8\_t \* out, uint8\_t tag) <br>_AES-GCM authenticated encryption (NIST SP 800-38D)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes\_gcm\_init**](#function-syn_aes_gcm_init) ([**SYN\_AES\_GCM\_Context**](structSYN__AES__GCM__Context.md) \* ctx, const uint8\_t \* key, size\_t key\_len) <br>_Initialize AES-GCM AEAD context (expands key and computes GHASH subkey H)._  |
|  void | [**syn\_aes\_ghash\_mult**](#function-syn_aes_ghash_mult) (const uint8\_t x, const uint8\_t h, uint8\_t out) <br>_Multiply 16-byte block by GHASH subkey H in GF(2^128)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes\_init**](#function-syn_aes_init) ([**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, const uint8\_t \* key, size\_t key\_len) <br>_Initialize AES context and perform key expansion for 128/192/256-bit key._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**ccm\_compute\_mac**](#function-ccm_compute_mac) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, const uint8\_t \* nonce, size\_t nonce\_len, size\_t L, const uint8\_t \* aad, size\_t aad\_len, const uint8\_t \* data, size\_t data\_len, size\_t tag\_len, uint8\_t mac) <br>_Compute NIST SP 800-38C / RFC 3610 CBC-MAC tag over header and payload._  |
|  void | [**ccm\_format\_ctr**](#function-ccm_format_ctr) (uint8\_t a, const uint8\_t \* nonce, size\_t nonce\_len, size\_t L, uint64\_t counter) <br>_Format counter block Ai for AES-CCM CTR mode._  |
|  void | [**ccm\_mac\_feed**](#function-ccm_mac_feed) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, uint8\_t mac, uint8\_t \* blk, size\_t \* blk\_len, const uint8\_t \* data, size\_t len) <br>_Feed data into AES-CCM CBC-MAC accumulator._  |
|  void | [**ccm\_mac\_pad\_zero**](#function-ccm_mac_pad_zero) (const [**SYN\_AES\_Context**](structSYN__AES__Context.md) \* ctx, uint8\_t mac, uint8\_t \* blk, size\_t \* blk\_len) <br>_Pad partial block with zeros and finish block encryption in CBC-MAC._  |
|  void | [**gcm\_compute\_j0**](#function-gcm_compute_j0) (const [**SYN\_AES\_GCM\_Context**](structSYN__AES__GCM__Context.md) \* ctx, const uint8\_t \* nonce, size\_t nonce\_len, uint8\_t j0) <br>_Compute initial J0 counter block for GCM._  |
|  void | [**ghash\_mult\_bit**](#function-ghash_mult_bit) (const uint8\_t x, const uint8\_t y, uint8\_t out) <br>_Bitwise GHASH field multiplication in GF(2^128)._  |
|  void | [**ghash\_process\_blocks**](#function-ghash_process_blocks) (const [**SYN\_AES\_GCM\_Context**](structSYN__AES__GCM__Context.md) \* ctx, const uint8\_t \* data, size\_t len, uint8\_t y) <br>_Process data blocks through GHASH._  |
|  uint8\_t | [**gmult**](#function-gmult) (uint8\_t a, uint8\_t b) <br>_General multiplication in GF(2^8)._  |
|  uint8\_t | [**gmult2**](#function-gmult2) (uint8\_t a) <br>_Multiply byte by 2 in GF(2^8) modulo x^8 + x^4 + x^3 + x + 1._  |
|  void | [**inc128**](#function-inc128) (uint8\_t counter) <br>_Increment 128-bit big-endian counter._  |
|  void | [**inc32**](#function-inc32) (uint8\_t block) <br>_Increment rightmost 32 bits of 128-bit counter._  |
|  void | [**put\_be64**](#function-put_be64) (uint8\_t out, uint64\_t val) <br>_Write 64-bit value big-endian._  |


























## Public Static Attributes Documentation




### variable rcon 

```C++
const uint8_t rcon[11];
```



Round constants 


        

<hr>



### variable rsbox 

```C++
const uint8_t rsbox[256];
```



Inverse Substitution Box (RS-Box) 


        

<hr>



### variable sbox 

```C++
const uint8_t sbox[256];
```



Forward Substitution Box (S-Box) 


        

<hr>
## Public Functions Documentation




### function syn\_aes\_cbc\_decrypt 

_Decrypt data using AES-CBC with PKCS#7 unpadding._ 
```C++
SYN_Status syn_aes_cbc_decrypt (
    const SYN_AES_Context * ctx,
    const uint8_t iv,
    const uint8_t * in,
    size_t in_len,
    uint8_t * out,
    size_t out_capacity,
    size_t * out_len
) 
```





**Parameters:**


* `ctx` Initialized AES context. 
* `iv` 16-byte initialization vector. 
* `in` Ciphertext buffer (must be non-empty multiple of 16 bytes). 
* `in_len` Ciphertext length in bytes. 
* `out` Plaintext output buffer. 
* `out_capacity` Capacity of plaintext output buffer. 
* `out_len` Number of plaintext bytes written. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM on invalid padding or parameters. 





        

<hr>



### function syn\_aes\_cbc\_encrypt 

_Encrypt data using AES-CBC with PKCS#7 padding._ 
```C++
SYN_Status syn_aes_cbc_encrypt (
    const SYN_AES_Context * ctx,
    const uint8_t iv,
    const uint8_t * in,
    size_t in_len,
    uint8_t * out,
    size_t out_capacity,
    size_t * out_len
) 
```





**Parameters:**


* `ctx` Initialized AES context. 
* `iv` 16-byte initialization vector. 
* `in` Plaintext buffer (may be NULL if in\_len is 0). 
* `in_len` Plaintext length in bytes. 
* `out` Ciphertext output buffer. 
* `out_capacity` Maximum capacity of output buffer (must be &gt;= in\_len + PKCS#7 pad). 
* `out_len` Number of ciphertext bytes written. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM on failure. 





        

<hr>



### function syn\_aes\_ccm\_decrypt 

_AES-CCM authenticated decryption and tag verification (NIST SP 800-38C / RFC 3610)._ 
```C++
SYN_Status syn_aes_ccm_decrypt (
    const SYN_AES_Context * ctx,
    const uint8_t * nonce,
    size_t nonce_len,
    const uint8_t * aad,
    size_t aad_len,
    const uint8_t * in,
    size_t in_len,
    const uint8_t * tag,
    size_t tag_len,
    uint8_t * out
) 
```



Decrypts ciphertext in CTR mode and verifies CBC-MAC authentication tag in constant time. If verification fails, plaintext buffer is zeroed (if out != in) and SYN\_ERROR is returned.




**Parameters:**


* `ctx` Initialized AES context. 
* `nonce` Nonce buffer (length must be 7..13 bytes). 
* `nonce_len` Nonce length in bytes (7 to 13). 
* `aad` Additional authenticated data (may be NULL if aad\_len is 0). 
* `aad_len` AAD length in bytes. 
* `in` Ciphertext buffer to decrypt (may be NULL if in\_len is 0). 
* `in_len` Ciphertext length in bytes. 
* `tag` Authentication tag to verify against. 
* `tag_len` Length of authentication tag in bytes (4, 6, 8, 10, 12, 14, or 16). 
* `out` Plaintext output buffer (must be at least in\_len bytes). 



**Returns:**

SYN\_OK on success, SYN\_ERROR on tag mismatch, or SYN\_INVALID\_PARAM on invalid params. 





        

<hr>



### function syn\_aes\_ccm\_encrypt 

_AES-CCM authenticated encryption (NIST SP 800-38C / RFC 3610)._ 
```C++
SYN_Status syn_aes_ccm_encrypt (
    const SYN_AES_Context * ctx,
    const uint8_t * nonce,
    size_t nonce_len,
    const uint8_t * aad,
    size_t aad_len,
    const uint8_t * in,
    size_t in_len,
    uint8_t * out,
    uint8_t * tag,
    size_t tag_len
) 
```



Computes CBC-MAC authentication tag and encrypts payload in CTR mode.




**Parameters:**


* `ctx` Initialized AES context. 
* `nonce` Nonce buffer (length must be 7..13 bytes). 
* `nonce_len` Nonce length in bytes (7 to 13). 
* `aad` Additional authenticated data (may be NULL if aad\_len is 0). 
* `aad_len` AAD length in bytes. 
* `in` Plaintext buffer to encrypt (may be NULL if in\_len is 0). 
* `in_len` Plaintext length in bytes. 
* `out` Ciphertext output buffer (must be at least in\_len bytes). 
* `tag` Authentication tag output buffer. 
* `tag_len` Length of authentication tag in bytes (4, 6, 8, 10, 12, 14, or 16). 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM on invalid parameters. 





        

<hr>



### function syn\_aes\_ctr 

_Encrypt/decrypt arbitrary length data using AES-CTR stream mode (NIST SP 800-38A)._ 
```C++
SYN_Status syn_aes_ctr (
    const SYN_AES_Context * ctx,
    const uint8_t nonce,
    const uint8_t * in,
    size_t len,
    uint8_t * out
) 
```





**Parameters:**


* `ctx` Initialized AES context. 
* `nonce` 16-byte initial counter block (incremented as 128-bit big-endian). 
* `in` Input data buffer (plaintext for encrypt, ciphertext for decrypt). 
* `len` Length of input and output data in bytes. 
* `out` Output data buffer. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM on NULL pointers. 





        

<hr>



### function syn\_aes\_decrypt\_block 

_Decrypt a single 16-byte block (ECB mode)._ 
```C++
void syn_aes_decrypt_block (
    const SYN_AES_Context * ctx,
    const uint8_t in,
    uint8_t out
) 
```





**Parameters:**


* `ctx` Initialized AES context. 
* `in` 16-byte ciphertext input block. 
* `out` 16-byte plaintext output block. 




        

<hr>



### function syn\_aes\_encrypt\_block 

_Encrypt a single 16-byte block (ECB mode)._ 
```C++
void syn_aes_encrypt_block (
    const SYN_AES_Context * ctx,
    const uint8_t in,
    uint8_t out
) 
```





**Parameters:**


* `ctx` Initialized AES context. 
* `in` 16-byte plaintext input block. 
* `out` 16-byte ciphertext output block. 




        

<hr>



### function syn\_aes\_gcm\_decrypt 

_AES-GCM authenticated decryption and tag verification (NIST SP 800-38D)._ 
```C++
SYN_Status syn_aes_gcm_decrypt (
    const SYN_AES_GCM_Context * ctx,
    const uint8_t * nonce,
    size_t nonce_len,
    const uint8_t * aad,
    size_t aad_len,
    const uint8_t * in,
    size_t in_len,
    uint8_t * out,
    const uint8_t tag
) 
```



Decrypts ciphertext and verifies the authentication tag in constant time. If authentication fails, plaintext output is zeroed and SYN\_AUTH\_FAILED is returned.




**Parameters:**


* `ctx` Initialized AES-GCM context. 
* `nonce` Initialization vector / nonce buffer. 
* `nonce_len` Nonce length in bytes. 
* `aad` Additional authenticated data (may be NULL if aad\_len is 0). 
* `aad_len` AAD length in bytes. 
* `in` Ciphertext buffer to decrypt (may be NULL if in\_len is 0). 
* `in_len` Ciphertext length in bytes. 
* `out` Plaintext output buffer (must be at least in\_len bytes). 
* `tag` 16-byte expected authentication tag to verify against. 



**Returns:**

SYN\_OK on successful verification, SYN\_ERROR on tag mismatch, or SYN\_INVALID\_PARAM on invalid parameters. 





        

<hr>



### function syn\_aes\_gcm\_encrypt 

_AES-GCM authenticated encryption (NIST SP 800-38D)._ 
```C++
SYN_Status syn_aes_gcm_encrypt (
    const SYN_AES_GCM_Context * ctx,
    const uint8_t * nonce,
    size_t nonce_len,
    const uint8_t * aad,
    size_t aad_len,
    const uint8_t * in,
    size_t in_len,
    uint8_t * out,
    uint8_t tag
) 
```





**Parameters:**


* `ctx` Initialized AES-GCM context. 
* `nonce` Initialization vector / nonce buffer. 
* `nonce_len` Nonce length in bytes (standard is 12 bytes). 
* `aad` Additional authenticated data (may be NULL if aad\_len is 0). 
* `aad_len` AAD length in bytes. 
* `in` Plaintext buffer to encrypt (may be NULL if in\_len is 0). 
* `in_len` Plaintext length in bytes. 
* `out` Ciphertext output buffer (must be at least in\_len bytes). 
* `tag` 16-byte authentication tag output buffer. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM on NULL pointers or invalid params. 





        

<hr>



### function syn\_aes\_gcm\_init 

_Initialize AES-GCM AEAD context (expands key and computes GHASH subkey H)._ 
```C++
SYN_Status syn_aes_gcm_init (
    SYN_AES_GCM_Context * ctx,
    const uint8_t * key,
    size_t key_len
) 
```





**Parameters:**


* `ctx` Pointer to GCM context to initialize. 
* `key` Secret key buffer. 
* `key_len` Length of secret key in bytes (16, 24, or 32). 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM on failure. 





        

<hr>



### function syn\_aes\_ghash\_mult 

_Multiply 16-byte block by GHASH subkey H in GF(2^128)._ 
```C++
void syn_aes_ghash_mult (
    const uint8_t x,
    const uint8_t h,
    uint8_t out
) 
```





**Parameters:**


* `x` Input 16-byte field element. 
* `h` Input 16-byte GHASH subkey H. 
* `out` Output 16-byte product block. 




        

<hr>



### function syn\_aes\_init 

_Initialize AES context and perform key expansion for 128/192/256-bit key._ 
```C++
SYN_Status syn_aes_init (
    SYN_AES_Context * ctx,
    const uint8_t * key,
    size_t key_len
) 
```





**Parameters:**


* `ctx` Pointer to AES context to initialize. 
* `key` Secret key buffer. 
* `key_len` Length of secret key in bytes (16, 24, or 32). 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM on invalid key size or NULL pointers. 





        

<hr>
## Public Static Functions Documentation




### function ccm\_compute\_mac 

_Compute NIST SP 800-38C / RFC 3610 CBC-MAC tag over header and payload._ 
```C++
static SYN_Status ccm_compute_mac (
    const SYN_AES_Context * ctx,
    const uint8_t * nonce,
    size_t nonce_len,
    size_t L,
    const uint8_t * aad,
    size_t aad_len,
    const uint8_t * data,
    size_t data_len,
    size_t tag_len,
    uint8_t mac
) 
```





**Parameters:**


* `ctx` AES context. 
* `nonce` Nonce buffer. 
* `nonce_len` Nonce length in bytes. 
* `L` Length parameter (15 - nonce\_len). 
* `aad` Associated data buffer. 
* `aad_len` Associated data length in bytes. 
* `data` Payload data buffer. 
* `data_len` Payload data length in bytes. 
* `tag_len` Desired MAC tag length in bytes. 
* `mac` 16-byte raw CBC-MAC output buffer. 



**Returns:**

SYN\_Status SYN\_OK on success, error code otherwise. 





        

<hr>



### function ccm\_format\_ctr 

_Format counter block Ai for AES-CCM CTR mode._ 
```C++
static void ccm_format_ctr (
    uint8_t a,
    const uint8_t * nonce,
    size_t nonce_len,
    size_t L,
    uint64_t counter
) 
```





**Parameters:**


* `a` 16-byte formatted counter output block. 
* `nonce` Pointer to nonce buffer. 
* `nonce_len` Length of nonce in bytes. 
* `L` Length parameter (15 - nonce\_len). 
* `counter` Big-endian counter integer. 




        

<hr>



### function ccm\_mac\_feed 

_Feed data into AES-CCM CBC-MAC accumulator._ 
```C++
static void ccm_mac_feed (
    const SYN_AES_Context * ctx,
    uint8_t mac,
    uint8_t * blk,
    size_t * blk_len,
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `ctx` AES context. 
* `mac` Running CBC-MAC accumulator block. 
* `blk` Partial block staging buffer. 
* `blk_len` Pointer to length of valid bytes in staging buffer. 
* `data` Input byte buffer to feed into MAC. 
* `len` Number of bytes in input data buffer. 




        

<hr>



### function ccm\_mac\_pad\_zero 

_Pad partial block with zeros and finish block encryption in CBC-MAC._ 
```C++
static void ccm_mac_pad_zero (
    const SYN_AES_Context * ctx,
    uint8_t mac,
    uint8_t * blk,
    size_t * blk_len
) 
```





**Parameters:**


* `ctx` AES context. 
* `mac` Running CBC-MAC accumulator block. 
* `blk` Partial block staging buffer. 
* `blk_len` Pointer to length of valid bytes in staging buffer. 




        

<hr>



### function gcm\_compute\_j0 

_Compute initial J0 counter block for GCM._ 
```C++
static void gcm_compute_j0 (
    const SYN_AES_GCM_Context * ctx,
    const uint8_t * nonce,
    size_t nonce_len,
    uint8_t j0
) 
```





**Parameters:**


* `ctx` GCM context. 
* `nonce` Nonce buffer. 
* `nonce_len` Nonce length in bytes. 
* `j0` Output 16-byte J0 block. 




        

<hr>



### function ghash\_mult\_bit 

_Bitwise GHASH field multiplication in GF(2^128)._ 
```C++
static void ghash_mult_bit (
    const uint8_t x,
    const uint8_t y,
    uint8_t out
) 
```





**Parameters:**


* `x` First 16-byte block. 
* `y` Second 16-byte block. 
* `out` Output 16-byte block product. 




        

<hr>



### function ghash\_process\_blocks 

_Process data blocks through GHASH._ 
```C++
static void ghash_process_blocks (
    const SYN_AES_GCM_Context * ctx,
    const uint8_t * data,
    size_t len,
    uint8_t y
) 
```





**Parameters:**


* `ctx` GCM context. 
* `data` Input data buffer. 
* `len` Data length in bytes. 
* `y` In/out 16-byte GHASH state accumulator. 




        

<hr>



### function gmult 

_General multiplication in GF(2^8)._ 
```C++
static inline uint8_t gmult (
    uint8_t a,
    uint8_t b
) 
```





**Parameters:**


* `a` First operand. 
* `b` Second operand. 



**Returns:**

Product in GF(2^8). 





        

<hr>



### function gmult2 

_Multiply byte by 2 in GF(2^8) modulo x^8 + x^4 + x^3 + x + 1._ 
```C++
static inline uint8_t gmult2 (
    uint8_t a
) 
```





**Parameters:**


* `a` Input byte. 



**Returns:**

Multiplied byte. 





        

<hr>



### function inc128 

_Increment 128-bit big-endian counter._ 
```C++
static void inc128 (
    uint8_t counter
) 
```





**Parameters:**


* `counter` 16-byte counter block to increment in place. 




        

<hr>



### function inc32 

_Increment rightmost 32 bits of 128-bit counter._ 
```C++
static void inc32 (
    uint8_t block
) 
```





**Parameters:**


* `block` 16-byte block whose last 4 bytes are incremented. 




        

<hr>



### function put\_be64 

_Write 64-bit value big-endian._ 
```C++
static void put_be64 (
    uint8_t out,
    uint64_t val
) 
```





**Parameters:**


* `out` 8-byte buffer. 
* `val` 64-bit integer. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_aes.c`

