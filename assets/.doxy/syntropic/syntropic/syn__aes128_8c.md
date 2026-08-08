

# File syn\_aes128.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_aes128.c**](syn__aes128_8c.md)

[Go to the source code of this file](syn__aes128_8c_source.md)

_AES-128 implementation (S-Box, Key Expansion, ECB, CBC mode)._ 

* `#include "syn_aes128.h"`
* `#include "syn_assert.h"`
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
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes128\_cbc\_decrypt**](#function-syn_aes128_cbc_decrypt) (const [**SYN\_AES128\_Context**](structSYN__AES128__Context.md) \* ctx, const uint8\_t iv, const uint8\_t \* in, size\_t in\_len, uint8\_t \* out, size\_t out\_capacity, size\_t \* out\_len) <br>_Decrypt data using AES-128-CBC with PKCS#7 padding removal._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes128\_cbc\_encrypt**](#function-syn_aes128_cbc_encrypt) (const [**SYN\_AES128\_Context**](structSYN__AES128__Context.md) \* ctx, const uint8\_t iv, const uint8\_t \* in, size\_t in\_len, uint8\_t \* out, size\_t out\_capacity, size\_t \* out\_len) <br>_Encrypt data using AES-128-CBC with PKCS#7 padding._  |
|  void | [**syn\_aes128\_decrypt\_block**](#function-syn_aes128_decrypt_block) (const [**SYN\_AES128\_Context**](structSYN__AES128__Context.md) \* ctx, const uint8\_t in, uint8\_t out) <br>_Decrypt a single 16-byte block (ECB mode)._  |
|  void | [**syn\_aes128\_encrypt\_block**](#function-syn_aes128_encrypt_block) (const [**SYN\_AES128\_Context**](structSYN__AES128__Context.md) \* ctx, const uint8\_t in, uint8\_t out) <br>_Encrypt a single 16-byte block (ECB mode)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_aes128\_init**](#function-syn_aes128_init) ([**SYN\_AES128\_Context**](structSYN__AES128__Context.md) \* ctx, const uint8\_t key) <br>_Initialize AES-128 context and expand 128-bit key._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**gmult**](#function-gmult) (uint8\_t a, uint8\_t b) <br>_Galois field multiplication of two bytes._  |
|  uint8\_t | [**gmult2**](#function-gmult2) (uint8\_t a) <br>_Galois field multiplication by 2._  |


























## Public Static Attributes Documentation




### variable rcon 

```C++
const uint8_t rcon[11];
```



Round constants array 


        

<hr>



### variable rsbox 

```C++
const uint8_t rsbox[256];
```



Inverse S-Box table 


        

<hr>



### variable sbox 

```C++
const uint8_t sbox[256];
```



Forward S-Box table 


        

<hr>
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
## Public Static Functions Documentation




### function gmult 

_Galois field multiplication of two bytes._ 
```C++
static inline uint8_t gmult (
    uint8_t a,
    uint8_t b
) 
```





**Parameters:**


* `a` First byte operand. 
* `b` Second byte operand. 



**Returns:**

Product byte. 





        

<hr>



### function gmult2 

_Galois field multiplication by 2._ 
```C++
static inline uint8_t gmult2 (
    uint8_t a
) 
```





**Parameters:**


* `a` Input byte. 



**Returns:**

Product byte. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_aes128.c`

