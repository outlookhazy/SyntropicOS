

# File syn\_blake2s.h



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_blake2s.h**](syn__blake2s_8h.md)

[Go to the source code of this file](syn__blake2s_8h_source.md)

_BLAKE2s cryptographic hash — RFC 7693, pure C99._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <string.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_BLAKE2s**](structSYN__BLAKE2s.md) <br>_BLAKE2s hash context — caller-owned._  |
| struct | [**SYN\_HMAC\_BLAKE2s**](structSYN__HMAC__BLAKE2s.md) <br>_HMAC-BLAKE2s context — caller-owned._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_blake2s\_final**](#function-syn_blake2s_final) ([**SYN\_BLAKE2s**](structSYN__BLAKE2s.md) \* ctx, uint8\_t \* out) <br>_Finalize and produce the digest._  |
|  void | [**syn\_blake2s\_init**](#function-syn_blake2s_init) ([**SYN\_BLAKE2s**](structSYN__BLAKE2s.md) \* ctx, size\_t outlen) <br>_Initialize BLAKE2s for unkeyed hashing._  |
|  void | [**syn\_blake2s\_init\_keyed**](#function-syn_blake2s_init_keyed) ([**SYN\_BLAKE2s**](structSYN__BLAKE2s.md) \* ctx, const void \* key, size\_t keylen, size\_t outlen) <br>_Initialize BLAKE2s for keyed hashing (MAC mode)._  |
|  void | [**syn\_blake2s\_update**](#function-syn_blake2s_update) ([**SYN\_BLAKE2s**](structSYN__BLAKE2s.md) \* ctx, const void \* data, size\_t len) <br>_Feed data into the hash._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_blake2s**](#function-syn_blake2s) (const void \* data, size\_t len, uint8\_t \* out, size\_t outlen) <br>_One-shot unkeyed BLAKE2s hash._  |
|  void | [**syn\_blake2s\_mac**](#function-syn_blake2s_mac) (const void \* key, size\_t keylen, const void \* data, size\_t len, uint8\_t \* out, size\_t outlen) <br>_One-shot keyed BLAKE2s MAC._  |
|  void | [**syn\_hmac\_blake2s**](#function-syn_hmac_blake2s) (const void \* key, size\_t keylen, const void \* data, size\_t datalen, uint8\_t mac) <br>_One-shot HMAC-BLAKE2s._  |
|  void | [**syn\_hmac\_blake2s\_final**](#function-syn_hmac_blake2s_final) ([**SYN\_HMAC\_BLAKE2s**](structSYN__HMAC__BLAKE2s.md) \* ctx, uint8\_t mac) <br>_Finalize HMAC-BLAKE2s and produce 32-byte MAC._  |
|  void | [**syn\_hmac\_blake2s\_init**](#function-syn_hmac_blake2s_init) ([**SYN\_HMAC\_BLAKE2s**](structSYN__HMAC__BLAKE2s.md) \* ctx, const void \* key, size\_t keylen) <br>_Initialize HMAC-BLAKE2s with a key._  |
|  void | [**syn\_hmac\_blake2s\_update**](#function-syn_hmac_blake2s_update) ([**SYN\_HMAC\_BLAKE2s**](structSYN__HMAC__BLAKE2s.md) \* ctx, const void \* data, size\_t len) <br>_Feed message data into the HMAC._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_BLAKE2S\_BLOCK\_SIZE**](syn__blake2s_8h.md#define-syn_blake2s_block_size)  `64`<br> |
| define  | [**SYN\_BLAKE2S\_MAX\_DIGEST**](syn__blake2s_8h.md#define-syn_blake2s_max_digest)  `32`<br> |
| define  | [**SYN\_BLAKE2S\_MAX\_KEY**](syn__blake2s_8h.md#define-syn_blake2s_max_key)  `32`<br> |

## Detailed Description


Supports both unkeyed hashing and keyed MAC mode. WireGuard uses both: unkeyed BLAKE2s-256 for chaining hash, keyed BLAKE2s for MAC.


Context is caller-owned (~120 bytes on 32-bit targets). No heap.


Also provides HMAC-BLAKE2s as static inline functions — used internally by the WireGuard module for HKDF key derivation.




**
**


```C++
// Unkeyed hash:
uint8_t hash[32];
syn_blake2s(data, len, hash, 32);

// Keyed MAC (16-byte tag):
uint8_t mac[16];
syn_blake2s_mac(key, 32, data, len, mac, 16);

// Streaming:
SYN_BLAKE2s ctx;
syn_blake2s_init(&ctx, 32);
syn_blake2s_update(&ctx, chunk1, len1);
syn_blake2s_update(&ctx, chunk2, len2);
syn_blake2s_final(&ctx, hash);
```
 





    
## Public Functions Documentation




### function syn\_blake2s\_final 

_Finalize and produce the digest._ 
```C++
void syn_blake2s_final (
    SYN_BLAKE2s * ctx,
    uint8_t * out
) 
```



After calling this, the context must be re-initialized before reuse.




**Parameters:**


* `ctx` BLAKE2s context. 
* `out` Output buffer (must be at least ctx-&gt;outlen bytes). 




        

<hr>



### function syn\_blake2s\_init 

_Initialize BLAKE2s for unkeyed hashing._ 
```C++
void syn_blake2s_init (
    SYN_BLAKE2s * ctx,
    size_t outlen
) 
```





**Parameters:**


* `ctx` Context to initialize. 
* `outlen` Desired digest length (1–32). 




        

<hr>



### function syn\_blake2s\_init\_keyed 

_Initialize BLAKE2s for keyed hashing (MAC mode)._ 
```C++
void syn_blake2s_init_keyed (
    SYN_BLAKE2s * ctx,
    const void * key,
    size_t keylen,
    size_t outlen
) 
```





**Parameters:**


* `ctx` Context to initialize. 
* `key` Secret key. 
* `keylen` Key length (1–32). 
* `outlen` Desired digest length (1–32). 




        

<hr>



### function syn\_blake2s\_update 

_Feed data into the hash._ 
```C++
void syn_blake2s_update (
    SYN_BLAKE2s * ctx,
    const void * data,
    size_t len
) 
```





**Parameters:**


* `ctx` BLAKE2s context. 
* `data` Data to hash. 
* `len` Length in bytes. 




        

<hr>
## Public Static Functions Documentation




### function syn\_blake2s 

_One-shot unkeyed BLAKE2s hash._ 
```C++
static inline void syn_blake2s (
    const void * data,
    size_t len,
    uint8_t * out,
    size_t outlen
) 
```





**Parameters:**


* `data` Data to hash. 
* `len` Length in bytes. 
* `out` Output buffer. 
* `outlen` Desired digest length (1–32). 




        

<hr>



### function syn\_blake2s\_mac 

_One-shot keyed BLAKE2s MAC._ 
```C++
static inline void syn_blake2s_mac (
    const void * key,
    size_t keylen,
    const void * data,
    size_t len,
    uint8_t * out,
    size_t outlen
) 
```





**Parameters:**


* `key` Secret key. 
* `keylen` Key length (1–32). 
* `data` Data to authenticate. 
* `len` Data length in bytes. 
* `out` Output buffer. 
* `outlen` Desired MAC length (1–32). 




        

<hr>



### function syn\_hmac\_blake2s 

_One-shot HMAC-BLAKE2s._ 
```C++
static inline void syn_hmac_blake2s (
    const void * key,
    size_t keylen,
    const void * data,
    size_t datalen,
    uint8_t mac
) 
```





**Parameters:**


* `key` Secret key. 
* `keylen` Key length in bytes. 
* `data` Message data. 
* `datalen` Message length in bytes. 
* `mac` Output buffer for the 32-byte MAC. 




        

<hr>



### function syn\_hmac\_blake2s\_final 

_Finalize HMAC-BLAKE2s and produce 32-byte MAC._ 
```C++
static inline void syn_hmac_blake2s_final (
    SYN_HMAC_BLAKE2s * ctx,
    uint8_t mac
) 
```





**Parameters:**


* `ctx` HMAC context. 
* `mac` Output buffer for the 32-byte MAC. 




        

<hr>



### function syn\_hmac\_blake2s\_init 

_Initialize HMAC-BLAKE2s with a key._ 
```C++
static inline void syn_hmac_blake2s_init (
    SYN_HMAC_BLAKE2s * ctx,
    const void * key,
    size_t keylen
) 
```





**Parameters:**


* `ctx` HMAC context. 
* `key` Secret key. 
* `keylen` Key length in bytes. 




        

<hr>



### function syn\_hmac\_blake2s\_update 

_Feed message data into the HMAC._ 
```C++
static inline void syn_hmac_blake2s_update (
    SYN_HMAC_BLAKE2s * ctx,
    const void * data,
    size_t len
) 
```





**Parameters:**


* `ctx` HMAC context. 
* `data` Message data. 
* `len` Data length in bytes. 




        

<hr>
## Macro Definition Documentation





### define SYN\_BLAKE2S\_BLOCK\_SIZE 

```C++
#define SYN_BLAKE2S_BLOCK_SIZE `64`
```



Input block size (bytes) 
 


        

<hr>



### define SYN\_BLAKE2S\_MAX\_DIGEST 

```C++
#define SYN_BLAKE2S_MAX_DIGEST `32`
```



Maximum digest size (bytes) 
 


        

<hr>



### define SYN\_BLAKE2S\_MAX\_KEY 

```C++
#define SYN_BLAKE2S_MAX_KEY `32`
```



Maximum key size (bytes) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_blake2s.h`

