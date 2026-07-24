

# Struct SYN\_SlabClass



[**ClassList**](annotated.md) **>** [**SYN\_SlabClass**](structSYN__SlabClass.md)



_Single Slab Size Class Descriptor._ 

* `#include <syn_slab.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  size\_t | [**block\_size**](#variable-block_size)  <br> |
|  size\_t | [**free\_count**](#variable-free_count)  <br> |
|  void \* | [**freelist**](#variable-freelist)  <br> |
|  uint8\_t \* | [**start\_ptr**](#variable-start_ptr)  <br> |
|  size\_t | [**total\_blocks**](#variable-total_blocks)  <br> |












































## Public Attributes Documentation




### variable block\_size 

```C++
size_t SYN_SlabClass::block_size;
```



Aligned block size for this class 


        

<hr>



### variable free\_count 

```C++
size_t SYN_SlabClass::free_count;
```



Current free blocks 


        

<hr>



### variable freelist 

```C++
void* SYN_SlabClass::freelist;
```



Embedded freelist head 


        

<hr>



### variable start\_ptr 

```C++
uint8_t* SYN_SlabClass::start_ptr;
```



Start pointer of slab region 


        

<hr>



### variable total\_blocks 

```C++
size_t SYN_SlabClass::total_blocks;
```



Total blocks in this slab class 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_slab.h`

