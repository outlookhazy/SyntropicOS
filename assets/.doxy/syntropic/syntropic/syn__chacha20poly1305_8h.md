

# File syn\_chacha20poly1305.h



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_chacha20poly1305.h**](syn__chacha20poly1305_8h.md)

[Go to the source code of this file](syn__chacha20poly1305_8h_source.md)

_ChaCha20-Poly1305 AEAD — RFC 8439, pure C99._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_aead\_decrypt**](#function-syn_aead_decrypt) (const uint8\_t key, const uint8\_t nonce, const uint8\_t \* aad, size\_t aad\_len, const uint8\_t \* ciphertext, size\_t ct\_len, const uint8\_t tag, uint8\_t \* plaintext) <br>_Decrypt and verify (AEAD)._  |
|  void | [**syn\_aead\_encrypt**](#function-syn_aead_encrypt) (const uint8\_t key, const uint8\_t nonce, const uint8\_t \* aad, size\_t aad\_len, const uint8\_t \* plaintext, size\_t pt\_len, uint8\_t \* ciphertext, uint8\_t tag) <br>_Encrypt and authenticate (AEAD)._  |
|  void | [**syn\_chacha20\_block**](#function-syn_chacha20_block) (const uint8\_t key, const uint8\_t nonce, uint32\_t counter, uint8\_t out) <br>_Generate ChaCha20 keystream block (no XOR)._  |
|  void | [**syn\_chacha20\_xor**](#function-syn_chacha20_xor) (const uint8\_t key, const uint8\_t nonce, uint32\_t counter, const uint8\_t \* in, size\_t len, uint8\_t \* out) <br>_XOR data with ChaCha20 keystream._  |




























## Detailed Description


Provides the Authenticated Encryption with Associated Data (AEAD) construction used by WireGuard for both handshake and transport encryption. Also exposes raw ChaCha20 for standalone use.


All state is caller-owned. No heap.




**
**


```C++
uint8_t key[32], nonce[12], tag[16];
uint8_t plaintext[64], ciphertext[64];

// Encrypt:
syn_aead_encrypt(key, nonce, NULL, 0,
                 plaintext, 64, ciphertext, tag);

// Decrypt:
bool ok = syn_aead_decrypt(key, nonce, NULL, 0,
                           ciphertext, 64, tag, plaintext);
```
 





    
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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_chacha20poly1305.h`

