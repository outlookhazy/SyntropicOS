

# Struct SYN\_Pool



[**ClassList**](annotated.md) **>** [**SYN\_Pool**](structSYN__Pool.md)



_Fixed-size block memory pool._ 

* `#include <syn_pool.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  size\_t | [**block\_count**](#variable-block_count)  <br> |
|  size\_t | [**block\_size**](#variable-block_size)  <br> |
|  uint8\_t \* | [**buf**](#variable-buf)  <br> |
|  void \* | [**freelist**](#variable-freelist)  <br> |
|  size\_t | [**high\_water**](#variable-high_water)  <br> |
|  size\_t | [**used**](#variable-used)  <br> |












































## Public Attributes Documentation




### variable block\_count 

```C++
size_t SYN_Pool::block_count;
```



Total number of blocks 
 


        

<hr>



### variable block\_size 

```C++
size_t SYN_Pool::block_size;
```



Aligned block size in bytes 
 


        

<hr>



### variable buf 

```C++
uint8_t* SYN_Pool::buf;
```



Backing storage 
 


        

<hr>



### variable freelist 

```C++
void* SYN_Pool::freelist;
```



Head of the freelist (NULL when empty) 
 


        

<hr>



### variable high\_water 

```C++
size_t SYN_Pool::high_water;
```



Peak allocation count 
 


        

<hr>



### variable used 

```C++
size_t SYN_Pool::used;
```



Currently allocated blocks 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_pool.h`

