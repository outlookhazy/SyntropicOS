

# Struct SYN\_SlabStats



[**ClassList**](annotated.md) **>** [**SYN\_SlabStats**](structSYN__SlabStats.md)



_Slab Allocator Statistics._ 

* `#include <syn_slab.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  size\_t | [**allocated\_bytes**](#variable-allocated_bytes)  <br> |
|  size\_t | [**total\_allocs**](#variable-total_allocs)  <br> |
|  size\_t | [**total\_bytes**](#variable-total_bytes)  <br> |












































## Public Attributes Documentation




### variable allocated\_bytes 

```C++
size_t SYN_SlabStats::allocated_bytes;
```



Currently allocated bytes 


        

<hr>



### variable total\_allocs 

```C++
size_t SYN_SlabStats::total_allocs;
```



Total active allocations count 


        

<hr>



### variable total\_bytes 

```C++
size_t SYN_SlabStats::total_bytes;
```



Total managed memory in bytes 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_slab.h`

