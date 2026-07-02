

# File syn\_barrier.h



[**FileList**](files.md) **>** [**common**](dir_b4b1f229ee23b400200c904b1998093f.md) **>** [**syn\_barrier.h**](syn__barrier_8h.md)

[Go to the source code of this file](syn__barrier_8h_source.md)

_Memory ordering primitives for cross-core and ISR safety._ [More...](#detailed-description)

* `#include "syn_compiler.h"`
* `#include <stddef.h>`







































## Public Static Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_load\_acquire**](#function-syn_load_acquire) (const volatile size\_t \* ptr) <br>_Load with acquire semantics (single-core path)._  |
|  void | [**syn\_store\_release**](#function-syn_store_release) (volatile size\_t \* ptr, size\_t val) <br>_Store with release semantics (single-core path)._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_LOAD\_ACQUIRE**](syn__barrier_8h.md#define-syn_load_acquire) (ptr) `[**syn\_load\_acquire**](syn__barrier_8h.md#function-syn_load_acquire)((ptr))`<br>_Load with acquire semantics (macro form)._  |
| define  | [**SYN\_STORE\_RELEASE**](syn__barrier_8h.md#define-syn_store_release) (ptr, val) `[**syn\_store\_release**](syn__barrier_8h.md#function-syn_store_release)((ptr), (val))`<br>_Store with release semantics (macro form)._  |

## Detailed Description


Provides acquire/release semantics for lock-free data structures that are shared between CPU cores (AMP multicore) or between ISR and main context.




**
**




When `SYN_USE_MULTICORE` is **off** (the default), the acquire/release functions compile to plain volatile reads and writes — identical to what the code would do without barriers. Zero overhead.


When `SYN_USE_MULTICORE` is **on**, the functions insert hardware memory barriers (via `syn_port_memory_barrier()`) at the correct points to ensure cross-core visibility.




**
**





* **STORE\_RELEASE:** All prior writes are visible before this store. Use after writing data, before publishing the index.
* **LOAD\_ACQUIRE:** No subsequent reads can be reordered before this load. Use after reading an index, before accessing the data it guards.






**
**


```C++
// Producer (core 0):
buf[head] = data;
SYN_STORE_RELEASE(&rb->head, next_head);

// Consumer (core 1):
size_t h = SYN_LOAD_ACQUIRE(&rb->head);
if (h != tail) {
    x = buf[tail];  // guaranteed to see data written before head
}
```
 





    
## Public Static Functions Documentation




### function syn\_load\_acquire 

_Load with acquire semantics (single-core path)._ 
```C++
static inline size_t syn_load_acquire (
    const volatile size_t * ptr
) 
```



On single-core targets, volatile access is sufficient. This compiles to a plain load — zero overhead.




**Parameters:**


* `ptr` Pointer to volatile size\_t. 



**Returns:**

The loaded value. 





        

<hr>



### function syn\_store\_release 

_Store with release semantics (single-core path)._ 
```C++
static inline void syn_store_release (
    volatile size_t * ptr,
    size_t val
) 
```



On single-core targets, volatile access is sufficient because interrupts imply a full pipeline flush. This compiles to a plain store — zero overhead.




**Parameters:**


* `ptr` Pointer to volatile size\_t. 
* `val` Value to store. 




        

<hr>
## Macro Definition Documentation





### define SYN\_LOAD\_ACQUIRE 

_Load with acquire semantics (macro form)._ 
```C++
#define SYN_LOAD_ACQUIRE (
    ptr
) `syn_load_acquire ((ptr))`
```




<hr>



### define SYN\_STORE\_RELEASE 

_Store with release semantics (macro form)._ 
```C++
#define SYN_STORE_RELEASE (
    ptr,
    val
) `syn_store_release ((ptr), (val))`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/common/syn_barrier.h`

