

# File syn\_sha512.c



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_sha512.c**](syn__sha512_8c.md)

[Go to the source code of this file](syn__sha512_8c_source.md)

_SHA-512 & SHA-384 cryptographic hash and HMAC implementation._ 

* `#include "syn_sha512.h"`
* `#include <string.h>`





































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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_sha512.c`

