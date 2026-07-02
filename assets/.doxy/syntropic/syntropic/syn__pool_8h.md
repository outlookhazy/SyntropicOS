

# File syn\_pool.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_pool.h**](syn__pool_8h.md)

[Go to the source code of this file](syn__pool_8h_source.md)

_Fixed-size block memory pool — zero-heap, header-only._ [More...](#detailed-description)

* `#include "../util/syn_assert.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Pool**](structSYN__Pool.md) <br>_Fixed-size block memory pool._  |
























## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void \* | [**syn\_pool\_alloc**](#function-syn_pool_alloc) ([**SYN\_Pool**](structSYN__Pool.md) \* pool) <br>_Allocate a block from the pool._  |
|  size\_t | [**syn\_pool\_available**](#function-syn_pool_available) (const [**SYN\_Pool**](structSYN__Pool.md) \* pool) <br>_Get the number of available (free) blocks._  |
|  size\_t | [**syn\_pool\_block\_count**](#function-syn_pool_block_count) (const [**SYN\_Pool**](structSYN__Pool.md) \* pool) <br>_Get the total block count._  |
|  size\_t | [**syn\_pool\_block\_size**](#function-syn_pool_block_size) (const [**SYN\_Pool**](structSYN__Pool.md) \* pool) <br>_Get the aligned block size._  |
|  void | [**syn\_pool\_free**](#function-syn_pool_free) ([**SYN\_Pool**](structSYN__Pool.md) \* pool, void \* block) <br>_Return a block to the pool._  |
|  size\_t | [**syn\_pool\_high\_watermark**](#function-syn_pool_high_watermark) (const [**SYN\_Pool**](structSYN__Pool.md) \* pool) <br>_Get the peak allocation count (high watermark)._  |
|  void | [**syn\_pool\_init**](#function-syn_pool_init) ([**SYN\_Pool**](structSYN__Pool.md) \* pool, void \* buf, size\_t buf\_size, size\_t block\_size) <br>_Initialize a memory pool._  |
|  size\_t | [**syn\_pool\_used**](#function-syn_pool_used) (const [**SYN\_Pool**](structSYN__Pool.md) \* pool) <br>_Get the number of currently allocated blocks._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_POOL\_ALIGN4**](syn__pool_8h.md#define-syn_pool_align4) (x) `(((x) + 3u) & ~3u)`<br>_Round_ `x` _up to the next multiple of 4._ |
| define  | [**SYN\_POOL\_BUF\_SIZE**](syn__pool_8h.md#define-syn_pool_buf_size) (block\_size, count) `([**SYN\_POOL\_ALIGN4**](syn__pool_8h.md#define-syn_pool_align4)(block\_size) \* (count))`<br>_Compute the buffer size required for a pool._  |

## Detailed Description


A pool allocator backed by a caller-provided byte array. All blocks are the same size. Allocation and free are O(1) via a freelist embedded in the free blocks themselves.


Block size is automatically rounded up to 4-byte alignment. Minimum block size is sizeof(void\*) (4 bytes on 32-bit targets).




**
**


```C++
// Allocate a pool of 8 blocks, each 64 bytes
static uint8_t backing[SYN_POOL_BUF_SIZE(64, 8)];
static SYN_Pool pool;

syn_pool_init(&pool, backing, sizeof(backing), 64);

void *blk = syn_pool_alloc(&pool);
// ... use blk ...
syn_pool_free(&pool, blk);
```
 





    
## Public Static Functions Documentation




### function syn\_pool\_alloc 

_Allocate a block from the pool._ 
```C++
static inline void * syn_pool_alloc (
    SYN_Pool * pool
) 
```





**Parameters:**


* `pool` Pool instance. 



**Returns:**

Pointer to allocated block, or NULL if pool is exhausted. 





        

<hr>



### function syn\_pool\_available 

_Get the number of available (free) blocks._ 
```C++
static inline size_t syn_pool_available (
    const SYN_Pool * pool
) 
```





**Parameters:**


* `pool` Pool instance. 



**Returns:**

Free block count. 





        

<hr>



### function syn\_pool\_block\_count 

_Get the total block count._ 
```C++
static inline size_t syn_pool_block_count (
    const SYN_Pool * pool
) 
```





**Parameters:**


* `pool` Pool instance. 



**Returns:**

Total number of blocks. 





        

<hr>



### function syn\_pool\_block\_size 

_Get the aligned block size._ 
```C++
static inline size_t syn_pool_block_size (
    const SYN_Pool * pool
) 
```





**Parameters:**


* `pool` Pool instance. 



**Returns:**

Block size in bytes (4-byte aligned). 





        

<hr>



### function syn\_pool\_free 

_Return a block to the pool._ 
```C++
static inline void syn_pool_free (
    SYN_Pool * pool,
    void * block
) 
```



The block must have been previously allocated from this pool.




**Parameters:**


* `pool` Pool instance. 
* `block` Pointer to block to free (NULL is safely ignored). 




        

<hr>



### function syn\_pool\_high\_watermark 

_Get the peak allocation count (high watermark)._ 
```C++
static inline size_t syn_pool_high_watermark (
    const SYN_Pool * pool
) 
```





**Parameters:**


* `pool` Pool instance. 



**Returns:**

Maximum simultaneous allocations observed. 





        

<hr>



### function syn\_pool\_init 

_Initialize a memory pool._ 
```C++
static inline void syn_pool_init (
    SYN_Pool * pool,
    void * buf,
    size_t buf_size,
    size_t block_size
) 
```



Partitions the backing buffer into fixed-size blocks and threads a freelist through all of them.




**Parameters:**


* `pool` Pool instance (caller-owned). 
* `buf` Backing storage (caller-owned, must be 4-byte aligned). 
* `buf_size` Size of backing storage in bytes. 
* `block_size` Desired block size. Rounded up to 4-byte alignment. Must be &gt;= sizeof(void\*). 




        

<hr>



### function syn\_pool\_used 

_Get the number of currently allocated blocks._ 
```C++
static inline size_t syn_pool_used (
    const SYN_Pool * pool
) 
```





**Parameters:**


* `pool` Pool instance. 



**Returns:**

Used block count. 





        

<hr>
## Macro Definition Documentation





### define SYN\_POOL\_ALIGN4 

_Round_ `x` _up to the next multiple of 4._
```C++
#define SYN_POOL_ALIGN4 (
    x
) `(((x) + 3u) & ~3u)`
```




<hr>



### define SYN\_POOL\_BUF\_SIZE 

_Compute the buffer size required for a pool._ 
```C++
#define SYN_POOL_BUF_SIZE (
    block_size,
    count
) `( SYN_POOL_ALIGN4 (block_size) * (count))`
```





**Parameters:**


* `block_size` Desired block size (will be aligned to 4). 
* `count` Number of blocks. 



**Returns:**

Required buffer size in bytes. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_pool.h`

