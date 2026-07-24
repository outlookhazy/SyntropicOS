

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



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**ALIGN4**](syn__slab_8c.md#define-align4) (x) `(((x) + 3u) & ~3u)`<br> |

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
## Macro Definition Documentation





### define ALIGN4 

```C++
#define ALIGN4 (
    x
) `(((x) + 3u) & ~3u)`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_slab.c`

