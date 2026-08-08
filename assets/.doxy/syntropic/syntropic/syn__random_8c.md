

# File syn\_random.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_random.c**](syn__random_8c.md)

[Go to the source code of this file](syn__random_8c_source.md)

_Random number utility implementation._ 

* `#include "syn_random.h"`
* `#include "../port/syn_port_system.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_random\_fill**](#function-syn_port_random_fill) (void \* buf, size\_t len) <br>_Porting hook: must be implemented by the platform to provide entropy._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_random\_fallback\_fill**](#function-syn_random_fallback_fill) (void \* buf, size\_t len) <br>_Internal: XOR-shift fallback for when no hardware entropy is available._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_random\_fill**](#function-syn_random_fill) (void \* buf, size\_t len) <br>_Fill a buffer with cryptographically secure random bytes._  |
|  uint32\_t | [**syn\_random\_range**](#function-syn_random_range) (uint32\_t min, uint32\_t max) <br>_Generate a random integer within a range [min, max]._  |
|  uint32\_t | [**syn\_random\_u32**](#function-syn_random_u32) (void) <br>_Generate a random 32-bit integer._  |




























## Public Functions Documentation




### function syn\_port\_random\_fill 

_Porting hook: must be implemented by the platform to provide entropy._ 
```C++
SYN_WEAK  SYN_Status syn_port_random_fill (
    void * buf,
    size_t len
) 
```





**Parameters:**


* `buf` Output buffer. 
* `len` Number of bytes. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_random\_fallback\_fill 

_Internal: XOR-shift fallback for when no hardware entropy is available._ 
```C++
SYN_Status syn_random_fallback_fill (
    void * buf,
    size_t len
) 
```



Internal XOR-shift fallback for random data.




**Parameters:**


* `buf` Output buffer. 
* `len` Length in bytes. 



**Returns:**

SYN\_OK. 





        

<hr>



### function syn\_random\_fill 

_Fill a buffer with cryptographically secure random bytes._ 
```C++
SYN_Status syn_random_fill (
    void * buf,
    size_t len
) 
```



This function relies on a platform-specific hardware RNG or secure OS source (via syn\_port\_random\_fill).




**Parameters:**


* `buf` Output buffer. 
* `len` Number of bytes to generate. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if the RNG is unavailable. 





        

<hr>



### function syn\_random\_range 

_Generate a random integer within a range [min, max]._ 
```C++
uint32_t syn_random_range (
    uint32_t min,
    uint32_t max
) 
```





**Parameters:**


* `min` Minimum value (inclusive). 
* `max` Maximum value (inclusive). 



**Returns:**

Random value. 





        

<hr>



### function syn\_random\_u32 

_Generate a random 32-bit integer._ 
```C++
uint32_t syn_random_u32 (
    void
) 
```





**Returns:**

Random value, or 0 on failure (RNG check is recommended for sensitive use). 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_random.c`

