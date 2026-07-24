

# File syn\_hmac.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_hmac.h**](syn__hmac_8h.md)

[Go to the source code of this file](syn__hmac_8h_source.md)

_HMAC-SHA256 — keyed-hash message authentication code._ [More...](#detailed-description)

* `#include "syn_sha256.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <string.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_HMAC\_SHA256**](structSYN__HMAC__SHA256.md) <br>_HMAC-SHA256 context — caller-owned._  |
























## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_hmac\_sha256**](#function-syn_hmac_sha256) (const void \* key, size\_t key\_len, const void \* data, size\_t data\_len, uint8\_t mac) <br>_Compute HMAC-SHA256 in one shot._  |
|  void | [**syn\_hmac\_sha256\_final**](#function-syn_hmac_sha256_final) ([**SYN\_HMAC\_SHA256**](structSYN__HMAC__SHA256.md) \* ctx, uint8\_t mac) <br>_Finalize and produce the 32-byte HMAC._  |
|  void | [**syn\_hmac\_sha256\_init**](#function-syn_hmac_sha256_init) ([**SYN\_HMAC\_SHA256**](structSYN__HMAC__SHA256.md) \* ctx, const void \* key, size\_t key\_len) <br>_Initialize HMAC-SHA256 with a key._  |
|  void | [**syn\_hmac\_sha256\_update**](#function-syn_hmac_sha256_update) ([**SYN\_HMAC\_SHA256**](structSYN__HMAC__SHA256.md) \* ctx, const void \* data, size\_t len) <br>_Feed message data into the HMAC._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_HMAC\_SHA256\_SIZE**](syn__hmac_8h.md#define-syn_hmac_sha256_size)  `[**SYN\_SHA256\_DIGEST\_SIZE**](syn__sha256_8h.md#define-syn_sha256_digest_size)`<br>_HMAC-SHA256 output size in bytes._  |

## Detailed Description


Built on top of syn\_sha256. Provides the same streaming pattern: init (with key) → update → final


The context wraps two SHA-256 contexts (~224 bytes caller-owned). No heap, no external dependencies.




**
**


```C++
// One-shot:
uint8_t mac[32];
syn_hmac_sha256(key, key_len, message, msg_len, mac);

// Streaming:
SYN_HMAC_SHA256 ctx;
syn_hmac_sha256_init(&ctx, key, key_len);
syn_hmac_sha256_update(&ctx, chunk1, len1);
syn_hmac_sha256_update(&ctx, chunk2, len2);
syn_hmac_sha256_final(&ctx, mac);
```
 





    
## Public Static Functions Documentation




### function syn\_hmac\_sha256 

_Compute HMAC-SHA256 in one shot._ 
```C++
static inline void syn_hmac_sha256 (
    const void * key,
    size_t key_len,
    const void * data,
    size_t data_len,
    uint8_t mac
) 
```





**Parameters:**


* `key` Secret key. 
* `key_len` Key length in bytes. 
* `data` Message data. 
* `data_len` Message length in bytes. 
* `mac` Output buffer (must be at least 32 bytes). 




        

<hr>



### function syn\_hmac\_sha256\_final 

_Finalize and produce the 32-byte HMAC._ 
```C++
static inline void syn_hmac_sha256_final (
    SYN_HMAC_SHA256 * ctx,
    uint8_t mac
) 
```





**Parameters:**


* `ctx` HMAC context. 
* `mac` Output buffer (must be at least 32 bytes). 




        

<hr>



### function syn\_hmac\_sha256\_init 

_Initialize HMAC-SHA256 with a key._ 
```C++
static inline void syn_hmac_sha256_init (
    SYN_HMAC_SHA256 * ctx,
    const void * key,
    size_t key_len
) 
```



If key\_len &gt; 64, the key is first hashed with SHA-256.




**Parameters:**


* `ctx` HMAC context. 
* `key` Secret key. 
* `key_len` Key length in bytes. 




        

<hr>



### function syn\_hmac\_sha256\_update 

_Feed message data into the HMAC._ 
```C++
static inline void syn_hmac_sha256_update (
    SYN_HMAC_SHA256 * ctx,
    const void * data,
    size_t len
) 
```





**Parameters:**


* `ctx` HMAC context. 
* `data` Message data. 
* `len` Length in bytes. 




        

<hr>
## Macro Definition Documentation





### define SYN\_HMAC\_SHA256\_SIZE 

_HMAC-SHA256 output size in bytes._ 
```C++
#define SYN_HMAC_SHA256_SIZE `SYN_SHA256_DIGEST_SIZE`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_hmac.h`

