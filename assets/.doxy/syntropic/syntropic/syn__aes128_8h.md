

# File syn\_aes128.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_aes128.h**](syn__aes128_8h.md)

[Go to the source code of this file](syn__aes128_8h_source.md)

_AES-128 cipher engine (ECB & CBC mode with PKCS#7 padding)._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_AES128\_Context**](structSYN__AES128__Context.md) <br>_AES-128 Context — stores round keys._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes128\_cbc\_decrypt**](#function-syn_aes128_cbc_decrypt) (const [**SYN\_AES128\_Context**](structSYN__AES128__Context.md) \* ctx, const uint8\_t iv, const uint8\_t \* in, size\_t in\_len, uint8\_t \* out, size\_t out\_capacity, size\_t \* out\_len) <br>_Decrypt data using AES-128-CBC with PKCS#7 padding removal._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes128\_cbc\_encrypt**](#function-syn_aes128_cbc_encrypt) (const [**SYN\_AES128\_Context**](structSYN__AES128__Context.md) \* ctx, const uint8\_t iv, const uint8\_t \* in, size\_t in\_len, uint8\_t \* out, size\_t out\_capacity, size\_t \* out\_len) <br>_Encrypt data using AES-128-CBC with PKCS#7 padding._  |
|  void | [**syn\_aes128\_decrypt\_block**](#function-syn_aes128_decrypt_block) (const [**SYN\_AES128\_Context**](structSYN__AES128__Context.md) \* ctx, const uint8\_t in, uint8\_t out) <br>_Decrypt a single 16-byte block (ECB mode)._  |
|  void | [**syn\_aes128\_encrypt\_block**](#function-syn_aes128_encrypt_block) (const [**SYN\_AES128\_Context**](structSYN__AES128__Context.md) \* ctx, const uint8\_t in, uint8\_t out) <br>_Encrypt a single 16-byte block (ECB mode)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes128\_init**](#function-syn_aes128_init) ([**SYN\_AES128\_Context**](structSYN__AES128__Context.md) \* ctx, const uint8\_t key) <br>_Initialize AES-128 context and expand 128-bit key._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_AES128\_BLOCK\_SIZE**](syn__aes128_8h.md#define-syn_aes128_block_size)  `16`<br> |
| define  | [**SYN\_AES128\_EXPANDED\_KEY\_SIZE**](syn__aes128_8h.md#define-syn_aes128_expanded_key_size)  `176`<br> |
| define  | [**SYN\_AES128\_KEY\_SIZE**](syn__aes128_8h.md#define-syn_aes128_key_size)  `16`<br> |

## Detailed Description


Constant-time, zero-heap implementation designed for embedded systems. Supports 128-bit key expansion and CBC cipher block chaining. 


    
## Public Functions Documentation




### function syn\_aes128\_cbc\_decrypt 

_Decrypt data using AES-128-CBC with PKCS#7 padding removal._ 
```C++
SYN_Status syn_aes128_cbc_decrypt (
    const SYN_AES128_Context * ctx,
    const uint8_t iv,
    const uint8_t * in,
    size_t in_len,
    uint8_t * out,
    size_t out_capacity,
    size_t * out_len
) 
```





**Parameters:**


* `ctx` Initialized AES-128 context. 
* `iv` 16-byte initialization vector. 
* `in` Ciphertext buffer (multiple of 16 bytes). 
* `in_len` Ciphertext length in bytes. 
* `out` Plaintext output buffer. 
* `out_capacity` Output buffer size. 
* `out_len` Decrypted plaintext byte count written. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM on invalid padding. 





        

<hr>



### function syn\_aes128\_cbc\_encrypt 

_Encrypt data using AES-128-CBC with PKCS#7 padding._ 
```C++
SYN_Status syn_aes128_cbc_encrypt (
    const SYN_AES128_Context * ctx,
    const uint8_t iv,
    const uint8_t * in,
    size_t in_len,
    uint8_t * out,
    size_t out_capacity,
    size_t * out_len
) 
```





**Parameters:**


* `ctx` Initialized AES-128 context. 
* `iv` 16-byte initialization vector. 
* `in` Plaintext buffer. 
* `in_len` Plaintext length in bytes. 
* `out` Ciphertext buffer (must be large enough for in\_len + PKCS7 padding). 
* `out_capacity` Maximum size of output buffer. 
* `out_len` Output byte count written. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_aes128\_decrypt\_block 

_Decrypt a single 16-byte block (ECB mode)._ 
```C++
void syn_aes128_decrypt_block (
    const SYN_AES128_Context * ctx,
    const uint8_t in,
    uint8_t out
) 
```





**Parameters:**


* `ctx` Initialized AES-128 context. 
* `in` 16-byte ciphertext block. 
* `out` 16-byte plaintext block. 




        

<hr>



### function syn\_aes128\_encrypt\_block 

_Encrypt a single 16-byte block (ECB mode)._ 
```C++
void syn_aes128_encrypt_block (
    const SYN_AES128_Context * ctx,
    const uint8_t in,
    uint8_t out
) 
```





**Parameters:**


* `ctx` Initialized AES-128 context. 
* `in` 16-byte plaintext block. 
* `out` 16-byte ciphertext block. 




        

<hr>



### function syn\_aes128\_init 

_Initialize AES-128 context and expand 128-bit key._ 
```C++
SYN_Status syn_aes128_init (
    SYN_AES128_Context * ctx,
    const uint8_t key
) 
```





**Parameters:**


* `ctx` AES-128 context instance. 
* `key` 16-byte key buffer. 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Macro Definition Documentation





### define SYN\_AES128\_BLOCK\_SIZE 

```C++
#define SYN_AES128_BLOCK_SIZE `16`
```



AES-128 cipher block size in bytes (16) 


        

<hr>



### define SYN\_AES128\_EXPANDED\_KEY\_SIZE 

```C++
#define SYN_AES128_EXPANDED_KEY_SIZE `176`
```



Total expanded round keys size in bytes (176) 


        

<hr>



### define SYN\_AES128\_KEY\_SIZE 

```C++
#define SYN_AES128_KEY_SIZE `16`
```



AES-128 key length in bytes (16) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_aes128.h`

