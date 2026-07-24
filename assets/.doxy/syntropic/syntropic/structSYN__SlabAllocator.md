

# Struct SYN\_SlabAllocator



[**ClassList**](annotated.md) **>** [**SYN\_SlabAllocator**](structSYN__SlabAllocator.md)



_Multi-Class Slab Allocator._ 

* `#include <syn_slab.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t \* | [**backing\_memory**](#variable-backing_memory)  <br> |
|  [**SYN\_SlabClass**](structSYN__SlabClass.md) | [**classes**](#variable-classes)  <br> |
|  size\_t | [**num\_classes**](#variable-num_classes)  <br> |
|  size\_t | [**total\_memory\_size**](#variable-total_memory_size)  <br> |












































## Public Attributes Documentation




### variable backing\_memory 

```C++
uint8_t* SYN_SlabAllocator::backing_memory;
```



Backing memory pointer 


        

<hr>



### variable classes 

```C++
SYN_SlabClass SYN_SlabAllocator::classes[SYN_SLAB_MAX_CLASSES];
```



Slab classes 


        

<hr>



### variable num\_classes 

```C++
size_t SYN_SlabAllocator::num_classes;
```



Number of configured classes 


        

<hr>



### variable total\_memory\_size 

```C++
size_t SYN_SlabAllocator::total_memory_size;
```



Total backing size 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_slab.h`

