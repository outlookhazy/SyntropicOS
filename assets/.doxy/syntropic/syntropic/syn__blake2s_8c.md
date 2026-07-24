

# File syn\_blake2s.c



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_blake2s.c**](syn__blake2s_8c.md)

[Go to the source code of this file](syn__blake2s_8c_source.md)

_BLAKE2s implementation — RFC 7693._ 

* `#include "syn_blake2s.h"`
* `#include <string.h>`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const uint32\_t | [**blake2s\_iv**](#variable-blake2s_iv)   = `/* multi line expression */`<br>_BLAKE2s initialization vector._  |
|  const uint8\_t | [**sigma**](#variable-sigma)   = `/* multi line expression */`<br>_BLAKE2s message word permutation schedule._  |














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
|  void | [**blake2s\_compress**](#function-blake2s_compress) ([**SYN\_BLAKE2s**](structSYN__BLAKE2s.md) \* ctx, const uint8\_t block, int is\_last) <br>_BLAKE2s compression (F). Mixes one 64-byte block into state._  |
|  void | [**blake2s\_increment\_counter**](#function-blake2s_increment_counter) ([**SYN\_BLAKE2s**](structSYN__BLAKE2s.md) \* ctx, uint32\_t inc) <br>_Increment byte counter._  |
|  uint32\_t | [**load32\_le**](#function-load32_le) (const uint8\_t \* p) <br>_Load 32-bit little-endian word._  |
|  uint32\_t | [**rotr32**](#function-rotr32) (uint32\_t x, unsigned n) <br>_32-bit right rotation._  |
|  void | [**store32\_le**](#function-store32_le) (uint8\_t \* p, uint32\_t v) <br>_Store 32-bit little-endian word._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**G**](syn__blake2s_8c.md#define-g) (v, a, b, c, d, x, y) `/* multi line expression */`<br>_BLAKE2s G mixing function._  |

## Public Static Attributes Documentation




### variable blake2s\_iv 

_BLAKE2s initialization vector._ 
```C++
const uint32_t blake2s_iv[8];
```




<hr>



### variable sigma 

_BLAKE2s message word permutation schedule._ 
```C++
const uint8_t sigma[10][16];
```




<hr>
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




### function blake2s\_compress 

_BLAKE2s compression (F). Mixes one 64-byte block into state._ 
```C++
static void blake2s_compress (
    SYN_BLAKE2s * ctx,
    const uint8_t block,
    int is_last
) 
```





**Parameters:**


* `ctx` Hash context. 
* `block` 64-byte input block. 
* `is_last` Flag indicating the last block. 




        

<hr>



### function blake2s\_increment\_counter 

_Increment byte counter._ 
```C++
static inline void blake2s_increment_counter (
    SYN_BLAKE2s * ctx,
    uint32_t inc
) 
```





**Parameters:**


* `ctx` Hash context. 
* `inc` Increment value in bytes. 




        

<hr>



### function load32\_le 

_Load 32-bit little-endian word._ 
```C++
static inline uint32_t load32_le (
    const uint8_t * p
) 
```





**Parameters:**


* `p` Pointer to source bytes. 



**Returns:**

32-bit value. 





        

<hr>



### function rotr32 

_32-bit right rotation._ 
```C++
static inline uint32_t rotr32 (
    uint32_t x,
    unsigned n
) 
```





**Parameters:**


* `x` Value to rotate. 
* `n` Bits to shift. 



**Returns:**

Rotated value. 





        

<hr>



### function store32\_le 

_Store 32-bit little-endian word._ 
```C++
static inline void store32_le (
    uint8_t * p,
    uint32_t v
) 
```





**Parameters:**


* `p` Pointer to destination. 
* `v` Value to store. 




        

<hr>
## Macro Definition Documentation





### define G 

_BLAKE2s G mixing function._ 
```C++
#define G (
    v,
    a,
    b,
    c,
    d,
    x,
    y
) `/* multi line expression */`
```





**Parameters:**


* `v` Working vector (16 words). 
* `a` Vector indices. 
* `x` Message words. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_blake2s.c`

