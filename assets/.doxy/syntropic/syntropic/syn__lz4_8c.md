

# File syn\_lz4.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_lz4.c**](syn__lz4_8c.md)

[Go to the source code of this file](syn__lz4_8c_source.md)

_Implementation of Zero-Heap LZ4 Raw Block Compression & Decompression._ 

* `#include "syn_lz4.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_lz4\_compress**](#function-syn_lz4_compress) ([**SYN\_Lz4**](structSYN__Lz4.md) \* ctx, const void \* src, size\_t src\_size, void \* dst, size\_t dst\_capacity) <br>_Compress data buffer into LZ4 raw block format._  |
|  size\_t | [**syn\_lz4\_compress\_bound**](#function-syn_lz4_compress_bound) (size\_t input\_size) <br>_Calculate worst-case compressed output bound for a given input size._  |
|  size\_t | [**syn\_lz4\_decompress**](#function-syn_lz4_decompress) (const void \* src, size\_t src\_size, void \* dst, size\_t dst\_capacity) <br>_Decompress LZ4 raw block buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_lz4\_init**](#function-syn_lz4_init) ([**SYN\_Lz4**](structSYN__Lz4.md) \* ctx) <br>_Initialize an LZ4 compression context._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**lz4\_hash4**](#function-lz4_hash4) (uint32\_t val) <br>_Hash 4 bytes into 12-bit hash index using Knuth multiplicative hash._  |
|  uint32\_t | [**lz4\_read32**](#function-lz4_read32) (const uint8\_t \* ptr) <br>_Read 32-bit little-endian word from unaligned byte pointer._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**LZ4\_HASH\_BITS**](syn__lz4_8c.md#define-lz4_hash_bits)  `12U`<br> |
| define  | [**LZ4\_MIN\_MATCH**](syn__lz4_8c.md#define-lz4_min_match)  `4U`<br> |

## Public Functions Documentation




### function syn\_lz4\_compress 

_Compress data buffer into LZ4 raw block format._ 
```C++
size_t syn_lz4_compress (
    SYN_Lz4 * ctx,
    const void * src,
    size_t src_size,
    void * dst,
    size_t dst_capacity
) 
```





**Parameters:**


* `ctx` Pointer to initialized LZ4 context. 
* `src` Source data buffer to compress. 
* `src_size` Source data size in bytes. 
* `dst` Destination buffer for compressed output. 
* `dst_capacity` Capacity of destination buffer in bytes. 



**Returns:**

Number of compressed bytes written to dst, or 0 on error/overflow. 





        

<hr>



### function syn\_lz4\_compress\_bound 

_Calculate worst-case compressed output bound for a given input size._ 
```C++
size_t syn_lz4_compress_bound (
    size_t input_size
) 
```





**Parameters:**


* `input_size` Uncompressed payload size in bytes. 



**Returns:**

Maximum compressed buffer size required. 





        

<hr>



### function syn\_lz4\_decompress 

_Decompress LZ4 raw block buffer._ 
```C++
size_t syn_lz4_decompress (
    const void * src,
    size_t src_size,
    void * dst,
    size_t dst_capacity
) 
```



Decompression is fully stateless and requires no context workspace.




**Parameters:**


* `src` Compressed source buffer. 
* `src_size` Compressed source size in bytes. 
* `dst` Destination buffer for decompressed output. 
* `dst_capacity` Capacity of destination buffer in bytes. 



**Returns:**

Number of decompressed bytes written to dst, or 0 on corruption/overflow. 





        

<hr>



### function syn\_lz4\_init 

_Initialize an LZ4 compression context._ 
```C++
SYN_Status syn_lz4_init (
    SYN_Lz4 * ctx
) 
```





**Parameters:**


* `ctx` Pointer to LZ4 context instance. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM. 





        

<hr>
## Public Static Functions Documentation




### function lz4\_hash4 

_Hash 4 bytes into 12-bit hash index using Knuth multiplicative hash._ 
```C++
static inline uint32_t lz4_hash4 (
    uint32_t val
) 
```





**Parameters:**


* `val` 32-bit integer word to hash. 



**Returns:**

12-bit hash index (0..4095). 





        

<hr>



### function lz4\_read32 

_Read 32-bit little-endian word from unaligned byte pointer._ 
```C++
static inline uint32_t lz4_read32 (
    const uint8_t * ptr
) 
```





**Parameters:**


* `ptr` Pointer to byte stream. 



**Returns:**

32-bit unsigned integer value. 





        

<hr>
## Macro Definition Documentation





### define LZ4\_HASH\_BITS 

```C++
#define LZ4_HASH_BITS `12U`
```



Log2 size of 4096-entry hash table 


        

<hr>



### define LZ4\_MIN\_MATCH 

```C++
#define LZ4_MIN_MATCH `4U`
```



Minimum byte match length for LZ4 compression 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_lz4.c`

