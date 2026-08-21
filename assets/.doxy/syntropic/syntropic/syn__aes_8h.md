

# File syn\_aes.h



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_aes.h**](syn__aes_8h.md)

[Go to the source code of this file](syn__aes_8h_source.md)

_Unified AES cipher engine (128/192/256-bit keys, ECB, CBC, CTR, and GCM AEAD)._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_AES\_Context**](structSYN__AES__Context.md) <br>_AES context containing expanded round keys._  |
| struct | [**SYN\_AES\_GCM\_Context**](structSYN__AES__GCM__Context.md) <br>_AES-GCM AEAD context — wraps AES key schedule and GHASH subkey/tables._  |






















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



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_AES\_BLOCK\_SIZE**](syn__aes_8h.md#define-syn_aes_block_size)  `16U`<br>_AES cipher block size in bytes (16 bytes / 128 bits)._  |
| define  | [**SYN\_AES\_GCM\_IV\_DEFAULT\_SIZE**](syn__aes_8h.md#define-syn_aes_gcm_iv_default_size)  `12U`<br>_Recommended GCM initialization vector size in bytes (12 bytes / 96 bits)._  |
| define  | [**SYN\_AES\_GCM\_TABLE**](syn__aes_8h.md#define-syn_aes_gcm_table)  `0`<br>_GCM GHASH acceleration table strategy. 0 = bit-by-bit Shoup (0 bytes table, low footprint), 4 = 4-bit nibble table (256 bytes per GCM context), 8 = 8-bit byte table (4096 bytes per GCM context)._  |
| define  | [**SYN\_AES\_GCM\_TAG\_SIZE**](syn__aes_8h.md#define-syn_aes_gcm_tag_size)  `16U`<br>_Standard GCM authentication tag size in bytes (16 bytes / 128 bits)._  |
| define  | [**SYN\_AES\_KEY\_SIZE\_128**](syn__aes_8h.md#define-syn_aes_key_size_128)  `16U`<br>_AES-128 key size in bytes (16 bytes / 128 bits)._  |
| define  | [**SYN\_AES\_KEY\_SIZE\_192**](syn__aes_8h.md#define-syn_aes_key_size_192)  `24U`<br>_AES-192 key size in bytes (24 bytes / 192 bits)._  |
| define  | [**SYN\_AES\_KEY\_SIZE\_256**](syn__aes_8h.md#define-syn_aes_key_size_256)  `32U`<br>_AES-256 key size in bytes (32 bytes / 256 bits)._  |
| define  | [**SYN\_AES\_MAX\_EXPANDED\_KEY**](syn__aes_8h.md#define-syn_aes_max_expanded_key)  `240U`<br>_Maximum expanded round keys storage size in bytes._  |
| define  | [**SYN\_AES\_MAX\_KEY\_BITS**](syn__aes_8h.md#define-syn_aes_max_key_bits)  `256`<br>_Maximum key size in bits (128, 192, or 256). Controls context RAM footprint._  |
| define  | [**SYN\_AES\_MAX\_ROUNDS**](syn__aes_8h.md#define-syn_aes_max_rounds)  `14U`<br>_Maximum rounds for configured key size._  |
| define  | [**SYN\_USE\_AES\_CBC**](syn__aes_8h.md#define-syn_use_aes_cbc)  `1`<br>_Enable CBC mode encryption and decryption with PKCS#7 padding._  |
| define  | [**SYN\_USE\_AES\_CTR**](syn__aes_8h.md#define-syn_use_aes_ctr)  `1`<br>_Enable CTR stream cipher mode._  |
| define  | [**SYN\_USE\_AES\_DECRYPT**](syn__aes_8h.md#define-syn_use_aes_decrypt)  `1`<br>_Enable ECB and CBC decryption functions (pulls rsbox, +256B Flash)._  |
| define  | [**SYN\_USE\_AES\_GCM**](syn__aes_8h.md#define-syn_use_aes_gcm)  `1`<br>_Enable GCM AEAD authenticated encryption and decryption._  |

## Detailed Description


Zero-heap, constant-time implementation supporting AES-128, AES-192, and AES-256 with configurable compile-time memory footprint guardrails. 


    
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
## Macro Definition Documentation





### define SYN\_AES\_BLOCK\_SIZE 

_AES cipher block size in bytes (16 bytes / 128 bits)._ 
```C++
#define SYN_AES_BLOCK_SIZE `16U`
```




<hr>



### define SYN\_AES\_GCM\_IV\_DEFAULT\_SIZE 

_Recommended GCM initialization vector size in bytes (12 bytes / 96 bits)._ 
```C++
#define SYN_AES_GCM_IV_DEFAULT_SIZE `12U`
```




<hr>



### define SYN\_AES\_GCM\_TABLE 

_GCM GHASH acceleration table strategy. 0 = bit-by-bit Shoup (0 bytes table, low footprint), 4 = 4-bit nibble table (256 bytes per GCM context), 8 = 8-bit byte table (4096 bytes per GCM context)._ 
```C++
#define SYN_AES_GCM_TABLE `0`
```




<hr>



### define SYN\_AES\_GCM\_TAG\_SIZE 

_Standard GCM authentication tag size in bytes (16 bytes / 128 bits)._ 
```C++
#define SYN_AES_GCM_TAG_SIZE `16U`
```




<hr>



### define SYN\_AES\_KEY\_SIZE\_128 

_AES-128 key size in bytes (16 bytes / 128 bits)._ 
```C++
#define SYN_AES_KEY_SIZE_128 `16U`
```




<hr>



### define SYN\_AES\_KEY\_SIZE\_192 

_AES-192 key size in bytes (24 bytes / 192 bits)._ 
```C++
#define SYN_AES_KEY_SIZE_192 `24U`
```




<hr>



### define SYN\_AES\_KEY\_SIZE\_256 

_AES-256 key size in bytes (32 bytes / 256 bits)._ 
```C++
#define SYN_AES_KEY_SIZE_256 `32U`
```




<hr>



### define SYN\_AES\_MAX\_EXPANDED\_KEY 

_Maximum expanded round keys storage size in bytes._ 
```C++
#define SYN_AES_MAX_EXPANDED_KEY `240U`
```




<hr>



### define SYN\_AES\_MAX\_KEY\_BITS 

_Maximum key size in bits (128, 192, or 256). Controls context RAM footprint._ 
```C++
#define SYN_AES_MAX_KEY_BITS `256`
```




<hr>



### define SYN\_AES\_MAX\_ROUNDS 

_Maximum rounds for configured key size._ 
```C++
#define SYN_AES_MAX_ROUNDS `14U`
```




<hr>



### define SYN\_USE\_AES\_CBC 

_Enable CBC mode encryption and decryption with PKCS#7 padding._ 
```C++
#define SYN_USE_AES_CBC `1`
```




<hr>



### define SYN\_USE\_AES\_CTR 

_Enable CTR stream cipher mode._ 
```C++
#define SYN_USE_AES_CTR `1`
```




<hr>



### define SYN\_USE\_AES\_DECRYPT 

_Enable ECB and CBC decryption functions (pulls rsbox, +256B Flash)._ 
```C++
#define SYN_USE_AES_DECRYPT `1`
```




<hr>



### define SYN\_USE\_AES\_GCM 

_Enable GCM AEAD authenticated encryption and decryption._ 
```C++
#define SYN_USE_AES_GCM `1`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_aes.h`

