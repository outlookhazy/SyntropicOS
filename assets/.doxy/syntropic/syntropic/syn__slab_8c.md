

# File syn\_slab.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_slab.c**](syn__slab_8c.md)

[Go to the source code of this file](syn__slab_8c_source.md)

_Multi-Class Slab Memory Allocator implementation._ 

* `#include "syn_slab.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void \* | [**syn\_slab\_alloc**](#function-syn_slab_alloc) ([**SYN\_SlabAllocator**](structSYN__SlabAllocator.md) \* slab, size\_t size) <br>_Allocate a block from the optimal slab class._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_slab\_free**](#function-syn_slab_free) ([**SYN\_SlabAllocator**](structSYN__SlabAllocator.md) \* slab, void \* ptr) <br>_Free a previously allocated slab block._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_slab\_get\_stats**](#function-syn_slab_get_stats) (const [**SYN\_SlabAllocator**](structSYN__SlabAllocator.md) \* slab, [**SYN\_SlabStats**](structSYN__SlabStats.md) \* stats) <br>_Retrieve current allocator usage statistics._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_slab\_init**](#function-syn_slab_init) ([**SYN\_SlabAllocator**](structSYN__SlabAllocator.md) \* slab, void \* backing\_memory, size\_t total\_size, const size\_t \* block\_sizes, const size\_t \* blocks\_per\_class, size\_t num\_classes) <br>_Initialize a Multi-Class Slab Allocator._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**slab\_freelist\_contains**](#function-slab_freelist_contains) (const [**SYN\_SlabClass**](structSYN__SlabClass.md) \* sc, const void \* ptr) <br>_Check whether_ `ptr` _is already linked in_`sc->freelist` _._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**slab\_init\_params\_valid**](#function-slab_init_params_valid) (const [**SYN\_SlabAllocator**](structSYN__SlabAllocator.md) \* slab, const void \* backing\_memory, const size\_t \* block\_sizes, const size\_t \* blocks\_per\_class, size\_t num\_classes) <br>_Check whether init parameters for syn\_slab\_init are valid._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**slab\_mark\_block\_free**](#function-slab_mark_block_free) ([**SYN\_SlabClass**](structSYN__SlabClass.md) \* sc, const void \* ptr, size\_t idx) <br>_Validate and mark a slab block free, rejecting double-frees in O(1) for idx &lt; 64._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SLAB\_ALIGN\_PTR**](syn__slab_8c.md#define-slab_align_ptr) (x) `(((size\_t)(x) + (sizeof(void \*) - 1U)) & ~(sizeof(void \*) - 1U))`<br> |

## Public Functions Documentation




### function syn\_slab\_alloc 

_Allocate a block from the optimal slab class._ 
```C++
void * syn_slab_alloc (
    SYN_SlabAllocator * slab,
    size_t size
) 
```





**Parameters:**


* `slab` Allocator context. 
* `size` Requested allocation size in bytes. 



**Returns:**

Pointer to allocated block, or NULL if unavailable. 





        

<hr>



### function syn\_slab\_free 

_Free a previously allocated slab block._ 
```C++
SYN_Status syn_slab_free (
    SYN_SlabAllocator * slab,
    void * ptr
) 
```





**Parameters:**


* `slab` Allocator context. 
* `ptr` Pointer to block to free. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM. 





        

<hr>



### function syn\_slab\_get\_stats 

_Retrieve current allocator usage statistics._ 
```C++
SYN_Status syn_slab_get_stats (
    const SYN_SlabAllocator * slab,
    SYN_SlabStats * stats
) 
```





**Parameters:**


* `slab` Allocator context. 
* `stats` Output statistics structure. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_slab\_init 

_Initialize a Multi-Class Slab Allocator._ 
```C++
SYN_Status syn_slab_init (
    SYN_SlabAllocator * slab,
    void * backing_memory,
    size_t total_size,
    const size_t * block_sizes,
    const size_t * blocks_per_class,
    size_t num_classes
) 
```





**Parameters:**


* `slab` Pointer to allocator context. 
* `backing_memory` Pointer to static backing array. 
* `total_size` Size of backing memory in bytes. 
* `block_sizes` Array of desired block sizes (e.g. {16, 64, 256}). 
* `blocks_per_class` Array of block counts for each class. 
* `num_classes` Number of classes (1 to SYN\_SLAB\_MAX\_CLASSES). 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Public Static Functions Documentation




### function slab\_freelist\_contains 

_Check whether_ `ptr` _is already linked in_`sc->freelist` _._
```C++
static bool slab_freelist_contains (
    const SYN_SlabClass * sc,
    const void * ptr
) 
```





**Parameters:**


* `sc` Slab class descriptor. 
* `ptr` Block pointer to search for in the freelist. 



**Returns:**

true if `ptr` is already present in the freelist. 





        

<hr>



### function slab\_init\_params\_valid 

_Check whether init parameters for syn\_slab\_init are valid._ 
```C++
static bool slab_init_params_valid (
    const SYN_SlabAllocator * slab,
    const void * backing_memory,
    const size_t * block_sizes,
    const size_t * blocks_per_class,
    size_t num_classes
) 
```





**Parameters:**


* `slab` Slab allocator instance. 
* `backing_memory` Backing memory buffer pointer. 
* `block_sizes` Array of block sizes per class. 
* `blocks_per_class` Array of block counts per class. 
* `num_classes` Number of slab size classes. 



**Returns:**

true if parameters are non-null and num\_classes is within bounds. 





        

<hr>



### function slab\_mark\_block\_free 

_Validate and mark a slab block free, rejecting double-frees in O(1) for idx &lt; 64._ 
```C++
static bool slab_mark_block_free (
    SYN_SlabClass * sc,
    const void * ptr,
    size_t idx
) 
```





**Parameters:**


* `sc` Slab class descriptor. 
* `ptr` Block pointer aligned within `sc`. 
* `idx` Block index within `sc`. 



**Returns:**

true if block was allocated and is now marked free, false on double-free. 





        

<hr>
## Macro Definition Documentation





### define SLAB\_ALIGN\_PTR 

```C++
#define SLAB_ALIGN_PTR (
    x
) `(((size_t)(x) + (sizeof(void *) - 1U)) & ~(sizeof(void *) - 1U))`
```



Align to pointer size 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_slab.c`

