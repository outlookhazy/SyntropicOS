

# File syn\_sha512.h



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_sha512.h**](syn__sha512_8h.md)

[Go to the source code of this file](syn__sha512_8h_source.md)

_SHA-512 & SHA-384 cryptographic hash and HMAC — pure C99, zero dependencies._ [More...](#detailed-description)

* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_HMAC\_SHA384**](structSYN__HMAC__SHA384.md) <br>_HMAC-SHA384 context — caller-owned._  |
| struct | [**SYN\_HMAC\_SHA512**](structSYN__HMAC__SHA512.md) <br>_HMAC-SHA512 context — caller-owned._  |
| struct | [**SYN\_SHA512**](structSYN__SHA512.md) <br>_SHA-512 / SHA-384 hash context — caller-owned._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_hmac\_sha384**](#function-syn_hmac_sha384) (const void \* key, size\_t key\_len, const void \* data, size\_t data\_len, uint8\_t mac) <br>_Compute HMAC-SHA384 over data using secret key (RFC 4231)._  |
|  void | [**syn\_hmac\_sha384\_final**](#function-syn_hmac_sha384_final) ([**SYN\_HMAC\_SHA384**](structSYN__HMAC__SHA384.md) \* ctx, uint8\_t mac) <br>_Finalize HMAC-SHA384 computation and retrieve 48-byte MAC._  |
|  void | [**syn\_hmac\_sha384\_init**](#function-syn_hmac_sha384_init) ([**SYN\_HMAC\_SHA384**](structSYN__HMAC__SHA384.md) \* ctx, const void \* key, size\_t key\_len) <br>_Initialize HMAC-SHA384 context with a key._  |
|  void | [**syn\_hmac\_sha384\_update**](#function-syn_hmac_sha384_update) ([**SYN\_HMAC\_SHA384**](structSYN__HMAC__SHA384.md) \* ctx, const void \* data, size\_t len) <br>_Feed data chunk into HMAC-SHA384 computation._  |
|  void | [**syn\_hmac\_sha512**](#function-syn_hmac_sha512) (const void \* key, size\_t key\_len, const void \* data, size\_t data\_len, uint8\_t mac) <br>_Compute HMAC-SHA512 over data using secret key (RFC 4231)._  |
|  void | [**syn\_hmac\_sha512\_final**](#function-syn_hmac_sha512_final) ([**SYN\_HMAC\_SHA512**](structSYN__HMAC__SHA512.md) \* ctx, uint8\_t mac) <br>_Finalize HMAC-SHA512 computation and retrieve 64-byte MAC._  |
|  void | [**syn\_hmac\_sha512\_init**](#function-syn_hmac_sha512_init) ([**SYN\_HMAC\_SHA512**](structSYN__HMAC__SHA512.md) \* ctx, const void \* key, size\_t key\_len) <br>_Initialize HMAC-SHA512 context with a key._  |
|  void | [**syn\_hmac\_sha512\_update**](#function-syn_hmac_sha512_update) ([**SYN\_HMAC\_SHA512**](structSYN__HMAC__SHA512.md) \* ctx, const void \* data, size\_t len) <br>_Feed data chunk into HMAC-SHA512 computation._  |
|  void | [**syn\_sha384**](#function-syn_sha384) (const void \* data, size\_t len, uint8\_t hash) <br>_One-shot SHA-384 calculation._  |
|  void | [**syn\_sha384\_final**](#function-syn_sha384_final) ([**SYN\_SHA512**](structSYN__SHA512.md) \* ctx, uint8\_t hash) <br>_Finalize SHA-384 hash and produce 48-byte digest._  |
|  void | [**syn\_sha384\_init**](#function-syn_sha384_init) ([**SYN\_SHA512**](structSYN__SHA512.md) \* ctx) <br>_Initialize context for SHA-384 hashing._  |
|  void | [**syn\_sha512**](#function-syn_sha512) (const void \* data, size\_t len, uint8\_t hash) <br>_One-shot SHA-512 calculation._  |
|  void | [**syn\_sha512\_final**](#function-syn_sha512_final) ([**SYN\_SHA512**](structSYN__SHA512.md) \* ctx, uint8\_t hash) <br>_Finalize SHA-512 hash and produce 64-byte digest._  |
|  void | [**syn\_sha512\_init**](#function-syn_sha512_init) ([**SYN\_SHA512**](structSYN__SHA512.md) \* ctx) <br>_Initialize context for SHA-512 hashing._  |
|  void | [**syn\_sha512\_update**](#function-syn_sha512_update) ([**SYN\_SHA512**](structSYN__SHA512.md) \* ctx, const void \* data, size\_t len) <br>_Feed data into the SHA-512 / SHA-384 hash._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_SHA384\_DIGEST\_SIZE**](syn__sha512_8h.md#define-syn_sha384_digest_size)  `48U`<br>_SHA-384 digest size in bytes (48)._  |
| define  | [**SYN\_SHA512\_BLOCK\_SIZE**](syn__sha512_8h.md#define-syn_sha512_block_size)  `128U`<br>_SHA-512 / SHA-384 block size in bytes (128)._  |
| define  | [**SYN\_SHA512\_DIGEST\_SIZE**](syn__sha512_8h.md#define-syn_sha512_digest_size)  `64U`<br>_SHA-512 digest size in bytes (64)._  |

## Detailed Description


Implements FIPS 180-4 compliant SHA-512 and SHA-384 secure hash algorithms and RFC 4231 HMAC-SHA512 / HMAC-SHA384 message authentication.


Follows the streaming pattern: init → update (repeated) → final


The context struct is caller-owned (~216 bytes on 32-bit/64-bit targets). Zero heap allocation, zero floating point.




**
**


```C++
// One-shot SHA-512:
uint8_t hash512[64];
syn_sha512("abc", 3, hash512);

// One-shot SHA-384:
uint8_t hash384[48];
syn_sha384("abc", 3, hash384);

// Streaming SHA-512:
SYN_SHA512 ctx;
syn_sha512_init(&ctx);
syn_sha512_update(&ctx, chunk1, len1);
syn_sha512_update(&ctx, chunk2, len2);
syn_sha512_final(&ctx, hash512);
```
 





    
## Public Functions Documentation




### function syn\_hmac\_sha384 

_Compute HMAC-SHA384 over data using secret key (RFC 4231)._ 
```C++
void syn_hmac_sha384 (
    const void * key,
    size_t key_len,
    const void * data,
    size_t data_len,
    uint8_t mac
) 
```





**Parameters:**


* `key` Secret key buffer. 
* `key_len` Secret key length in bytes. 
* `data` Data buffer to authenticate. 
* `data_len` Data length in bytes. 
* `mac` Output 48-byte MAC buffer. 




        

<hr>



### function syn\_hmac\_sha384\_final 

_Finalize HMAC-SHA384 computation and retrieve 48-byte MAC._ 
```C++
void syn_hmac_sha384_final (
    SYN_HMAC_SHA384 * ctx,
    uint8_t mac
) 
```





**Parameters:**


* `ctx` HMAC-SHA384 context. 
* `mac` Output 48-byte MAC buffer. 




        

<hr>



### function syn\_hmac\_sha384\_init 

_Initialize HMAC-SHA384 context with a key._ 
```C++
void syn_hmac_sha384_init (
    SYN_HMAC_SHA384 * ctx,
    const void * key,
    size_t key_len
) 
```





**Parameters:**


* `ctx` HMAC-SHA384 context. 
* `key` Secret key buffer. 
* `key_len` Secret key length in bytes. 




        

<hr>



### function syn\_hmac\_sha384\_update 

_Feed data chunk into HMAC-SHA384 computation._ 
```C++
void syn_hmac_sha384_update (
    SYN_HMAC_SHA384 * ctx,
    const void * data,
    size_t len
) 
```





**Parameters:**


* `ctx` HMAC-SHA384 context. 
* `data` Data buffer to authenticate. 
* `len` Length in bytes. 




        

<hr>



### function syn\_hmac\_sha512 

_Compute HMAC-SHA512 over data using secret key (RFC 4231)._ 
```C++
void syn_hmac_sha512 (
    const void * key,
    size_t key_len,
    const void * data,
    size_t data_len,
    uint8_t mac
) 
```





**Parameters:**


* `key` Secret key buffer. 
* `key_len` Secret key length in bytes. 
* `data` Data buffer to authenticate. 
* `data_len` Data length in bytes. 
* `mac` Output 64-byte MAC buffer. 




        

<hr>



### function syn\_hmac\_sha512\_final 

_Finalize HMAC-SHA512 computation and retrieve 64-byte MAC._ 
```C++
void syn_hmac_sha512_final (
    SYN_HMAC_SHA512 * ctx,
    uint8_t mac
) 
```





**Parameters:**


* `ctx` HMAC-SHA512 context. 
* `mac` Output 64-byte MAC buffer. 




        

<hr>



### function syn\_hmac\_sha512\_init 

_Initialize HMAC-SHA512 context with a key._ 
```C++
void syn_hmac_sha512_init (
    SYN_HMAC_SHA512 * ctx,
    const void * key,
    size_t key_len
) 
```





**Parameters:**


* `ctx` HMAC-SHA512 context. 
* `key` Secret key buffer. 
* `key_len` Secret key length in bytes. 




        

<hr>



### function syn\_hmac\_sha512\_update 

_Feed data chunk into HMAC-SHA512 computation._ 
```C++
void syn_hmac_sha512_update (
    SYN_HMAC_SHA512 * ctx,
    const void * data,
    size_t len
) 
```





**Parameters:**


* `ctx` HMAC-SHA512 context. 
* `data` Data buffer to authenticate. 
* `len` Length in bytes. 




        

<hr>



### function syn\_sha384 

_One-shot SHA-384 calculation._ 
```C++
void syn_sha384 (
    const void * data,
    size_t len,
    uint8_t hash
) 
```





**Parameters:**


* `data` Data to hash. 
* `len` Length in bytes. 
* `hash` Output 48-byte digest buffer. 




        

<hr>



### function syn\_sha384\_final 

_Finalize SHA-384 hash and produce 48-byte digest._ 
```C++
void syn_sha384_final (
    SYN_SHA512 * ctx,
    uint8_t hash
) 
```





**Parameters:**


* `ctx` SHA-512 context initialized with [**syn\_sha384\_init()**](syn__sha512_8h.md#function-syn_sha384_init). 
* `hash` Output buffer (must be at least 48 bytes). 




        

<hr>



### function syn\_sha384\_init 

_Initialize context for SHA-384 hashing._ 
```C++
void syn_sha384_init (
    SYN_SHA512 * ctx
) 
```





**Parameters:**


* `ctx` Context to initialize. 




        

<hr>



### function syn\_sha512 

_One-shot SHA-512 calculation._ 
```C++
void syn_sha512 (
    const void * data,
    size_t len,
    uint8_t hash
) 
```





**Parameters:**


* `data` Data to hash. 
* `len` Length in bytes. 
* `hash` Output 64-byte digest buffer. 




        

<hr>



### function syn\_sha512\_final 

_Finalize SHA-512 hash and produce 64-byte digest._ 
```C++
void syn_sha512_final (
    SYN_SHA512 * ctx,
    uint8_t hash
) 
```





**Parameters:**


* `ctx` SHA-512 context. 
* `hash` Output buffer (must be at least 64 bytes). 




        

<hr>



### function syn\_sha512\_init 

_Initialize context for SHA-512 hashing._ 
```C++
void syn_sha512_init (
    SYN_SHA512 * ctx
) 
```





**Parameters:**


* `ctx` Context to initialize. 




        

<hr>



### function syn\_sha512\_update 

_Feed data into the SHA-512 / SHA-384 hash._ 
```C++
void syn_sha512_update (
    SYN_SHA512 * ctx,
    const void * data,
    size_t len
) 
```





**Parameters:**


* `ctx` SHA-512 context. 
* `data` Data buffer to hash. 
* `len` Length in bytes. 




        

<hr>
## Macro Definition Documentation





### define SYN\_SHA384\_DIGEST\_SIZE 

_SHA-384 digest size in bytes (48)._ 
```C++
#define SYN_SHA384_DIGEST_SIZE `48U`
```




<hr>



### define SYN\_SHA512\_BLOCK\_SIZE 

_SHA-512 / SHA-384 block size in bytes (128)._ 
```C++
#define SYN_SHA512_BLOCK_SIZE `128U`
```




<hr>



### define SYN\_SHA512\_DIGEST\_SIZE 

_SHA-512 digest size in bytes (64)._ 
```C++
#define SYN_SHA512_DIGEST_SIZE `64U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_sha512.h`

