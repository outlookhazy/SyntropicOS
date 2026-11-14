

# File syn\_lz4.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_lz4.h**](syn__lz4_8h.md)

[Go to the source code of this file](syn__lz4_8h_source.md)

_Zero-Heap LZ4 Raw Block Compression & Decompression._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Lz4**](structSYN__Lz4.md) <br>_LZ4 Compression Workspace Context._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_lz4\_compress**](#function-syn_lz4_compress) ([**SYN\_Lz4**](structSYN__Lz4.md) \* ctx, const void \* src, size\_t src\_size, void \* dst, size\_t dst\_capacity) <br>_Compress data buffer into LZ4 raw block format._  |
|  size\_t | [**syn\_lz4\_compress\_bound**](#function-syn_lz4_compress_bound) (size\_t input\_size) <br>_Calculate worst-case compressed output bound for a given input size._  |
|  size\_t | [**syn\_lz4\_decompress**](#function-syn_lz4_decompress) (const void \* src, size\_t src\_size, void \* dst, size\_t dst\_capacity) <br>_Decompress LZ4 raw block buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_lz4\_init**](#function-syn_lz4_init) ([**SYN\_Lz4**](structSYN__Lz4.md) \* ctx) <br>_Initialize an LZ4 compression context._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_LZ4\_HASH\_SIZE**](syn__lz4_8h.md#define-syn_lz4_hash_size)  `4096U`<br> |

## Detailed Description


Implements standard LZ4 raw block format compression and decompression without dynamic memory allocation or complex framing overhead. 


    
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
## Macro Definition Documentation





### define SYN\_LZ4\_HASH\_SIZE 

```C++
#define SYN_LZ4_HASH_SIZE `4096U`
```



LZ4 hash table size (4096 entries = 8 KB) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_lz4.h`

