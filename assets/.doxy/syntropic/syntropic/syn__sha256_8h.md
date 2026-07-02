

# File syn\_sha256.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_sha256.h**](syn__sha256_8h.md)

[Go to the source code of this file](syn__sha256_8h_source.md)

_SHA-256 cryptographic hash — pure C99, zero dependencies._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stddef.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_SHA256**](structSYN__SHA256.md) <br>_SHA-256 hash context — caller-owned._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_sha256\_final**](#function-syn_sha256_final) ([**SYN\_SHA256**](structSYN__SHA256.md) \* ctx, uint8\_t hash) <br>_Finalize the hash and produce the 32-byte digest._  |
|  void | [**syn\_sha256\_init**](#function-syn_sha256_init) ([**SYN\_SHA256**](structSYN__SHA256.md) \* ctx) <br>_Initialize a SHA-256 context._  |
|  void | [**syn\_sha256\_update**](#function-syn_sha256_update) ([**SYN\_SHA256**](structSYN__SHA256.md) \* ctx, const void \* data, size\_t len) <br>_Feed data into the hash._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_sha256**](#function-syn_sha256) (const void \* data, size\_t len, uint8\_t hash) <br>_Compute SHA-256 of a complete buffer (one-shot convenience)._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_SHA256\_BLOCK\_SIZE**](syn__sha256_8h.md#define-syn_sha256_block_size)  `64`<br>_SHA-256 block size in bytes._  |
| define  | [**SYN\_SHA256\_DIGEST\_SIZE**](syn__sha256_8h.md#define-syn_sha256_digest_size)  `32`<br>_SHA-256 digest size in bytes._  |

## Detailed Description


Follows the same streaming pattern as [**syn\_crc.h**](syn__crc_8h.md): init → update (repeated) → final


The context struct is caller-owned (~112 bytes on 32-bit targets). No heap allocation, no floating point, no external libraries.




**
**


```C++
// One-shot:
uint8_t hash[32];
syn_sha256("abc", 3, hash);

// Streaming:
SYN_SHA256 ctx;
syn_sha256_init(&ctx);
syn_sha256_update(&ctx, chunk1, len1);
syn_sha256_update(&ctx, chunk2, len2);
syn_sha256_final(&ctx, hash);
```
 





    
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




### function syn\_sha256 

_Compute SHA-256 of a complete buffer (one-shot convenience)._ 
```C++
static inline void syn_sha256 (
    const void * data,
    size_t len,
    uint8_t hash
) 
```





**Parameters:**


* `data` Data to hash. 
* `len` Length in bytes. 
* `hash` Output buffer (must be at least 32 bytes). 




        

<hr>
## Macro Definition Documentation





### define SYN\_SHA256\_BLOCK\_SIZE 

_SHA-256 block size in bytes._ 
```C++
#define SYN_SHA256_BLOCK_SIZE `64`
```




<hr>



### define SYN\_SHA256\_DIGEST\_SIZE 

_SHA-256 digest size in bytes._ 
```C++
#define SYN_SHA256_DIGEST_SIZE `32`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_sha256.h`

